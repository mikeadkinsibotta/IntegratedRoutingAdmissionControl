/*
 * VoicePacket.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: mike
 */
#include <VoicePacketSender.h>

VoicePacketSender::VoicePacketSender() {
	codecSetting = 2;
	dupSetting = 0.0;
	admcpm = ADPCM();
	senderAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	frameId = 0;
	myNextHop = XBeeAddress64();
	heartbeatProtocol = 0;
	xbee = XBee();
	voiceStreamStatManager = 0;
	pathLoss = 0;
	injectionRate = 0;
	payloadSize = 0;
	compressionTable.buildCompressionLookUpTable();

}

VoicePacketSender::VoicePacketSender(XBee& xbee, HeartbeatProtocol * heartbeatProtocol, Thread * pathLoss,
		VoiceStreamStatManager * voiceStreamStatManager, const XBeeAddress64& myAddress,
		const XBeeAddress64& sinkAddress, const uint8_t codecSetting, const float dupSetting,
		const uint8_t payloadSize) {

	this->codecSetting = codecSetting;
	this->dupSetting = dupSetting;
	admcpm = ADPCM();
	this->senderAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->voiceStreamStatManager = voiceStreamStatManager;
	this->payloadSize = payloadSize;
	frameId = 0;
	injectionRate = 0;
	myNextHop = XBeeAddress64();

	//If I don't past the pointer, it just makes a copy of the heartbeat protocol during assignment,
	//If I make changes to the heartbeat protocol outside the class the member variable does not pick up the changes.
	//Thats why I need the pointer.
	this->heartbeatProtocol = heartbeatProtocol;

	this->xbee = xbee;
	this->pathLoss = pathLoss;

	compressionTable.buildCompressionLookUpTable();

}

void VoicePacketSender::generateVoicePacket() {

	injectionRate = 64.00 * (codecSetting / 16.00) * (1.00 + dupSetting);

	heartbeatProtocol->setDataRate(injectionRate);
	myNextHop = heartbeatProtocol->getNextHop().getAddress();

	//uint8_t * payload = (uint8_t*) malloc(sizeof(uint8_t) * payloadSize);
	uint8_t code = 0;
	int actualPayloadSize = 0;
	int i = 0;

	while (i <= payloadSize) {

		switch (codecSetting) {
			case 2:
				code = admcpm.twoBitEncode();
				//payload[i] = code;
				i++;
				actualPayloadSize = i;
				break;
			case 3:
				if (i + 3 <= payloadSize) {
					uint8_t buffer[3] = { 0 };
					admcpm.threeBitEncode(buffer);
					for (int j = 0; j < 3; ++j) {
						//payload[i] = buffer[j];
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
				//payload[i] = code;
				i++;
				actualPayloadSize = i;
				break;
			case 5:
				if (i + 5 <= payloadSize) {
					uint8_t buffer[5] = { 0 };
					admcpm.fiveBitEncode(buffer);

					for (int j = 0; j < 5; ++j) {
						//payload[i] = buffer[j];
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

	uint8_t destination[100] = { 'D', 'A', 'T', 'A', '\0', (senderAddress.getMsb() >> 24) & 0xff,
			(senderAddress.getMsb() >> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
			(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb() >> 8)
					& 0xff, senderAddress.getLsb() & 0xff, (sinkAddress.getMsb() >> 24) & 0xff, (sinkAddress.getMsb()
					>> 16) & 0xff, (sinkAddress.getMsb() >> 8) & 0xff, sinkAddress.getMsb() & 0xff,
			(sinkAddress.getLsb() >> 24) & 0xff, (sinkAddress.getLsb() >> 16) & 0xff, (sinkAddress.getLsb() >> 8)
					& 0xff, sinkAddress.getLsb() & 0xff, frameId, codecSetting };

	Tx64Request tx = Tx64Request(myNextHop, destination, sizeof(destination));

	xbee.send(tx);
	frameId++;

	if (dupSetting != 0 && floor(frameId * dupSetting) == (frameId * dupSetting)) {
		xbee.send(tx);
		frameId++;
	}

}

void VoicePacketSender::handleDataPacket(const Rx64Response &response) {

	myNextHop = heartbeatProtocol->getNextHop().getAddress();

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
	if (!senderAddress.equals(packetDestination)) {
		//need to forward to next hop
		Serial.print("ForwardData");

		voiceStreamStatManager->updateStreamsIntermediateNode(packetSource, previousHop);

		Tx64Request tx = Tx64Request(myNextHop, response.getData(), response.getDataLength());

		xbee.send(tx);

	} else {
		voiceStreamStatManager->updateVoiceLoss(packetSource, previousHop, dataPtr);
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

	uint8_t dataLoss = dataPtr[13];
	uint8_t totalPacketSent = dataPtr[14];
	uint8_t totalPacketReceived = dataPtr[15];

	if (!senderAddress.equals(packetSource)) {

		XBeeAddress64 nextHop;
		voiceStreamStatManager->getStreamPreviousHop(packetSource, nextHop);

		SerialUSB.print("Forward Path Packet - Stream Sender: ");
		packetSource.printAddressASCII(&SerialUSB);
		SerialUSB.println();

		Tx64Request tx = Tx64Request(nextHop, response.getData(), response.getDataLength());
		xbee.send(tx);

	} else {
		SerialUSB.println("Received Path Packet");

		//Returned to the original sender, update packet loss
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

/*void VoicePacketSender::sendStreamRestart(const XBeeAddress64& dataSenderAddress) {

 uint8_t payload[] = { 'R', 'S', 'T', 'R', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
 (dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
 dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff, (dataSenderAddress.getLsb()
 >> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff, dataSenderAddress.getLsb() & 0xff };
 Tx64Request tx = Tx64Request(myNextHop, payload, sizeof(payload));
 xbee.send(tx);

 }*/

void VoicePacketSender::updateDataRate(const uint8_t dataLoss) {

	SerialUSB.print("DataLoss: ");
	SerialUSB.println(dataLoss);

	VoiceSetting * v = compressionTable.getCompressionTable();
	VoiceSetting newSetting = *(v + dataLoss);

	dupSetting = newSetting.getDupRatio();
	codecSetting = newSetting.getCompressionSetting();

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

