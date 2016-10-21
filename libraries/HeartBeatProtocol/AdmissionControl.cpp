/*
 * AdmissionControl.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include "AdmissionControl.h"

AdmissionControl::AdmissionControl() {
	heartbeatProtocol = 0;
	voiceStreamManager = 0;
	voicePacketSender = 0;
	grantTimeoutLength = 0;
	rejcTimeoutLength = 0;

}

AdmissionControl::AdmissionControl(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const XBee& xbee,
		HeartbeatProtocol * heartbeatProtocol, VoiceStreamManager * voiceStreamManager,
		VoicePacketSender * voicePacketSender, const unsigned long grantTimeoutLength,
		const unsigned long rejcTimeoutLength) {
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->heartbeatProtocol = heartbeatProtocol;
	this->voiceStreamManager = voiceStreamManager;
	this->voicePacketSender = voicePacketSender;
	this->xbee = xbee;
	this->grantTimeoutLength = grantTimeoutLength;
	this->rejcTimeoutLength = rejcTimeoutLength;

}

void AdmissionControl::checkTimers() {
	if (!potentialStreams.empty()) {
		for (std::map<XBeeAddress64, PotentialStream>::iterator it = potentialStreams.begin();
				it != potentialStreams.end();) {
			XBeeAddress64 sourceAddress = it->first;

			if (it->second.getGrantTimer().timeoutTimer() && myAddress.equals(sinkAddress)) {
				//Only sink should send grant message when timer expires
				XBeeAddress64 nextHop = it->second.getUpStreamNeighbor();
				sendGRANTPacket(sourceAddress, nextHop);
				potentialStreams.erase(it++);
			} else if (it->second.getRejcTimer().timeoutTimer()) {
				//Wait for all init messages then send rejc if violate local capacity
				//SerialUSB.println("REJC Timer Expired. Check Local Capacity...");
				bool rejected = checkLocalCapacity(it->second);
				if (rejected) {
					SerialUSB.println("Sending Reject Packet...");
					sendREDJPacket(it->first);
				}

				//OnPath will be receiving a GRNT packet.  Potential Stream will be removed when forwarding
				//GRNT packet.
				if (!it->second.isOnPath()) {
					potentialStreams.erase(it++);
				} else {
					++it;
				}
			} else {
				++it;
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
		const uint8_t * injectionRateP = reinterpret_cast<uint8_t*>(&injectionRate);

		SerialUSB.print("Injection Rate: ");
		SerialUSB.println(injectionRate);
		uint8_t payloadBroadCast[25];

		payloadBroadCast[0] = 'I';
		payloadBroadCast[1] = 'N';
		payloadBroadCast[2] = 'I';
		payloadBroadCast[3] = 'T';
		payloadBroadCast[4] = '\0';

		HeartbeatMessage::addAddressToMessage(payloadBroadCast, myAddress, 5);
		HeartbeatMessage::addAddressToMessage(payloadBroadCast, myNextHop, 13);
		HeartbeatMessage::addFloat(payloadBroadCast, injectionRateP, 21);

		Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));
		// send the command
		xbee.send(tx);
	}
}

void AdmissionControl::sendREDJPacket(const XBeeAddress64 &senderAddress) {

	XBeeAddress64 nextHop = heartbeatProtocol->getNextHop().getAddress();
	if (!nextHop.equals(XBeeAddress64())) {

		uint8_t payload[13];

		payload[0] = 'R';
		payload[1] = 'E';
		payload[2] = 'D';
		payload[3] = 'J';
		payload[4] = '\0';
		HeartbeatMessage::addAddressToMessage(payload, senderAddress, 5);

		Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), DEFAULT_FRAME_ID);
		xbee.send(tx);
	} else {
		Serial.print("No nextHop for reject message");
	}

}

void AdmissionControl::sendGRANTPacket(const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop) {

	SerialUSB.println("Sending GRNT");

	uint8_t payload[13];

	payload[0] = 'G';
	payload[1] = 'R';
	payload[2] = 'N';
	payload[3] = 'T';
	payload[4] = '\0';
	HeartbeatMessage::addAddressToMessage(payload, senderAddress, 5);

	Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), DEFAULT_FRAME_ID);
	xbee.send(tx);

}

void AdmissionControl::handleInitPacket(const Rx64Response &response) {

	XBeeAddress64 receivedAddress = response.getRemoteAddress64();

	XBeeAddress64 senderAddress;
	XBeeAddress64 nextHop;
	uint8_t* dataPtr = response.getData();

	HeartbeatMessage::setAddress(dataPtr, senderAddress, 5);
	HeartbeatMessage::setAddress(dataPtr, nextHop, 13);

	float dataRate;
	memcpy(&dataRate, dataPtr + 21, sizeof(float));

	//remove any old streams
	voiceStreamManager->removeStream(senderAddress);

	if (nextHop.equals(sinkAddress) && myAddress.equals(sinkAddress)) {
		//sink node
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
		SerialUSB.print("Receiving on path request for new stream via: ");
		receivedAddress.printAddressASCII(&SerialUSB);
		SerialUSB.print("  Sender Address: ");
		senderAddress.printAddressASCII(&SerialUSB);
		SerialUSB.print("  Potential Data Rate: ");
		SerialUSB.println(dataRate);
		XBeeAddress64 heartbeatAddress = heartbeatProtocol->getBroadcastAddress();
		XBeeAddress64 myNextHop = heartbeatProtocol->getNextHop().getAddress();

		const uint8_t * injectionRateP = reinterpret_cast<uint8_t*>(&dataRate);

		uint8_t payloadBroadCast[25];
		payloadBroadCast[0] = 'I';
		payloadBroadCast[1] = 'N';
		payloadBroadCast[2] = 'I';
		payloadBroadCast[3] = 'T';
		payloadBroadCast[4] = '\0';

		HeartbeatMessage::addAddressToMessage(payloadBroadCast, senderAddress, 5);
		HeartbeatMessage::addAddressToMessage(payloadBroadCast, myNextHop, 13);
		HeartbeatMessage::addFloat(payloadBroadCast, injectionRateP, 21);

		Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));
		xbee.send(tx);
		PotentialStream potentialStream = PotentialStream(senderAddress, receivedAddress, grantTimeoutLength,
				rejcTimeoutLength, dataRate);

		potentialStream.getRejcTimer().startTimer();
		potentialStream.setOnPath(true);
		addPotentialStream(potentialStream, dataRate);

	} else if (!myAddress.equals(sinkAddress)) {
		//node affected but not node on path and not sink node.
		SerialUSB.print("Receiving not on path request for new stream via: ");
		receivedAddress.printAddressASCII(&SerialUSB);
		SerialUSB.print("  Sender Address: ");
		senderAddress.printAddressASCII(&SerialUSB);
		SerialUSB.print("  Potential Data Rate: ");
		SerialUSB.println(dataRate);
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

	HeartbeatMessage::setAddress(dataPtr, senderAddress, 5);

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

	HeartbeatMessage::setAddress(dataPtr, sourceAddress, 5);

	if (!myAddress.equals(sourceAddress)) {

		//SerialUSB.println("Old Stream");
		if (potentialStreams.find(sourceAddress) != potentialStreams.end()) {
			sendGRANTPacket(sourceAddress, potentialStreams[sourceAddress].getUpStreamNeighbor());
			removePotentialStream(sourceAddress);

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

	if (potentialStreams.find(packetSource) != potentialStreams.end()) {
		/*SerialUSB.print("Removed Potential Stream: ");
		 potentialStreams.at(i).getSourceAddress().printAddressASCII(&SerialUSB);
		 SerialUSB.println();*/
		potentialStreams.erase(packetSource);
		return true;
	}
	return false;
}

void AdmissionControl::addPotentialStream(PotentialStream& potentialStream, float const addDataRate) {

	const XBeeAddress64 sourceAddress = potentialStream.getSourceAddress();

	if (potentialStreams.find(potentialStream.getSourceAddress()) != potentialStreams.end()) {

		potentialStreams[sourceAddress].increaseDataRate(addDataRate);
		if (potentialStream.isOnPath()) {
			potentialStream.increaseDataRate(potentialStreams[sourceAddress].getIncreasedDataRate());
			potentialStreams[sourceAddress] = potentialStream;
		}
	} else {
		potentialStreams.insert(pair<XBeeAddress64, PotentialStream>(sourceAddress, potentialStream));
	}
}

void AdmissionControl::printPotentialStreams() {
	SerialUSB.println("Potential Streams: ");
	for (std::map<XBeeAddress64, PotentialStream>::iterator it = potentialStreams.begin(); it != potentialStreams.end();
			++it) {
		SerialUSB.print("    ");
		it->second.printPotentialStream();
	}
}

bool AdmissionControl::checkLocalCapacity(const PotentialStream& potentialStream) {
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
