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
	myAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	previousHop = XBeeAddress64();
	frameId = 0;
	myNextHop = XBeeAddress64();
	heartbeatProtocol = 0;
	xbee = XBee();
	voiceStreamManager = 0;
	pathLoss = 0;
	calculateThroughput = 0;
	injectionRate = 0;
	payloadSize = 0;
	compressionTable.buildCompressionLookUpTable();

}

VoicePacketSender::VoicePacketSender(XBee& xbee, HeartbeatProtocol * heartbeatProtocol, Thread * pathLoss,
		Thread * calculateThroughput, VoiceStreamManager * voiceStreamManager, const XBeeAddress64& myAddress,
		const XBeeAddress64& sinkAddress, const uint8_t codecSetting, const float dupSetting,
		const uint8_t payloadSize) {

	this->codecSetting = codecSetting;
	this->dupSetting = dupSetting;
	admcpm = ADPCM();
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->voiceStreamManager = voiceStreamManager;
	this->payloadSize = payloadSize;
	frameId = 0;
	injectionRate = 0;
	myNextHop = XBeeAddress64();
	previousHop = XBeeAddress64();
	//If I don't past the pointer, it just makes a copy of the heartbeat protocol during assignment,
	//If I make changes to the heartbeat protocol outside the class the member variable does not pick up the changes.
	//Thats why I need the pointer.
	this->heartbeatProtocol = heartbeatProtocol;

	this->xbee = xbee;
	this->pathLoss = pathLoss;
	this->calculateThroughput = calculateThroughput;

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

	bool r = (random(100)) < (dupSetting * 100);

	uint8_t destination[100];
	memset(destination, 0, sizeof(destination));

	if (dupSetting != 0 && r && !justSentDup) {
		frameId--;

		destination[0] = 'D';
		destination[1] = 'A';
		destination[2] = 'T';
		destination[3] = 'A';
		destination[4] = '\0';
		HeartbeatMessage::addAddressToMessage(destination, myAddress, 5);
		HeartbeatMessage::addAddressToMessage(destination, sinkAddress, 13);
		destination[21] = frameId;
		destination[22] = codecSetting;

		Tx64Request tx = Tx64Request(myNextHop, destination, sizeof(destination));

		xbee.send(tx);
		frameId++;
		justSentDup = true;
	} else {

		destination[0] = 'D';
		destination[1] = 'A';
		destination[2] = 'T';
		destination[3] = 'A';
		destination[4] = '\0';
		HeartbeatMessage::addAddressToMessage(destination, myAddress, 5);
		HeartbeatMessage::addAddressToMessage(destination, sinkAddress, 13);
		destination[21] = frameId;
		destination[22] = codecSetting;

		Tx64Request tx = Tx64Request(myNextHop, destination, sizeof(destination));

		xbee.send(tx);
		frameId++;
		justSentDup = false;
	}

}

void VoicePacketSender::handleDataPacket(const Rx64Response &response) {

	//Extract the packet's final destination

	//check to see if the packet final destination is this node's address
	//If not setup another request to forward it.

	uint8_t * dataPtr = response.getData();

	previousHop = response.getRemoteAddress64();

	HeartbeatMessage::setAddress(dataPtr, packetDestination, 13);
	HeartbeatMessage::setAddress(dataPtr, packetSource, 5);

	if (!myAddress.equals(packetDestination)) {

		myNextHop = heartbeatProtocol->getNextHop().getAddress();

		//need to forward to next hop
		Tx64Request tx = Tx64Request(myNextHop, response.getData(), response.getDataLength());
		xbee.send(tx);

		voiceStreamManager->updateStreamsIntermediateNode(packetSource, previousHop);

	} else {

		uint8_t * dataPtr = response.getData();

		voiceStreamManager->updateVoiceLoss(packetSource, previousHop, dataPtr);
		(*pathLoss).enabled = true;
		(*calculateThroughput).enabled = true;
	}

}

void VoicePacketSender::handlePathPacket(const Rx64Response &response) {

	XBeeAddress64 packetSource;

	uint8_t * dataPtr = response.getData();

	HeartbeatMessage::setAddress(dataPtr, packetSource, 5);

	uint8_t dataLoss = dataPtr[13];

	if (!myAddress.equals(packetSource)) {

		XBeeAddress64 nextHop;
		voiceStreamManager->getStreamPreviousHop(packetSource, nextHop);

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

void VoicePacketSender::updateDataRate(uint8_t dataLoss) {

	SerialUSB.print("DataLoss: ");
	SerialUSB.println(dataLoss);

	if (dataLoss >= 25) {
		dataLoss = 24;
	}

	VoiceSetting * v = compressionTable.getCompressionTable();
	VoiceSetting newSetting = v[dataLoss];

	dupSetting = newSetting.getDupRatio();
	codecSetting = newSetting.getCompressionSetting();

	SerialUSB.print("DupSetting: ");
	SerialUSB.println(dupSetting);

	SerialUSB.print("Codec Setting: ");
	SerialUSB.println(codecSetting);

}

void VoicePacketSender::resetFrameID() {
	frameId = 0;
}

void VoicePacketSender::sendTracePacket() {

	TraceMessage traceMessage;

	uint8_t traceMessagePayLoad[] = { 'T', 'R', 'C', 'E', '\0', 0 };
	Tx64Request tx = Tx64Request(myNextHop, traceMessagePayLoad, sizeof(traceMessagePayLoad));
	// send the command
	xbee.send(tx);

}

void VoicePacketSender::handleTracePacket(const Rx64Response &response) {

	myNextHop = heartbeatProtocol->getNextHop().getAddress();

	TraceMessage traceMessage;
	traceMessage.transcribeMessage(response);

	if (!myAddress.equals(sinkAddress)) {

		uint8_t traceMessagePayLoad[62];
		memset(traceMessagePayLoad, 0, sizeof(traceMessagePayLoad));

		traceMessagePayLoad[0] = 'T';
		traceMessagePayLoad[1] = 'R';
		traceMessagePayLoad[2] = 'C';
		traceMessagePayLoad[3] = 'E';
		traceMessagePayLoad[4] = '\0';
		traceMessagePayLoad[5] = traceMessage.getAddressListLength();

		uint8_t lastAddress = traceMessage.getAddressListLength() * 8 + 6;
		uint8_t i = 6;

		for (int j = 0; j < traceMessage.getAddressListLength(); ++j) {

			HeartbeatMessage::addAddressToMessage(traceMessagePayLoad, traceMessage.getAddresses()[j], i);

			i += 8;
		}

		Tx64Request tx = Tx64Request(myNextHop, traceMessagePayLoad, sizeof(traceMessagePayLoad));
		xbee.send(tx);

	} else {
		//Reach the sink
		traceMessage.addAddress(myAddress);
		traceMessage.printTraceMessage();
	}

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

