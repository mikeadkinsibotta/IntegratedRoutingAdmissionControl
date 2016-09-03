// Do not remove the include below
#include "AdminRouteACM0.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define VOICE_DATA_INTERVAL 2
#define SENDER false
#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B317F6
#define HEARTBEAT_ADDRESS_1 0x00000000
#define HEARTBEAT_ADDRESS_2 0x0000FFFF
#define MANIPULATE false
#define MANIPULATE_ADDRESS_1 0x00000000
#define MANIPULATE_ADDRESS_2 0x0000FFFF
#define PAYLOAD_SIZE 76
//#define HEARTBEAT_ADDRESS_1 0x0013A200
//#define HEARTBEAT_ADDRESS_2 0x40B317F6

const uint8_t NUM_MISSED_HB_BEFORE_PURGE = 2;

const float INITAL_DUPLICATION_SETTING = 0.0;
const uint8_t CODEC_SETTTING = 2;
const uint8_t TRACE_INTERVAL = 2000;
const unsigned long REQUEST_STREAM = 1000;
const unsigned long GRANT_TIMEOUT_LENGTH = 100;
const unsigned long REJECT_TIMEOUT_LENGTH = 10;
const unsigned long HEARTBEAT_INTERVAL = 10000;
const unsigned long PATHLOSS_INTERVAL = 16000;
const unsigned long CALCULATE_THROUGHPUT_INTERVAL = 8000;
const unsigned long STREAM_DELAY_START = 5000;
const unsigned long INIT_MESSAGE_RESTART = 10000;
unsigned long STREAM_DELAY_START_BEGIN = 0;
bool NEXT_TIME = false;

XBee xbee = XBee();
HeartbeatProtocol * heartbeatProtocol;
VoicePacketSender * voicePacketSender;
AdmissionControl * admissionControl;
VoiceStreamManager * voiceStreamManager;

ThreadController controller = ThreadController();
Thread heartbeat = Thread();
Thread sendInital = Thread();
Thread responseThread = Thread();
Thread pathLoss = Thread();
Thread generateVoice = Thread();
Thread calculateThroughput = Thread();
Thread initialRestart = Thread();

XBeeAddress64 heartBeatAddress = XBeeAddress64(HEARTBEAT_ADDRESS_1, HEARTBEAT_ADDRESS_2);
XBeeAddress64 manipulateAddress = XBeeAddress64(MANIPULATE_ADDRESS_1, MANIPULATE_ADDRESS_2);
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
	heartbeatProtocol = new HeartbeatProtocol(heartBeatAddress, manipulateAddress, MANIPULATE, myAddress, sinkAddress,
			xbee);
	voicePacketSender = new VoicePacketSender(xbee, heartbeatProtocol, &pathLoss, &calculateThroughput,
			voiceStreamManager, myAddress, sinkAddress, CODEC_SETTTING, INITAL_DUPLICATION_SETTING, PAYLOAD_SIZE,
			TRACE_INTERVAL);
	admissionControl = new AdmissionControl(myAddress, sinkAddress, xbee, heartbeatProtocol, voiceStreamManager,
			voicePacketSender, GRANT_TIMEOUT_LENGTH, REJECT_TIMEOUT_LENGTH);
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
		admissionControl->sendInitPacket(CODEC_SETTTING, INITAL_DUPLICATION_SETTING);
	}
}

void sendVoicePacket() {
	Neighbor nextHop = heartbeatProtocol->getNextHop();
	if (nextHop.equals(Neighbor())) {
		SerialUSB.println("Lost NextHop");
		generateVoice.enabled = false;
		initialRestart.enabled = true;
	} else {
		voicePacketSender->generateVoicePacket();
	}
}

void runInitialRestart() {

	if (NEXT_TIME) {
		SerialUSB.println("Countdown complete");
		sendInital.enabled = true;
		NEXT_TIME = false;
		initialRestart.enabled = false;

	} else {
		SerialUSB.println("Starting 8 second countdown");
		NEXT_TIME = true;
	}
}

void broadcastHeartbeat() {
	if (millis() > 10000) {
		heartbeatProtocol->broadcastHeartBeat();
	}
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
	heartbeatProtocol->purgeNeighborhoodTable();

	if (xbee.readPacketNoTimeout(DEBUG)) {

		Rx64Response response;
		xbee.getResponse().getRx64Response(response);
		uint8_t* data = response.getData();

		if (xbee.getResponse().getApiId() == RX_64_RESPONSE && response.getRelativeDistance() < 2.50) {

			switch (data[0]) {
				case 'B':
					heartbeatProtocol->receiveHeartBeat(response);
					break;
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
					admissionControl->handleGRANTPacket(response, sendInital.enabled, generateVoice.enabled);
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

	responseThread.ThreadName = "Packet Listener";
	responseThread.enabled = true;
	responseThread.setInterval(1);
	responseThread.onRun(listenForResponses);

	heartbeat.ThreadName = "Broadcast Heartbeat";
	heartbeat.enabled = true;
	heartbeat.setInterval(HEARTBEAT_INTERVAL + random(200));
	heartbeat.onRun(broadcastHeartbeat);
	heartbeatProtocol->setTimeoutLength((heartbeat.getInterval() * NUM_MISSED_HB_BEFORE_PURGE));

	pathLoss.ThreadName = "Send Path Loss";
	pathLoss.enabled = false;
	pathLoss.setInterval(PATHLOSS_INTERVAL + random(200));
	pathLoss.onRun(sendPathPacket);

	generateVoice.ThreadName = "Send Voice Data";
	generateVoice.enabled = false;
	generateVoice.setInterval(VOICE_DATA_INTERVAL);
	generateVoice.onRun(sendVoicePacket);

	sendInital.ThreadName = "Send Voice Data";
	if (SENDER) {
		sendInital.enabled = true;
	} else {
		sendInital.enabled = false;
	}
	sendInital.setInterval(REQUEST_STREAM);
	sendInital.onRun(sendInitPacket);

	calculateThroughput.ThreadName = "Calculate Throughput";
	calculateThroughput.enabled = false;
	calculateThroughput.setInterval(CALCULATE_THROUGHPUT_INTERVAL);
	calculateThroughput.onRun(runCalculateThroughput);

	initialRestart.ThreadName = "Start initial restart";
	initialRestart.enabled = false;
	initialRestart.setInterval(INIT_MESSAGE_RESTART);
	initialRestart.onRun(runInitialRestart);

	controller.add(&responseThread);
	controller.add(&sendInital);
	controller.add(&pathLoss);
	controller.add(&calculateThroughput);
	controller.add(&generateVoice);
	controller.add(&heartbeat);
	controller.add(&initialRestart);
}
