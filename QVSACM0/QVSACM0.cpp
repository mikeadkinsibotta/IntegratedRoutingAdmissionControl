// Do not remove the include below
#include "QVSACM0.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define VOICE_DATA_INTERVAL 2
#define SENDER false
#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define BROADCAST_ADDRESS_1 0x00000000
#define BROADCAST_ADDRESS_2 0x0000FFFF
#define PAYLOAD_SIZE 76
//#define BROADCAST_ADDRESS_1 0x0013A200
//#define BROADCAST_ADDRESS_2 0x40B317FA

const uint8_t NUM_MISSED_HB_BEFORE_PURGE = 30;

const float INITAL_DUPLICATION_SETTING = 0.0;
const uint8_t CODEC_SETTTING = 2;
const unsigned long REQUEST_STREAM = 3000;
const unsigned long GRANT_TIMEOUT_LENGTH = 300;
const unsigned long REJECT_TIMEOUT_LENGTH = 100;
const unsigned long HEARTBEAT_INTERVAL = 15000;
const unsigned long PATHLOSS_INTERVAL = 4000;
const unsigned long STREAM_DELAY_START = 5000;
unsigned long STREAM_DELAY_START_BEGIN = 0;

XBee xbee = XBee();
VoicePacketSender * voicePacketSender;
AdmissionControl * admissionControl;
VoiceStreamStatManager * voiceStreamStatManager;
AODV * aodv;

ThreadController controller = ThreadController();
Thread heartbeat = Thread();
Thread sendInital = Thread();
Thread responseThread = Thread();
Thread pathLoss = Thread();
Thread generateVoice = Thread();

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

	aodv = new AODV(xbee, myAddress, broadcastAddress, sinkAddress, CODEC_SETTTING, INITAL_DUPLICATION_SETTING);
	voiceStreamStatManager = new VoiceStreamStatManager(xbee, PAYLOAD_SIZE);
	voicePacketSender = new VoicePacketSender(xbee, aodv, &pathLoss, voiceStreamStatManager, myAddress, sinkAddress,
			CODEC_SETTTING, INITAL_DUPLICATION_SETTING, PAYLOAD_SIZE);
	admissionControl = new AdmissionControl(myAddress, sinkAddress, xbee, aodv, voiceStreamStatManager,
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

void startPathDiscovery() {

	if (millis() - STREAM_DELAY_START_BEGIN > STREAM_DELAY_START) {
		aodv->getRoute();
	}
}

void broadcastHeartbeat() {
	admissionControl->broadcastHeartBeat(voicePacketSender->getInjectionRate(), broadcastAddress,
			aodv->getNextHop(sinkAddress));
}

void sendVoicePacket() {
	voicePacketSender->generateVoicePacket();
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

	if (xbee.readPacketNoTimeout(DEBUG)) {
		if (xbee.getResponse().getApiId() == RX_64_RESPONSE) {
			Rx64Response response;
			xbee.getResponse().getRx64Response(response);
			uint8_t* data = response.getData();

			if (response.getRelativeDistance() < 4.00) {

				char control[5];

				for (int i = 0; i < 5; i++) {
					control[i] = data[i];
				}

				if (!strcmp(control, "RREQ") || !strcmp(control, "RREP")) {
					//routing data
					aodv->listenForResponses(response, control);
				} else if (!strcmp(control, "DATA")) {
					//voice data
					voicePacketSender->handleDataPacket(response);
				} else if (!strcmp(control, "PATH")) {
					//path loss packet
					voicePacketSender->handlePathPacket(response);
				} else if (!strcmp(control, "BEAT")) {
					admissionControl->receiveHeartBeat(voicePacketSender->getInjectionRate(), response);
				} else if (!strcmp(control, "INIT")) {
					admissionControl->handleInitPacket(response);
				} else if (!strcmp(control, "REDJ")) {
					admissionControl->handleREDJPacket(response);
				} else if (!strcmp(control, "GRNT")) {
					admissionControl->handleGRANTPacket(response, sendInital.enabled, generateVoice.enabled);
				} else if (!strcmp(control, "TRCE")) {
					voicePacketSender->handleTracePacket(response);
				}
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
	heartbeat.setInterval(HEARTBEAT_INTERVAL + random(100));
	heartbeat.onRun(broadcastHeartbeat);

	pathLoss.ThreadName = "Send Path Loss";
	pathLoss.enabled = false;
	pathLoss.setInterval(PATHLOSS_INTERVAL + random(100));
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
	sendInital.onRun(startPathDiscovery);

	controller.add(&responseThread);
	controller.add(&heartbeat);
	controller.add(&sendInital);
	controller.add(&pathLoss);
	controller.add(&generateVoice);

}
