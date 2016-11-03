// Do not remove the include below
#include "QVSACM1.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define SENDER false
#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define BROADCAST_ADDRESS_1 0x00000000
#define BROADCAST_ADDRESS_2 0x0000FFFF
//#define BROADCAST_ADDRESS_1 0x0013A200
//#define BROADCAST_ADDRESS_2 0x40B317FA

const uint8_t NUM_MISSED_HB_BEFORE_PURGE = 6;

const float INITAL_DUPLICATION_SETTING = 0.0;
const uint8_t CODEC_SETTTING = 2;
const unsigned long TRACE_INTERVAL = 10000;
const uint8_t PAYLOAD_SIZE = 76;
const uint8_t VOICE_DATA_INTERVAL = 2;
const unsigned long REQUEST_STREAM = 200;
const unsigned long GRANT_TIMEOUT_LENGTH = 50;
const unsigned long REJECT_TIMEOUT_LENGTH = 10;
const unsigned long HEARTBEAT_INTERVAL = 500;
const unsigned long PATHLOSS_INTERVAL = 10000;
const unsigned long CALCULATE_THROUGHPUT_INTERVAL = 8000;
const unsigned long STREAM_DELAY_START = 5000;
const float DISTANCE_THRESHOLD = 7.00;
unsigned long STREAM_DELAY_START_BEGIN = 0;

XBee xbee = XBee();
VoicePacketSender * voicePacketSender;
AdmissionControl * admissionControl;
VoiceStreamManager * voiceStreamManager;
AODV * aodv;

ThreadController controller = ThreadController();
Thread * heartbeat = new Thread();
Thread * sendInital = new Thread();
Thread * responseThread = new Thread();
Thread * pathLoss = new Thread();
Thread * generateVoice = new Thread();
Thread * calculateThroughput = new Thread();
Thread * debugHeartbeatTable = new Thread();
Thread * endMessage = new Thread();
Thread * threadMessage = new Thread();

XBeeAddress64 broadcastAddress = XBeeAddress64(BROADCAST_ADDRESS_1, BROADCAST_ADDRESS_2);
XBeeAddress64 sinkAddress = XBeeAddress64(SINK_ADDRESS_1, SINK_ADDRESS_2);
XBeeAddress64 myAddress;

void setup() {
	arduinoSetup();

	clearBuffer();
	xbee.getMyAddress(myAddress, DEBUG);
	SerialUSB.print("My Address: ");
	myAddress.printAddressASCII(&SerialUSB);
	SerialUSB.println();

	voiceStreamManager = new VoiceStreamManager(xbee, PAYLOAD_SIZE);
	aodv = new AODV(xbee, myAddress, broadcastAddress, sinkAddress, CODEC_SETTTING, INITAL_DUPLICATION_SETTING);
	voicePacketSender = new VoicePacketSender(xbee, aodv, pathLoss, calculateThroughput, voiceStreamManager, myAddress,
			sinkAddress, CODEC_SETTTING, INITAL_DUPLICATION_SETTING, PAYLOAD_SIZE);
	admissionControl = new AdmissionControl(myAddress, sinkAddress, xbee, aodv, voiceStreamManager, voicePacketSender,
			GRANT_TIMEOUT_LENGTH, REJECT_TIMEOUT_LENGTH);
	setupThreads();

	digitalWrite(13, LOW);
	STREAM_DELAY_START_BEGIN = millis();
}

void loop() {
	controller.run();
}

void arduinoSetup() {
	pinMode(STATUS_LED, OUTPUT);
	pinMode(ERROR_LED, OUTPUT);

	randomSeed(analogRead(0));

	Serial.begin(111111);
	SerialUSB.begin(111111);

	xbee.setSerial(Serial);

	digitalWrite(13, HIGH);
	unsigned long start = millis();

	clearBuffer();

	while (millis() - start < 15000) {

	}

}

void sendInitPacket() {

	if (millis() - STREAM_DELAY_START_BEGIN > STREAM_DELAY_START) {
		aodv->getRoute();
	}
}

void sendVoicePacket() {

	XBeeAddress64 nextHop = aodv->getNextHop(sinkAddress);
	if (!nextHop.equals(XBeeAddress64())) {
		voicePacketSender->generateVoicePacket();
	}
}

void sendTracePacket() {

	if ((*generateVoice).enabled) {
		voicePacketSender->sendTracePacket();
	}

}

void broadcastHeartbeat() {
	admissionControl->broadcastHeartBeat(voicePacketSender->getInjectionRate(), broadcastAddress,
			aodv->getNextHop(sinkAddress));
}

void sendPathPacket() {
	voiceStreamManager->sendPathPacket();
}

void runCalculateThroughput() {
	voiceStreamManager->calculateThroughput();
}

void clearBuffer() {
	while (Serial.available())
		Serial.read();
}

void listenForResponses() {
	admissionControl->checkTimers();

	if (xbee.readPacketNoTimeout(DEBUG)) {

		Rx64Response response;
		xbee.getResponse().getRx64Response(response);
		uint8_t* data = response.getData();

		if (xbee.getResponse().getApiId() == RX_64_RESPONSE && response.getRelativeDistance() < DISTANCE_THRESHOLD) {

			switch (data[0]) {
				case 'D':
					voicePacketSender->handleDataPacket(response);
					break;
				case 'P':
					voicePacketSender->handlePathPacket(response);
					break;
				case 'I':
					admissionControl->handleInitPacket(response);
					break;
				case 'R':
					admissionControl->handleREDJPacket(response);
					break;
				case 'G':
					admissionControl->handleGRANTPacket(response, (*sendInital).enabled, (*generateVoice).enabled);
					break;
				case 'T':
					voicePacketSender->handleTracePacket(response);
			}
		} else if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
			TxStatusResponse response;
			xbee.getResponse().getTxStatusResponse(response);
			uint8_t status = response.getStatus();
			if (status != 0) {
//				SerialUSB.print("TX_STATUS_ERROR: ");
//				SerialUSB.println(status);
			}
		}
	}
}

void setupThreads() {

	(*responseThread).ThreadName = "Packet Listener";
	(*responseThread).enabled = true;
	(*responseThread).setInterval(1);
	(*responseThread).onRun(listenForResponses);

	(*heartbeat).ThreadName = "Broadcast Heartbeat";
	(*heartbeat).enabled = true;
	(*heartbeat).setInterval(HEARTBEAT_INTERVAL + random(200));
	(*heartbeat).onRun(broadcastHeartbeat);

	//Set to true after receiving first data packet
	(*pathLoss).ThreadName = "Send Path Loss";
	(*pathLoss).enabled = false;
	(*pathLoss).setInterval(PATHLOSS_INTERVAL + random(200));
	(*pathLoss).onRun(sendPathPacket);

	//Set to true after being admitted by admission control
	(*generateVoice).ThreadName = "Send Voice Data";
	(*generateVoice).enabled = false;
	(*generateVoice).setInterval(VOICE_DATA_INTERVAL);
	(*generateVoice).onRun(sendVoicePacket);

	(*sendInital).ThreadName = "Send Voice Data";
	if (SENDER) {
		(*sendInital).enabled = true;
	} else {
		(*sendInital).enabled = false;
	}
	(*sendInital).setInterval(REQUEST_STREAM);
	(*sendInital).onRun(sendInitPacket);

	//Set to true after receiving first data packet
	(*calculateThroughput).ThreadName = "Calculate Throughput";
	(*calculateThroughput).enabled = false;
	(*calculateThroughput).setInterval(CALCULATE_THROUGHPUT_INTERVAL);
	(*calculateThroughput).onRun(runCalculateThroughput);

	(*threadMessage).ThreadName = "Thread Messages";
	(*threadMessage).enabled = SENDER;
	(*threadMessage).setInterval(TRACE_INTERVAL);
	(*threadMessage).onRun(sendTracePacket);

	controller.add(responseThread);
	controller.add(sendInital);
	controller.add(pathLoss);
	controller.add(calculateThroughput);
	controller.add(generateVoice);
	controller.add(heartbeat);
	controller.add(debugHeartbeatTable);
	controller.add(endMessage);
	controller.add(threadMessage);
}
