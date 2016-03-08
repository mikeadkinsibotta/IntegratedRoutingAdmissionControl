// Do not remove the include below
#include "XBeeTX.h"

#define PAYLOAD_SIZE 76 /*If receiver mysteriously stops receiving data
packets but is receiving other packets, payload size likely the problem*/
#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define SENDER false
#define RREQ_ADDRESS_PART1 0x00000000
#define RREQ_ADDRESS_PART2 0x0000FFFF
//#define RREQ_ADDRESS_PART1 0x0013a200
//#define RREQ_ADDRESS_PART2 0x40B317FA
#define SINK_ADDRESS 0x40b519cc
#define VOICE_DATA_INTERVAL 2
#define SENDER_START 5000

AODV aodv;

unsigned long startTime = 0;
unsigned long currentTime = 0;
XBee xbee = XBee();
VoiceStreamStatManager voiceStreamStatManager(xbee, PAYLOAD_SIZE);
XBeeAddress64 sink = XBeeAddress64(0x0013a200, SINK_ADDRESS);
XBeeAddress64 broadcast = XBeeAddress64(RREQ_ADDRESS_PART1, RREQ_ADDRESS_PART2);

uint8_t frameId = 1;

uint8_t payload[PAYLOAD_SIZE] = { 0 };

uint8_t dupInc = 0;

unsigned long totalPacketsSent = 0;
unsigned long totalPacketsLost = 0;
unsigned long totalPacketsRecieved = 0;
unsigned long totalDupPackets = 0;

unsigned long trial = 0;

uint8_t packetLossRounded = 0;
double packetLoss = 0;

struct CompressionSettings settings;

Compression compressionTable;
AdmissionController admissionController;
ADPCM admcpm;
uint8_t code = 0;

float injectionRate = 0;

ThreadController controller = ThreadController();

Thread responseThread = Thread();
Thread cleanUpThread = Thread();
Thread sendData = Thread();
Thread dataRate = Thread();
Thread getRoute = Thread();
Thread pathLoss = Thread();
XBeeResponseThread xbeeResponseThread = XBeeResponseThread();

XBeeAddress64 myNextHop;
XBeeAddress64 myAddress;
XBeeAddress64 admissionSenderAddress;

//std::map<int, Frame> packetWait;

bool admissiontimerActive = false;
unsigned long admissionTimer = 0;

bool sinkTimerActive = false;
unsigned long sinkTimer = 0;
bool receivedReject = false;

#if SENDER
bool streamRequestTimerActive = false;
unsigned long streamRequestTimer = 0;

bool streamDeactivate = false;
unsigned long streamDeactivateTimer = 0;

#endif
void setup() {

	compressionTable.buildCompressionLookUpTable();
	//buildSaturationTable(satT);

	pinMode(STATUS_LED, OUTPUT);
	pinMode(ERROR_LED, OUTPUT);

	Serial.begin(111111);
	SerialUSB.begin(111111);

	xbee.setSerial(Serial);

	settings.codecSetting = 2;
	settings.dupSetting = 0;

	digitalWrite(13, HIGH);
	unsigned long start = millis();

#if SENDER

	while(millis() - start < SENDER_START) {

	}
	Serial.print("SENDER");
#else
	while (millis() - start < 5000) {

	}
#endif
	clearBuffer();

	myAddress = getMyAddress();
	admissionController = AdmissionController(sink, myAddress);

	getRoute.ThreadName = "Get Route";
#if SENDER
	getRoute.enabled = true;

#else
	getRoute.enabled = false;
#endif

	getRoute.setInterval(10000);
	getRoute.onRun(requestDataInjectionThread);

	responseThread.ThreadName = "Packet Listener";
	responseThread.enabled = true;
	responseThread.setInterval(1);
	responseThread.onRun(listenForResponses);

	cleanUpThread.ThreadName = "Purge Routing Table";
	cleanUpThread.enabled = false;
	cleanUpThread.setInterval(10);
	cleanUpThread.onRun(cleanUpRouteTable);

	sendData.ThreadName = "Send Voice Data";
	sendData.enabled = false;
	sendData.setInterval(VOICE_DATA_INTERVAL);
	sendData.onRun(loopSend);

	dataRate.ThreadName = "Broadcast Data Rate";
	dataRate.enabled = false;
	dataRate.setInterval(1000);
	dataRate.onRun(calculateTotalDataRate);

	pathLoss.ThreadName = "Send Path Loss";
	pathLoss.enabled = false;
	pathLoss.setInterval(1000);
	pathLoss.onRun(sendPathPacket);

	xbeeResponseThread.ThreadName = "XBee response Thread";
	xbeeResponseThread.enabled = false;
	xbeeResponseThread.setInterval(30);

	controller.add(&responseThread);
	controller.add(&dataRate);
	controller.add(&pathLoss);
	controller.add(&cleanUpThread);
	controller.add(&sendData);
	controller.add(&xbeeResponseThread);
	controller.add(&getRoute);

	aodv = AODV(xbee, myAddress, broadcast);
#if SENDER
	aodv.getRoute(sink);
#endif
	digitalWrite(13, LOW);
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
			// got a response!

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

	Serial.print("ThisAddress");
	address.printAddress(&Serial);

	return address;
}

/**
 * Nodes not sending a stream need to send check for responses
 */
void loop() {

//#if SENDER
	//while(millis() - trial < 120000) {
	//controller.run();
	//}
//#else
	controller.run();
//#endif

}

void loopSend() {

	int actualPayloadSize = 0;
	int i = 0;

	while (i <= PAYLOAD_SIZE) {

		switch (settings.codecSetting) {
			case 2:
				code = admcpm.twoBitEncode();
				payload[i] = code;
				i++;
				actualPayloadSize = i;
				break;
			case 3:
				if (i + 3 <= PAYLOAD_SIZE) {
					uint8_t buffer[3] = { 0 };
					admcpm.threeBitEncode(buffer);
					for (int j = 0; j < 3; ++j) {
						payload[i] = buffer[j];
						i++;
					}
					actualPayloadSize = i;
				} else {
					actualPayloadSize = i;
					i += 3;
				}
				break;
			case 4:
				code = admcpm.fourBitEncode();
				payload[i] = code;
				i++;
				actualPayloadSize = i;
				break;
			case 5:
				if (i + 5 <= PAYLOAD_SIZE) {
					uint8_t buffer[5] = { 0 };
					admcpm.fiveBitEncode(buffer);

					for (int j = 0; j < 5; ++j) {
						payload[i] = buffer[j];
						i++;
					}
					actualPayloadSize = i;
				} else {
					actualPayloadSize = i;
					i += 5;
				}
				break;
		}
	}

	uint8_t combinedSize = 0;
	uint8_t* combined = addDestinationToPayload(myAddress, sink, payload, actualPayloadSize, combinedSize, frameId,
			settings.codecSetting);

	Tx64Request tx = Tx64Request(myNextHop, ACK_OPTION, combined, combinedSize, 0);

	xbee.send(tx);

	dupInc++;
	frameId++;
	totalPacketsSent++;

	if (settings.dupSetting != 0 && floor(dupInc * settings.dupSetting) == (dupInc * settings.dupSetting)) {
		Serial.print("Duppp");
		Tx64Request duptx = Tx64Request(myNextHop, ACK_OPTION, combined, combinedSize, 0);
		xbee.send(duptx);
		//packetWait[frameId] = Frame(myAddress, millis());
		//frameId++;
		totalPacketsSent++;
		totalDupPackets++;
	}

	//free malloc data
	free(combined);

}

void listenForResponses() {
	checkTimers();

	if (xbee.readPacketNoTimeout(DEBUG)) {
		if (xbee.getResponse().getApiId() == RX_64_RESPONSE) {
			Rx64Response response;
			xbee.getResponse().getRx64Response(response);
			uint8_t* data = response.getData();

			char control[5];

			for (int i = 0; i < 5; i++) {
				control[i] = data[i];
			}

			if (!strcmp(control, "RREQ") || !strcmp(control, "RREP")) {
				//routing data
				handleAODVPacket(response, control);
			} else if (!strcmp(control, "DATA")) {
				//voice data
				handleDataPacket(response);
			} else if (!strcmp(control, "PATH")) {
				//path loss packet
				handlePathPacket(response);
			} else if (!strcmp(control, "ASM_")) {
				/*When I receive the neighborhood rate of a neighbor I update my neighborhood rate. */
				admissionController.updateNeighborRatesCalculateMyNeighborhoodRate(response);
			} else if (!strcmp(control, "INIT")) {
				handleInitPacket(response);
			} else if (!strcmp(control, "REDJ")) {
				handleREDJPacket(response);
			} else if (!strcmp(control, "GRNT")) {
				handleGRANTPacket(response);
			}
		}
	}
}

void handleAODVPacket(Rx64Response &response, const char control[]) {

	aodv.listenForResponses(response, control);

}

void handlePathPacket(const Rx64Response &response) {

	XBeeAddress64 packetSource;
	XBeeAddress64 nextHop;
	uint8_t * dataPtr = response.getData();

	packetSource.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	packetSource.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (!myAddress.equals(packetSource)) {

		if (aodv.hasRoute(packetSource, nextHop)) {
			Serial.print("ForwardPathPacket");

			Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, response.getData(), response.getDataLength(), 0);
			xbee.send(tx);
		} else {
			Serial.print("No Path");
		}
	} else {
		uint8_t dataLoss = dataPtr[13];

		//Returned to the orignal sender, update packet loss
		updateDataRate(dataLoss);

	}
}

void handleDataPacket(const Rx64Response &response) {

	//Extract the packet's final destination
	XBeeAddress64 packetDestination;
	XBeeAddress64 packetSource;
	XBeeAddress64 previousHop;

	uint8_t * dataPtr = response.getData();

	packetSource.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	packetSource.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	packetDestination.setMsb(
			(uint32_t(dataPtr[13]) << 24) + (uint32_t(dataPtr[14]) << 16) + (uint16_t(dataPtr[15]) << 8) + dataPtr[16]);

	packetDestination.setLsb(
			(uint32_t(dataPtr[17]) << 24) + (uint32_t(dataPtr[18]) << 16) + (uint16_t(dataPtr[19]) << 8) + dataPtr[20]);

	previousHop.setMsb(
			(uint32_t(response.getFrameData()[0]) << 24) + (uint32_t(response.getFrameData()[1]) << 16)
					+ (uint16_t(response.getFrameData()[2]) << 8) + response.getFrameData()[3]);
	previousHop.setLsb(
			(uint32_t(response.getFrameData()[4]) << 24) + (uint32_t(response.getFrameData()[5]) << 16)
					+ (uint16_t(response.getFrameData()[6]) << 8) + response.getFrameData()[7]);

//check to see if the packet final destination is this node's address
//If not setup another request to forward it.
	if (!myAddress.equals(packetDestination)) {
		//need to forward to next hop
		Serial.print("ForwardData");
		XBeeAddress64 nextHop;

		if (aodv.hasRoute(packetDestination, nextHop)) {
			uint8_t combinedSize = 0;
			Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, response.getData(), response.getDataLength(), 0);

			xbee.send(tx);

			uint8_t packetSize = response.getFrameDataLength() - 31;

			//Update Total Data Rate
			admissionController.updateFlowList(packetSource);

		} else if (!xbeeResponseThread.enabled) {
			xbeeResponseThread.setResponse(response);
			xbeeResponseThread._callback = handleDataPacket;
			xbeeResponseThread.setDestination(packetDestination);
			xbeeResponseThread.setNextHop(nextHop);
			xbeeResponseThread.onRun(getRouteWithResponse);
			xbeeResponseThread.enabled = true;
			aodv.getRoute(packetDestination);
		}
	} else {
		//voiceStreamStatManager.updateVoiceLoss(packetSource, dataPtr);
		pathLoss.enabled = true;
	}
}

void handleInitPacket(const Rx64Response &response) {
	Serial.print("ReceivedInitPacket");
	float injectionRateIncrease = 0;
	float contentionDomainRate = 0;
	XBeeAddress64 senderAddress;
	XBeeAddress64 nextHop;
	uint8_t* dataPtr = response.getData();
	senderAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	senderAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	nextHop.setMsb(
			(uint32_t(dataPtr[13]) << 24) + (uint32_t(dataPtr[14]) << 16) + (uint16_t(dataPtr[15]) << 8) + dataPtr[16]);

	nextHop.setLsb(
			(uint32_t(dataPtr[17]) << 24) + (uint32_t(dataPtr[18]) << 16) + (uint16_t(dataPtr[19]) << 8) + dataPtr[20]);

	contentionDomainRate = admissionController.updateContentionDomainIncrease(response, injectionRateIncrease);
	if (myAddress.equals(sink)) {
		sinkTimer = millis();
		sinkTimerActive = true;
		admissionSenderAddress = senderAddress;
		Serial.print("WaitingForREDJ");

	} else {
		if (!admissiontimerActive) {
			admissionTimer = millis();
			admissiontimerActive = true;

		}

		if (myAddress.equals(nextHop)) {
			Serial.print("PathToSink");
			if (aodv.hasRoute(sink, nextHop)) {
				contentionDomainRate = admissionController.requestToStream(xbee, senderAddress, nextHop,
						injectionRateIncrease);
			} else if (!xbeeResponseThread.enabled) {
				xbeeResponseThread.setResponse(response);
				xbeeResponseThread._callback = handleInitPacket;
				xbeeResponseThread.setDestination(nextHop);
				xbeeResponseThread.setNextHop(nextHop);
				xbeeResponseThread.enabled = true;
				aodv.getRoute(nextHop);
			} else {
				//drop packet
			}
		}
	}
}
void handleREDJPacket(Rx64Response &response) {
	Serial.print("ReceiveREDJ");
	XBeeAddress64 senderAddress;
	uint8_t * dataPtr = response.getData();

	senderAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	senderAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (!myAddress.equals(sink)) {
		sendREDJPacket(senderAddress);
	} else {
		receivedReject = true;
	}

}

void handleGRANTPacket(const Rx64Response &response) {
	XBeeAddress64 senderAddress;
	uint8_t * dataPtr = response.getData();

	senderAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	senderAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (!myAddress.equals(senderAddress)) {
		Serial.print("ForwardGRANT");
		sendGRANTPacket(senderAddress);
	} else {
#if SENDER
		Serial.print("OK_SEND_DATA!!");
		streamRequestTimerActive = false;
		streamRequestTimer = 0;
		sendData.enabled = true;
		trial = millis();
#endif
	}

}

/************************************ Utility Functions ***************************************************************/
void requestDataInjectionThread() {

//wait for multiple rrep responses so we choose the best path
	if (aodv.checkRouteTimer()) {
#if SENDER
		//if we still don't have a route try again.
		if(aodv.hasRoute(sink, myNextHop)) {

			Serial.print("HASROUTE");
			getRoute.enabled = false;
			injectionRate = 64.00 * (settings.codecSetting / 16.00) * (1.00 + settings.dupSetting);

			//make sure we don't send off multiple stream requests.
			if(!streamRequestTimerActive) {
				admissionController.requestToStream(xbee, myAddress, myNextHop, injectionRate);
			}
			streamRequestTimer = millis();
			streamRequestTimerActive = true;

		} else {
			//trying again.
			aodv.getRoute(sink);
		}
#endif
	}

}

void getRouteWithResponse() {
	XBeeAddress64 destination = xbeeResponseThread.getDestination();
	XBeeAddress64 nextHop = xbeeResponseThread.getNextHop();

//wait for multiple rrep responses so we choose the best path
	if (aodv.checkRouteTimer()) {
		if (aodv.hasRoute(destination, nextHop)) {
			xbeeResponseThread.enabled = false;
			xbeeResponseThread._callback(xbeeResponseThread.getResponse());
		} else {
			//trying again.
			aodv.getRoute(destination);
		}
	}

}

void getRouteWithAddress() {
	XBeeAddress64 destination = xbeeResponseThread.getDestination();
	XBeeAddress64 nextHop = xbeeResponseThread.getNextHop();

//wait for multiple rrep responses so we choose the best path
	if (aodv.checkRouteTimer()) {
		if (aodv.hasRoute(destination, nextHop)) {
			xbeeResponseThread.enabled = false;
			xbeeResponseThread._callbackAddress(xbeeResponseThread.getSenderAddress());
		} else {
			//trying again.
			Serial.print("GetRouteTryAgain");
			aodv.getRoute(destination);
		}
	}
}

void sendGRANTPacket(const XBeeAddress64 &senderAddress) {
	XBeeAddress64 nextHop;

	if (aodv.hasRoute(senderAddress, nextHop)) {
		uint8_t payload[] = { 'G', 'R', 'N', 'T', '\0', (senderAddress.getMsb() >> 24) & 0xff, (senderAddress.getMsb()
				>> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
				(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb()
						>> 8) & 0xff, senderAddress.getLsb() & 0xff };
		Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), 0);
		xbee.send(tx);
	} else if (!xbeeResponseThread.enabled) {
		xbeeResponseThread.setSenderAddress(senderAddress);
		xbeeResponseThread._callbackAddress = sendGRANTPacket;
		xbeeResponseThread.setDestination(sink);
		xbeeResponseThread.setNextHop(nextHop);
		xbeeResponseThread.onRun(getRouteWithAddress);
		xbeeResponseThread.enabled = true;
		aodv.getRoute(sink);
	} else {
		//drop packet
	}
}

void sendREDJPacket(const XBeeAddress64 &senderAddress) {

	XBeeAddress64 nextHop;
	if (aodv.hasRoute(sink, nextHop)) {
		uint8_t payload[] = { 'R', 'E', 'D', 'J', '\0', (senderAddress.getMsb() >> 24) & 0xff, (senderAddress.getMsb()
				>> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
				(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb()
						>> 8) & 0xff, senderAddress.getLsb() & 0xff };
		Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), 0);
		xbee.send(tx);
	} else if (!xbeeResponseThread.enabled) {
		xbeeResponseThread.setSenderAddress(senderAddress);
		xbeeResponseThread._callbackAddress = sendREDJPacket;
		xbeeResponseThread.setDestination(sink);
		xbeeResponseThread.setNextHop(nextHop);
		xbeeResponseThread.onRun(getRouteWithAddress);
		xbeeResponseThread.enabled = true;
		aodv.getRoute(sink);
	} else {
		//drop packet
	}
}

void sendPathPacket() {
	vector < VoiceStreamStats > streams = voiceStreamStatManager.getStreams();
	int i = 0;

	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		Serial.print("SENDPATH");
		const XBeeAddress64 &dataSenderAddress = streams.at(i).getSenderAddress();
		const uint8_t dataLoss = streams.at(i).getPacketLoss();

		streams.at(i).calculateThroughput();

		XBeeAddress64 nextHop;
		if (aodv.hasRoute(dataSenderAddress, nextHop)) {
			uint8_t payload[] = { 'P', 'A', 'T', 'H', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
					(dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
					dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff,
					(dataSenderAddress.getLsb() >> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff,
					dataSenderAddress.getLsb() & 0xff, dataLoss };
			Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), 0);
			xbee.send(tx);
		}
		i++;
	}
	voiceStreamStatManager.setStreams(streams);
}
void updateDataRate(const uint8_t dataLoss) {

	VoiceSetting * v = compressionTable.getCompressionTable();
	VoiceSetting newSetting = *(v + dataLoss);

	double dupSetting = newSetting.getDupRatio();
	settings.dupSetting = dupSetting;
	settings.codecSetting = newSetting.getCompressionSetting();

	injectionRate = 64.00 * (settings.codecSetting / 16.00) * (1.00 + settings.dupSetting);

}

void checkTimers() {

#if SENDER
	if(streamRequestTimerActive && millis() - streamRequestTimer > 2000) {
		Serial.print("StreamRequestRejected");
		streamRequestTimerActive = false;
		streamRequestTimer = 0;
	}

	if(streamDeactivate && millis() - streamDeactivateTimer > 10000) {
		streamDeactivate = false;
		streamDeactivateTimer = 0;

		//Restart Admission process
		getRoute.enabled = true;
	}

#endif
	if (admissiontimerActive && millis() - admissionTimer > 1000) {
		bool capacity = admissionController.checkLocalCapacity();
		if (!capacity) {
			sendREDJPacket(myAddress);
		}
		admissiontimerActive = false;
		admissionTimer = 0;
	}

	if (sinkTimerActive && millis() - sinkTimer > 1000 && !receivedReject) {
		Serial.print("SendGRANT");
		voiceStreamStatManager.removeStream(admissionSenderAddress);
		sendGRANTPacket(admissionSenderAddress);
		sinkTimerActive = false;
		sinkTimer = 0;
		receivedReject = false;
	} else if (millis() - sinkTimer > 1000 && receivedReject) {
		Serial.print("RECIVEDALLREDJ");
		sinkTimerActive = false;
		sinkTimer = 0;
		receivedReject = false;
	}

}

void calculateTotalDataRate() {
	admissionController.setInjectionRate(injectionRate);
//Broadcast my neighborhood rate
	admissionController.sendDatasendInjectionRateAndContentionDomainRate(xbee);
}

void cleanUpRouteTable() {
	Serial.print("Purge Routing Table");
	aodv.purgeExpiredRoutes();
}

uint8_t* addDestinationToPayload(XBeeAddress64 packetSource, XBeeAddress64 packetDestination, uint8_t payload[],
		uint8_t sizePayload, uint8_t& resultSize, uint8_t& frameId, uint8_t& codec) {

	uint8_t destination[] = { 'D', 'A', 'T', 'A', '\0', (packetSource.getMsb() >> 24) & 0xff, (packetSource.getMsb()
			>> 16) & 0xff, (packetSource.getMsb() >> 8) & 0xff, packetSource.getMsb() & 0xff, (packetSource.getLsb()
			>> 24) & 0xff, (packetSource.getLsb() >> 16) & 0xff, (packetSource.getLsb() >> 8) & 0xff,
			packetSource.getLsb() & 0xff, (packetDestination.getMsb() >> 24) & 0xff, (packetDestination.getMsb() >> 16)
					& 0xff, (packetDestination.getMsb() >> 8) & 0xff, packetDestination.getMsb() & 0xff,
			(packetDestination.getLsb() >> 24) & 0xff, (packetDestination.getLsb() >> 16) & 0xff,
			(packetDestination.getLsb() >> 8) & 0xff, packetDestination.getLsb() & 0xff, frameId, codec };

	uint8_t* result = (uint8_t*) malloc(sizeof(destination) + sizePayload);
	resultSize = sizeof(destination) + sizePayload;
	memcpy(result, destination, sizeof(destination));
	memcpy(result + sizeof(destination), payload, sizePayload);

	return result;

}

void clearBuffer() {
	while (Serial.available())
		Serial.read();
}
