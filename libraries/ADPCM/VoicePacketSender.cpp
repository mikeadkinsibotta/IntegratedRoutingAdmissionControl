/*
 * VoicePacket.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: mike
 */
#include <VoicePacketSender.h>
#define PAYLOAD_SIZE 76

VoicePacketSender::VoicePacketSender() {
	codecSetting = 2;
	dupSetting = 0.0;
	admcpm = ADPCM();
	myAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	frameId = 0;
	myNextHop = XBeeAddress64();
	heartbeatProtocol = 0;
	xbee = XBee();
	voiceStreamStatManager = VoiceStreamStatManager(xbee, PAYLOAD_SIZE);
	pathLoss = 0;
	injectionRate = 0;
	compressionTable.buildCompressionLookUpTable();

}

VoicePacketSender::VoicePacketSender(XBee& xbee, HeartbeatProtocol * heartbeatProtocol, Thread * pathLoss,
		const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const uint8_t codecSetting,
		const float dupSetting) {

	this->codecSetting = codecSetting;
	this->dupSetting = dupSetting;
	admcpm = ADPCM();
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	frameId = 0;
	injectionRate = 0;
	myNextHop = XBeeAddress64();

	//If I don't past the pointer, it just makes a copy of the heartbeat protocol during assignment,
	//If I make changes to the heartbeat protocol outside the class the member variable does not pick up the changes.
	//Thats why I need the pointer.
	this->heartbeatProtocol = heartbeatProtocol;

	this->xbee = xbee;
	voiceStreamStatManager = VoiceStreamStatManager(xbee, PAYLOAD_SIZE);
	this->pathLoss = pathLoss;
	compressionTable.buildCompressionLookUpTable();

}

void VoicePacketSender::generateVoicePacket() {

	bool hasNextHop = heartbeatProtocol->isRouteFlag();

	if (hasNextHop) {

		injectionRate = 64.00 * (codecSetting / 16.00) * (1.00 + dupSetting);
		heartbeatProtocol->setDataRate(injectionRate);

		if (myNextHop.equals(XBeeAddress64())) {
			SerialUSB.println("Has Route to Sink");
			myNextHop = heartbeatProtocol->getNextHopAddress();
			sendStreamRestart (myAddress);
		}

		uint8_t payload[PAYLOAD_SIZE] = { 0 };
		uint8_t code = 0;
		int actualPayloadSize = 0;
		int i = 0;

		while (i <= PAYLOAD_SIZE) {

			switch (codecSetting) {
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
		uint8_t* combined = addDestinationToPayload(myAddress, sinkAddress, payload, actualPayloadSize, combinedSize,
				frameId);

		Tx64Request tx = Tx64Request(myNextHop, combined, combinedSize);

		xbee.send(tx);
		frameId++;

		if (dupSetting != 0 && floor(frameId * dupSetting) == (frameId * dupSetting)) {
			xbee.send(tx);
			frameId++;
		}

		//free malloc data
		free(combined);
	} else {
		myNextHop = XBeeAddress64();
	}
}

void VoicePacketSender::handleDataPacket(const Rx64Response &response) {

	myNextHop = heartbeatProtocol->getNextHopAddress();

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

		voiceStreamStatManager.updateStreamsIntermediateNode(packetSource, previousHop);

		Tx64Request tx = Tx64Request(myNextHop, response.getData(), response.getDataLength());

		xbee.send(tx);

		//Update Total Data Rate
		//admissionController.updateFlowList(packetSource);

	} else {
		voiceStreamStatManager.updateVoiceLoss(packetSource, previousHop, dataPtr);
		(*pathLoss).enabled = true;
	}

}

void VoicePacketSender::handlePathPacket(const Rx64Response &response) {

	XBeeAddress64 packetSource;

	uint8_t * dataPtr = response.getData();

	packetSource.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	packetSource.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (!myAddress.equals(packetSource)) {

		//if (aodv.hasRoute(packetSource, nextHop)) {
		XBeeAddress64 nextHop;
		voiceStreamStatManager.getStreamPreviousHop(packetSource, nextHop);

		Serial.print("ForwardPathPacket");

		Tx64Request tx = Tx64Request(nextHop, response.getData(), response.getDataLength());
		xbee.send(tx);
		//} else {
		//	Serial.print("No Path");
		//}
	} else {
		Serial.println("Received Path Packet");
		uint8_t dataLoss = dataPtr[13];
		uint8_t totalPacketSent = dataPtr[14];
		uint8_t totalPacketReceived = dataPtr[15];

		SerialUSB.print("TotalPacketSent: ");
		SerialUSB.print(totalPacketSent);
		SerialUSB.print(" TotalPacketReceived: ");
		SerialUSB.print(totalPacketReceived);

		//Returned to the orignal sender, update packet loss
		updateDataRate(dataLoss);

	}
}

uint8_t* VoicePacketSender::addDestinationToPayload(const XBeeAddress64& packetSource,
		const XBeeAddress64& packetDestination, const uint8_t * payload, const uint8_t sizePayload, uint8_t& resultSize,
		const uint8_t frameId) {

	uint8_t destination[] = { 'D', 'A', 'T', 'A', '\0', (packetSource.getMsb() >> 24) & 0xff, (packetSource.getMsb()
			>> 16) & 0xff, (packetSource.getMsb() >> 8) & 0xff, packetSource.getMsb() & 0xff, (packetSource.getLsb()
			>> 24) & 0xff, (packetSource.getLsb() >> 16) & 0xff, (packetSource.getLsb() >> 8) & 0xff,
			packetSource.getLsb() & 0xff, (packetDestination.getMsb() >> 24) & 0xff, (packetDestination.getMsb() >> 16)
					& 0xff, (packetDestination.getMsb() >> 8) & 0xff, packetDestination.getMsb() & 0xff,
			(packetDestination.getLsb() >> 24) & 0xff, (packetDestination.getLsb() >> 16) & 0xff,
			(packetDestination.getLsb() >> 8) & 0xff, packetDestination.getLsb() & 0xff, frameId, codecSetting };

	uint8_t* result = (uint8_t*) malloc(sizeof(destination) + sizePayload);
	resultSize = sizeof(destination) + sizePayload;
	memcpy(result, destination, sizeof(destination));
	memcpy(result + sizeof(destination), payload, sizePayload);

	return result;

}

void VoicePacketSender::sendStreamRestart(const XBeeAddress64& dataSenderAddress) {

	uint8_t payload[] = { 'R', 'S', 'T', 'R', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
			(dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
			dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff, (dataSenderAddress.getLsb()
					>> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff, dataSenderAddress.getLsb() & 0xff };
	Tx64Request tx = Tx64Request(myNextHop, payload, sizeof(payload));
	xbee.send(tx);

}

void VoicePacketSender::updateDataRate(const uint8_t dataLoss) {

	SerialUSB.print(" DataLoss: ");
	SerialUSB.println(dataLoss);

	VoiceSetting * v = compressionTable.getCompressionTable();
	VoiceSetting newSetting = *(v + dataLoss);

	dupSetting = newSetting.getDupRatio();
	codecSetting = newSetting.getCompressionSetting();

}

void VoicePacketSender::requestToStream() {

	XBeeAddress64 heartbeatAddress = heartbeatProtocol->getBroadcastAddress();

	uint8_t * injectionRateP = (uint8_t *) &injectionRate;

	uint8_t payloadBroadCast[] = { 'I', 'N', 'I', 'T', '\0', (myAddress.getMsb() >> 24) & 0xff, (myAddress.getMsb()
			>> 16) & 0xff, (myAddress.getMsb() >> 8) & 0xff, myAddress.getMsb() & 0xff, (myAddress.getLsb() >> 24)
			& 0xff, (myAddress.getLsb() >> 16) & 0xff, (myAddress.getLsb() >> 8) & 0xff, myAddress.getLsb() & 0xff,
			myNextHop.getMsb() >> 24, myNextHop.getMsb() >> 16, myNextHop.getMsb() >> 8, myNextHop.getMsb(),
			myNextHop.getLsb() >> 24, myNextHop.getLsb() >> 16, myNextHop.getLsb() >> 8, myNextHop.getLsb(),
			injectionRateP[0], injectionRateP[1], injectionRateP[2], injectionRateP[3] };
	Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));
	// send the command
	xbee.send(tx);

}

void VoicePacketSender::handleInitPacket(const Rx64Response &response) {
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

}

void VoicePacketSender::sendPathPacket() {
	voiceStreamStatManager.sendPathPacket();
}

void VoicePacketSender::handleStreamRestart(const Rx64Response &response) {
	voiceStreamStatManager.handleStreamRestart(response);
}

const XBeeAddress64& VoicePacketSender::getMyNextHop() const {
	return myNextHop;
}

void VoicePacketSender::setMyNextHop(const XBeeAddress64& myNextHop) {
	this->myNextHop = myNextHop;
}

const XBeeAddress64& VoicePacketSender::getSinkAddress() const {
	return sinkAddress;
}

void VoicePacketSender::setSinkAddress(const XBeeAddress64& sinkAddress) {
	this->sinkAddress = sinkAddress;
}

uint8_t VoicePacketSender::getCodecSetting() const {
	return codecSetting;
}

void VoicePacketSender::setCodecSetting(uint8_t codecSetting) {
	this->codecSetting = codecSetting;
}

float VoicePacketSender::getDupSetting() const {
	return dupSetting;
}

void VoicePacketSender::setDupSetting(float dupSetting) {
	this->dupSetting = dupSetting;
}

