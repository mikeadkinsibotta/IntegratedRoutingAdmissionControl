/*
 * AdmissionControl.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include "AdmissionControl.h"

AdmissionControl::AdmissionControl() {
	heartbeatProtocol = 0;
	voiceStreamStatManager = 0;
	voicePacketSender = 0;
	grantTimeoutLength = 0;
	rejcTimeoutLength = 0;

}

AdmissionControl::AdmissionControl(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const XBee& xbee,
		HeartbeatProtocol * heartbeatProtocol, VoiceStreamStatManager * voiceStreamStatManager,
		VoicePacketSender * voicePacketSender, const unsigned long grantTimeoutLength,
		const unsigned long rejcTimeoutLength) {
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->heartbeatProtocol = heartbeatProtocol;
	this->voiceStreamStatManager = voiceStreamStatManager;
	this->voicePacketSender = voicePacketSender;
	this->xbee = xbee;
	this->grantTimeoutLength = grantTimeoutLength;
	this->rejcTimeoutLength = rejcTimeoutLength;

}

void AdmissionControl::checkTimers() {

	for (int i = 0; i < potentialStreams.size(); i++) {
		XBeeAddress64 sourceAddress = potentialStreams.at(i).getSourceAddress();
		if (potentialStreams.at(i).getGrantTimer().timeoutTimer() && myAddress.equals(sinkAddress)) {
			//Only sink should send grant message when timer expires
			XBeeAddress64 nextHop = potentialStreams.at(i).getUpStreamNeighbor();
			sendGRANTPacket(sourceAddress, nextHop);
			removePotentialStream(sourceAddress);
		} else if (potentialStreams.at(i).getRejcTimer().timeoutTimer()) {
			//Wait for all init messages then send rejc if violate local capacity
			//SerialUSB.println("REJC Timer Expired. Check Local Capacity...");
			bool rejected = checkLocalCapacity(potentialStreams.at(i));
			if (rejected) {
				SerialUSB.println("Sending Reject Packet...");
				sendREDJPacket(potentialStreams.at(i).getSourceAddress());
			}

			//OnPath will be receiving a GRNT packet.  Potential Stream will be removed when forwarding
			//GRNT packet.
			if (!potentialStreams.at(i).isOnPath()) {
				removePotentialStream(sourceAddress);
			}
		}
	}

}

void AdmissionControl::sendInitPacket(const uint8_t codecSetting, const float dupSetting) {

	bool hasNextHop = heartbeatProtocol->isRouteFlag();
	if (hasNextHop) {
		SerialUSB.println("Sending Init");
		XBeeAddress64 heartbeatAddress = heartbeatProtocol->getBroadcastAddress();
		XBeeAddress64 myNextHop = heartbeatProtocol->getNextHop().getAddress();
		float injectionRate = 64.00 * (codecSetting / 16.00) * (1.00 + dupSetting);
		uint8_t * injectionRateP = (uint8_t *) &injectionRate;

		SerialUSB.print("Injection Rate: ");
		SerialUSB.println(injectionRate);
		uint8_t payloadBroadCast[25];

		payloadBroadCast[0] = 'I';
		payloadBroadCast[1] = 'N';
		payloadBroadCast[2] = 'I';
		payloadBroadCast[3] = 'T';
		payloadBroadCast[4] = '\0';
		payloadBroadCast[5] = (myAddress.getMsb() >> 24) & 0xff;
		payloadBroadCast[6] = (myAddress.getMsb() >> 16) & 0xff;
		payloadBroadCast[7] = (myAddress.getMsb() >> 8) & 0xff;
		payloadBroadCast[8] = myAddress.getMsb() & 0xff;
		payloadBroadCast[9] = (myAddress.getLsb() >> 24) & 0xff;
		payloadBroadCast[10] = (myAddress.getLsb() >> 16) & 0xff;
		payloadBroadCast[11] = (myAddress.getLsb() >> 8) & 0xff;
		payloadBroadCast[12] = myAddress.getLsb() & 0xff;
		payloadBroadCast[13] = (myNextHop.getMsb() >> 24) & 0xff;
		payloadBroadCast[14] = (myNextHop.getMsb() >> 16) & 0xff;
		payloadBroadCast[15] = (myNextHop.getMsb() >> 8) & 0xff;
		payloadBroadCast[16] = myNextHop.getMsb() & 0xff;
		payloadBroadCast[17] = (myNextHop.getLsb() >> 24) & 0xff;
		payloadBroadCast[18] = (myNextHop.getLsb() >> 16) & 0xff;
		payloadBroadCast[19] = (myNextHop.getLsb() >> 8) & 0xff;
		payloadBroadCast[20] = myNextHop.getLsb() & 0xff;
		payloadBroadCast[21] = injectionRateP[0];
		payloadBroadCast[22] = injectionRateP[1];
		payloadBroadCast[23] = injectionRateP[2];
		payloadBroadCast[24] = injectionRateP[3];

		Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));
		// send the command
		xbee.send(tx);
	}
}

void AdmissionControl::sendREDJPacket(const XBeeAddress64 &senderAddress) {

	XBeeAddress64 nextHop = heartbeatProtocol->getNextHop().getAddress();
	if (!nextHop.equals(XBeeAddress64())) {

		uint8_t payload[] = { 'R', 'E', 'D', 'J', '\0', (senderAddress.getMsb() >> 24) & 0xff, (senderAddress.getMsb()
				>> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
				(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb()
						>> 8) & 0xff, senderAddress.getLsb() & 0xff };
		Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), DEFAULT_FRAME_ID);
		xbee.send(tx);
	} else {
		Serial.print("No nextHop for reject message");
	}

}

void AdmissionControl::sendGRANTPacket(const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop) {

	SerialUSB.println("Sending GRNT");
	uint8_t payload[] = { 'G', 'R', 'N', 'T', '\0', (senderAddress.getMsb() >> 24) & 0xff,
			(senderAddress.getMsb() >> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
			(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb() >> 8)
					& 0xff, senderAddress.getLsb() & 0xff };
	Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), DEFAULT_FRAME_ID);
	xbee.send(tx);

}

void AdmissionControl::handleInitPacket(const Rx64Response &response) {

	XBeeAddress64 receivedAddress = response.getRemoteAddress64();

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

	float * dataRateP = (float*) (dataPtr + 21);
	float dataRate = *dataRateP;

	//remove any old streams
	voiceStreamStatManager->removeStream(senderAddress);

	if (nextHop.equals(sinkAddress) && myAddress.equals(sinkAddress)) {
		//sink node
		SerialUSB.println("Recieved INT");
		SerialUSB.print("Receiving request for new stream via: ");
		receivedAddress.printAddressASCII(&SerialUSB);
		SerialUSB.print("  Sender Address: ");
		senderAddress.printAddressASCII(&SerialUSB);
		SerialUSB.print("  Potential Data Rate: ");
		SerialUSB.println(dataRate);

		PotentialStream potentialStream = PotentialStream(senderAddress, receivedAddress, grantTimeoutLength,
				rejcTimeoutLength, dataRate);
		//Start Grant Timer
		potentialStream.getGrantTimer().startTimer();
		potentialStream.setOnPath(true);
		addPotentialStream(potentialStream, dataRate);

	} else if (nextHop.equals(myAddress)) {
		//on path node

		XBeeAddress64 heartbeatAddress = heartbeatProtocol->getBroadcastAddress();
		XBeeAddress64 myNextHop = heartbeatProtocol->getNextHop().getAddress();

		uint8_t * injectionRateP = (uint8_t *) &dataRate;

		uint8_t payloadBroadCast[25];
		payloadBroadCast[0] = 'I';
		payloadBroadCast[1] = 'N';
		payloadBroadCast[2] = 'I';
		payloadBroadCast[3] = 'T';
		payloadBroadCast[4] = '\0';
		payloadBroadCast[5] = (senderAddress.getMsb() >> 24) & 0xff;
		payloadBroadCast[6] = (senderAddress.getMsb() >> 16) & 0xff;
		payloadBroadCast[7] = (senderAddress.getMsb() >> 8) & 0xff;
		payloadBroadCast[8] = senderAddress.getMsb() & 0xff;
		payloadBroadCast[9] = (senderAddress.getLsb() >> 24) & 0xff;
		payloadBroadCast[10] = (senderAddress.getLsb() >> 16) & 0xff;
		payloadBroadCast[11] = (senderAddress.getLsb() >> 8) & 0xff;
		payloadBroadCast[12] = senderAddress.getLsb() & 0xff;
		payloadBroadCast[13] = (myNextHop.getMsb() >> 24) & 0xff;
		payloadBroadCast[14] = (myNextHop.getMsb() >> 16) & 0xff;
		payloadBroadCast[15] = (myNextHop.getMsb() >> 8) & 0xff;
		payloadBroadCast[16] = myNextHop.getMsb() & 0xff;
		payloadBroadCast[17] = (myNextHop.getLsb() >> 24) & 0xff;
		payloadBroadCast[18] = (myNextHop.getLsb() >> 16) & 0xff;
		payloadBroadCast[19] = (myNextHop.getLsb() >> 8) & 0xff;
		payloadBroadCast[20] = myNextHop.getLsb() & 0xff;
		payloadBroadCast[21] = injectionRateP[0];
		payloadBroadCast[22] = injectionRateP[1];
		payloadBroadCast[23] = injectionRateP[2];
		payloadBroadCast[24] = injectionRateP[3];

		Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));
		xbee.send(tx);
		PotentialStream potentialStream = PotentialStream(senderAddress, receivedAddress, grantTimeoutLength,
				rejcTimeoutLength, dataRate);

		potentialStream.getRejcTimer().startTimer();
		potentialStream.setOnPath(true);
		addPotentialStream(potentialStream, dataRate);

	} else if (!myAddress.equals(sinkAddress)) {
		//node affected but not node on path and not sink node.
		PotentialStream potentialStream = PotentialStream(senderAddress, receivedAddress, grantTimeoutLength,
				rejcTimeoutLength, dataRate);
		potentialStream.getRejcTimer().startTimer();
		addPotentialStream(potentialStream, dataRate);
	}

}

void AdmissionControl::handleREDJPacket(Rx64Response &response) {
	SerialUSB.println("ReceiveREDJ");
	XBeeAddress64 senderAddress;
	uint8_t * dataPtr = response.getData();

	senderAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	senderAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (myAddress.equals(sinkAddress)) {
		removePotentialStream(senderAddress);
	} else {
		sendREDJPacket(senderAddress);
	}

}

void AdmissionControl::handleGRANTPacket(const Rx64Response &response, bool& initThreadActive,
		bool& voiceThreadActive) {
	XBeeAddress64 sourceAddress;
	uint8_t * dataPtr = response.getData();
	XBeeAddress64 previousHop = response.getRemoteAddress64();

	sourceAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	sourceAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (!myAddress.equals(sourceAddress)) {
		for (int i = 0; i < potentialStreams.size(); i++) {
			//SerialUSB.println("Old Stream");
			if (potentialStreams.at(i).getSourceAddress().equals(sourceAddress)) {
				sendGRANTPacket(sourceAddress, potentialStreams.at(i).getUpStreamNeighbor());
				removePotentialStream(sourceAddress);
				break;
			}
		}

	} else {
		SerialUSB.println("Received GRNT.  Stop sending init messages! Starting sending this mother!");
		initThreadActive = false;
		voicePacketSender->resetFrameID();
		voiceThreadActive = true;
	}
}

void AdmissionControl::intializationSenderTimeout() {
	SerialUSB.print("InitializationTimeout");

	//TODO need to resend INIT message;
}

bool AdmissionControl::removePotentialStream(const XBeeAddress64& packetSource) {

	for (vector<PotentialStream>::iterator it = potentialStreams.begin(); it != potentialStreams.end();) {
		if (it->getSourceAddress().equals(packetSource)) {
			/*SerialUSB.print("Removed Potential Stream: ");
			 potentialStreams.at(i).getSourceAddress().printAddressASCII(&SerialUSB);
			 SerialUSB.println();*/
			it = potentialStreams.erase(it);
			return true;
		}
		++it;
	}
	return false;
}

void AdmissionControl::addPotentialStream(PotentialStream& potentialStream, float const addDataRate) {

	bool found = false;

	for (int i = 0; i < potentialStreams.size(); i++) {
		if (potentialStreams.at(i).getSourceAddress().equals(potentialStream.getSourceAddress())) {
			potentialStreams.at(i).increaseDataRate(addDataRate);
			if (potentialStream.isOnPath()) {
				potentialStream.increaseDataRate(potentialStreams.at(i).getIncreasedDataRate());
				potentialStreams.at(i) = potentialStream;
			}

			found = true;
			break;
		}
	}

	if (!found) {
		potentialStreams.push_back(potentialStream);
	}
}

void AdmissionControl::printPotentialStreams() const {
	SerialUSB.println("Potential Streams: ");
	for (int i = 0; i < potentialStreams.size(); i++) {
		/*	SerialUSB.print("    ");
		 potentialStreams.at(i).printPotentialStream();*/
	}
}

bool AdmissionControl::checkLocalCapacity(const PotentialStream& potentialStream) const {
	float neighborhoodCapacity = heartbeatProtocol->getLocalCapacity();
	float potentialDataRate = potentialStream.getIncreasedDataRate();
	XBeeAddress64 sourceAddress = potentialStream.getSourceAddress();

	SerialUSB.print("Potential Stream: ");
	sourceAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(" Potential Data Rate: ");
	SerialUSB.print(potentialDataRate);

	SerialUSB.print(" Local Capacity: ");
	SerialUSB.print(neighborhoodCapacity);
	SerialUSB.println();

	if (potentialDataRate > neighborhoodCapacity) {
		/*SerialUSB.println("Stream Rejected");
		 SerialUSB.println();*/
		return true;
	}
	/*SerialUSB.println("Stream Accepted");
	 SerialUSB.println();*/
	return false;
}
