// Do not remove the include below
#include "AdminRouteACM1.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define SENDER true
#define HEARTBEAT_ADDRESS_1 0x00000000
#define HEARTBEAT_ADDRESS_2 0x0000FFFF
#define MANIPULATE false
#define MANIPULATE_ADDRESS_1 0x0013A200
#define MANIPULATE_ADDRESS_2 0x4102FC32
#define DEBUG_HEARTBEAT false
//#define HEARTBEAT_ADDRESS_1 0x0013A200
//#define HEARTBEAT_ADDRESS_2 0x40B317F6

const uint8_t NUM_MISSED_HB_BEFORE_PURGE = 6;

const float INITAL_DUPLICATION_SETTING = 0.0;
const uint8_t CODEC_SETTTING = 2;
const unsigned long TRACE_INTERVAL = 10000;
const unsigned long END_TIME = 200;
const uint8_t PAYLOAD_SIZE = 76;
const uint8_t VOICE_DATA_INTERVAL = 2;
const unsigned long REQUEST_STREAM = 100;
const unsigned long GRANT_TIMEOUT_LENGTH = 30;
const unsigned long REJECT_TIMEOUT_LENGTH = 8;
const unsigned long HEARTBEAT_INTERVAL = 500;
const unsigned long PATHLOSS_INTERVAL = 10000;
const unsigned long CALCULATE_THROUGHPUT_INTERVAL = 8000;
const unsigned long STREAM_DELAY_START = 5000;
const unsigned long DEBUG_HEARTBEAT_TABLE = 10000;
const float DISTANCE_THRESHOLD = 1.80;
unsigned long STREAM_DELAY_START_BEGIN = 0;
const float DIFFERENCE_DISTANCE = 0.60;
const bool IS_SINK = false;

bool endMessageSent = false;
uint8_t nextHopSwitchListSize = 0;
uint8_t nextHopSwitchListIndex = 0;

XBee xbee = XBee();
HeartbeatProtocol * heartbeatProtocol;
VoicePacketSender * voicePacketSender;
AdmissionControl * admissionControl;
VoiceStreamManager * voiceStreamManager;

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

XBeeAddress64 heartBeatAddress = XBeeAddress64(HEARTBEAT_ADDRESS_1, HEARTBEAT_ADDRESS_2);
XBeeAddress64 manipulateAddress = XBeeAddress64(MANIPULATE_ADDRESS_1, MANIPULATE_ADDRESS_2);
XBeeAddress64 myAddress;

void setup() {
	arduinoSetup();

	clearBuffer();
	xbee.getMyAddress(myAddress, DEBUG);
	SerialUSB.print("My Address: ");
	myAddress.printAddressASCII(&SerialUSB);
	SerialUSB.println();

	voiceStreamManager = new VoiceStreamManager(xbee, PAYLOAD_SIZE);
	heartbeatProtocol = new HeartbeatProtocol(heartBeatAddress, manipulateAddress, MANIPULATE, myAddress, xbee, SENDER,
			DIFFERENCE_DISTANCE, IS_SINK);
	voicePacketSender = new VoicePacketSender(xbee, heartbeatProtocol, pathLoss, calculateThroughput,
			voiceStreamManager, myAddress, CODEC_SETTTING, INITAL_DUPLICATION_SETTING, PAYLOAD_SIZE);
	admissionControl = new AdmissionControl(myAddress, xbee, heartbeatProtocol, voiceStreamManager, voicePacketSender,
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
		admissionControl->sendInitPacket(CODEC_SETTTING, INITAL_DUPLICATION_SETTING);
	}
}

void sendVoicePacket() {

	Neighbor nextHop = heartbeatProtocol->getNextHop();
	if (!nextHop.equals(Neighbor())) {
		voicePacketSender->generateVoicePacket();
	}
}

void sendTracePacket() {

	if ((*generateVoice).enabled) {
		voicePacketSender->sendTracePacket();
	}

}

void broadcastHeartbeat() {
	if (millis() > 10000) {
		heartbeatProtocol->broadcastHeartBeat();
	}
	double timepoint = millis() / 1000.0;
	if (timepoint > END_TIME && !endMessageSent) {

		//kill Sender
		if (!IS_SINK) {
			nextHopSwitchListSize = heartbeatProtocol->getNextHopSwitchList().size();
			(*endMessage).enabled = true;

		}
		controller.remove(sendInital);
		controller.remove(generateVoice);
		controller.remove(debugHeartbeatTable);
		controller.remove(threadMessage);
		endMessageSent = true;
	}
}

void sendEndMessage() {
	if (nextHopSwitchListIndex < nextHopSwitchListSize) {
		heartbeatProtocol->sendEndMessage(nextHopSwitchListIndex);
		nextHopSwitchListIndex++;
	} else {
		(*endMessage).enabled = false;
	}
}

void debugHeartbeat() {
	heartbeatProtocol->printNeighborHoodTable();
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

		if (xbee.getResponse().getApiId() == RX_64_RESPONSE && response.getRelativeDistance() < DISTANCE_THRESHOLD) {

			switch (data[0]) {
				case 'E':
					heartbeatProtocol->handleEndPacket(response);
					break;
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
	heartbeatProtocol->setTimeoutLength(((*heartbeat).getInterval() * NUM_MISSED_HB_BEFORE_PURGE) + 250);

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

	(*debugHeartbeatTable).ThreadName = "Debug Heartbeat";
	(*debugHeartbeatTable).enabled = DEBUG_HEARTBEAT;
	(*debugHeartbeatTable).setInterval(DEBUG_HEARTBEAT_TABLE);
	(*debugHeartbeatTable).onRun(debugHeartbeat);

	(*endMessage).ThreadName = "Send End Messages";
	(*endMessage).enabled = false;
	(*endMessage).setInterval(500);
	(*endMessage).onRun(sendEndMessage);

	(*threadMessage).ThreadName = "Thread Messages";
	(*threadMessage).enabled = false;
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
