// Do not remove the include below
#include "AdminRoute.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false

XBee xbee = XBee();
HeartbeatProtocol heartbeatProtocol = HeartbeatProtocol(xbee);

ThreadController controller = ThreadController();
Thread heartbeat = Thread();
Thread responseThread = Thread();

void setup() {
	arduinoSetup();

	//XBeeAddress64 myAddress = getMyAddress();

	setupThreads();
}

void loop() {
	controller.run();
}

void arduinoSetup() {
	pinMode(STATUS_LED, OUTPUT);
	pinMode(ERROR_LED, OUTPUT);

	Serial.begin(111111);
	SerialUSB.begin(111111);

	xbee.setSerial(Serial);

	digitalWrite(13, HIGH);
	unsigned long start = millis();

	clearBuffer();

	while (millis() - start < 5000) {

	}

}

void broadcastHeartbeat() {
	heartbeatProtocol.broadcastHeartBeat();

}

void clearBuffer() {
	while (Serial.available())
		Serial.read();
}

XBeeAddress64 getMyAddress() {
	XBeeAddress64 address = XBeeAddress64();

	uint8_t shCmd[] = { 'S', 'H' };

	// serial low
	uint8_t slCmd[] = { 'S', 'L' };

	AtCommandRequest atCommandRequest = AtCommandRequest(shCmd);

	xbee.send(atCommandRequest);
	atCommandRequest.setCommand(slCmd);
	xbee.send(atCommandRequest);
	uint8_t notfound = 0;

	while (notfound < 2) {
		if (xbee.readPacket(1000, DEBUG)) {

			// should be an AT command response
			if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
				AtCommandResponse atResponse = AtCommandResponse();
				xbee.getResponse().getAtCommandResponse(atResponse);

				if (atResponse.isOk()) {
					if (!notfound) {
						address.setMsb(
								(atResponse.getValue()[0] << 24) + (atResponse.getValue()[1] << 16)
										+ (atResponse.getValue()[2] << 8) + atResponse.getValue()[3]);
						++notfound;
					} else {
						address.setLsb(
								(atResponse.getValue()[0] << 24) + (atResponse.getValue()[1] << 16)
										+ (atResponse.getValue()[2] << 8) + atResponse.getValue()[3]);
						++notfound;
					}
				}
			}
		}
	}

	SerialUSB.print("ThisAddress");
	address.printAddress(&SerialUSB);

	return address;
}

void listenForResponses() {
	//checkTimers();

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
				heartbeatProtocol.receiveHeartBeat(response);
			} /*else if(!strcmp(control, "DATA")) {
			 //voice data
			 handleDataPacket(response);
			 } else if(!strcmp(control, "PATH")) {
			 //path loss packet
			 handlePathPacket(response);
			 } else if(!strcmp(control, "ASM_")) {
			 /*When I receive the neighborhood rate of a neighbor I update my neighborhood rate.
			 admissionController.updateNeighborRatesCalculateMyNeighborhoodRate(response);
			 } else if(!strcmp(control, "INIT")) {
			 handleInitPacket(response);
			 } else if(!strcmp(control, "REDJ")) {
			 handleREDJPacket(response);
			 } else if(!strcmp(control, "GRNT")) {
			 handleGRANTPacket(response);
			 }*/
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
	heartbeat.setInterval(1000);
	heartbeat.onRun(broadcastHeartbeat);

	controller.add(&heartbeat);
	controller.add(&responseThread);
}
