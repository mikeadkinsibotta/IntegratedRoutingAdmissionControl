// Do not remove the include below
#include "AdminRouteACM2.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define VOICE_DATA_INTERVAL 3000
#define REQUEST_STREAM 12000
#define SENDER false
#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define HEARTBEAT_ADDRESS_1 0x00000000
#define HEARTBEAT_ADDRESS_2 0x0000FFFF
#define IGNORE_HEARTBEAT false
#define PAYLOAD_SIZE 76
//#define HEARTBEAT_ADDRESS_1 0x0013A200
//#define HEARTBEAT_ADDRESS_2 0x40B31805

const uint8_t NUM_MISSED_HB_BEFORE_PURGE = 3;

const float INITAL_DUPLICATION_SETTING = 0.0;
const uint8_t CODEC_SETTTING = 2;
const unsigned long GRANT_TIMEOUT_LENGTH = 5000;
const unsigned long REJECT_TIMEOUT_LENGTH = 4000;
const unsigned long HEARTBEAT_INTERVAL = 5000;

XBee xbee = XBee();
HeartbeatProtocol * heartbeatProtocol;
VoicePacketSender * voicePacketSender;
AdmissionControl * admissionControl;
VoiceStreamStatManager * voiceStreamStatManager;

ThreadController controller = ThreadController();
Thread heartbeat = Thread();
Thread sendInital = Thread();
Thread responseThread = Thread();
Thread pathLoss = Thread();
Thread generateVoice = Thread();

XBeeAddress64 heartBeatAddress = XBeeAddress64(HEARTBEAT_ADDRESS_1, HEARTBEAT_ADDRESS_2);
XBeeAddress64 sinkAddress = XBeeAddress64(SINK_ADDRESS_1, SINK_ADDRESS_2);
XBeeAddress64 myAddress;

void setup() {
	arduinoSetup();

	xbee.getMyAddress(myAddress, DEBUG);
	SerialUSB.print("My Address: ");
	myAddress.printAddressASCII(&SerialUSB);
	SerialUSB.println();

	voiceStreamStatManager = new VoiceStreamStatManager(xbee, PAYLOAD_SIZE);
	heartbeatProtocol = new HeartbeatProtocol(heartBeatAddress, myAddress, sinkAddress, xbee);
	voicePacketSender = new VoicePacketSender(xbee, heartbeatProtocol, &pathLoss, voiceStreamStatManager, myAddress,
			sinkAddress, CODEC_SETTTING, INITAL_DUPLICATION_SETTING, PAYLOAD_SIZE);
	admissionControl = new AdmissionControl(myAddress, sinkAddress, xbee, heartbeatProtocol, voiceStreamStatManager,
			GRANT_TIMEOUT_LENGTH, REJECT_TIMEOUT_LENGTH);
	setupThreads();

	digitalWrite(13, LOW);
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

	while (millis() - start < 8000) {

	}

}

void sendInitPacket() {
	admissionControl->sendInitPacket(CODEC_SETTTING, INITAL_DUPLICATION_SETTING);
}

void sendVoicePacket() {
	voicePacketSender->generateVoicePacket();
}

void broadcastHeartbeat() {
	if (millis() > 10000) {
		heartbeatProtocol->broadcastHeartBeat();
	}
}

void sendPathPacket() {
	voiceStreamStatManager->sendPathPacket();
}

void clearBuffer() {
	while (Serial.available())
		Serial.read();
}

void listenForResponses() {
	admissionControl->checkTimers();
	heartbeatProtocol->purgeNeighborhoodTable();

	if (xbee.readPacketNoTimeout(DEBUG)) {
		if (xbee.getResponse().getApiId() == RX_64_RESPONSE) {
			Rx64Response response;
			xbee.getResponse().getRx64Response(response);
			uint8_t* data = response.getData();

			char control[5];

			for (int i = 0; i < 5; i++) {
				control[i] = data[i];
			}

			if (!strcmp(control, "BEAT")) {
				//routing data
				heartbeatProtocol->receiveHeartBeat(response, IGNORE_HEARTBEAT);
			} else if (!strcmp(control, "DATA")) {
				//voice data
				voicePacketSender->handleDataPacket(response);
			} else if (!strcmp(control, "PATH")) {
				//path loss packet
				voicePacketSender->handlePathPacket(response);
			} else if (!strcmp(control, "RSTR")) {
				//voicePacketSender->handleStreamRestart(response);
			} else if (!strcmp(control, "INIT")) {
				admissionControl->handleInitPacket(response);
			} else if (!strcmp(control, "REDJ")) {
				admissionControl->handleREDJPacket(response);
			} else if (!strcmp(control, "GRNT")) {
				admissionControl->handleGRANTPacket(response, sendInital.enabled, generateVoice.enabled);
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
	heartbeat.setInterval(HEARTBEAT_INTERVAL + random(100));
	heartbeat.onRun(broadcastHeartbeat);

	heartbeatProtocol->setTimeoutLength((heartbeat.getInterval() * NUM_MISSED_HB_BEFORE_PURGE));

	pathLoss.ThreadName = "Send Path Loss";
	pathLoss.enabled = false;
	pathLoss.setInterval(3000 + random(100));
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

	controller.add(&responseThread);
	controller.add(&sendInital);
	controller.add(&pathLoss);
	controller.add(&generateVoice);
	controller.add(&heartbeat);
}
