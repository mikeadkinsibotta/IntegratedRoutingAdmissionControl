/*
 * AdmissionControl.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include "AdmissionControl.h"

const float MAX_FLT = 9999.0;
const uint8_t HEARTBEAT_PAYLOAD_SIZE = 17;
AdmissionControl::AdmissionControl() {
	aodv = 0;
	voiceStreamManager = 0;
	voicePacketSender = 0;
	grantTimeoutLength = 0;
	rejcTimeoutLength = 0;
	localCapacity = MAX_FLT;
	buildSaturationTable();

}

AdmissionControl::AdmissionControl(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const XBee& xbee,
		AODV * aodv, VoiceStreamManager * voiceStreamManager, VoicePacketSender * voicePacketSender,
		const unsigned long grantTimeoutLength, const unsigned long rejcTimeoutLength) {
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->aodv = aodv;
	this->voiceStreamManager = voiceStreamManager;
	this->voicePacketSender = voicePacketSender;
	this->xbee = xbee;
	this->grantTimeoutLength = grantTimeoutLength;
	this->rejcTimeoutLength = rejcTimeoutLength;
	localCapacity = MAX_FLT;
	buildSaturationTable();

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

void AdmissionControl::sendREDJPacket(const XBeeAddress64 &senderAddress) {
	XBeeAddress64 nextHop = XBeeAddress64(); //nextHopheartbeatProtocol->getNextHop().getAddress();
	if (!nextHop.equals(XBeeAddress64())) {

		uint8_t payload[13];
		//REDJ backwards so it doesn't conflict with RREQ and RREP
		payload[0] = 'J';
		payload[1] = 'D';
		payload[2] = 'E';
		payload[3] = 'R';
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
		XBeeAddress64 heartbeatAddress = aodv->getBroadcastAddress();
		XBeeAddress64 myNextHop = aodv->getNextHop(sinkAddress);

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
	SerialUSB.print("Receive Reject from: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.println();

	if (myAddress.equals(sinkAddress)) {
		removePotentialStream(senderAddress);
	} else {
		SerialUSB.println("Sending Reject Packet...");
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

	float potentialDataRate = potentialStream.getIncreasedDataRate();
	XBeeAddress64 sourceAddress = potentialStream.getSourceAddress();

	SerialUSB.print("Potential Stream: ");
	sourceAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(" Potential Data Rate: ");
	SerialUSB.print(potentialDataRate);

	SerialUSB.print(" Local Capacity: ");
	SerialUSB.print(localCapacity);
	SerialUSB.println();

	if (potentialDataRate > localCapacity) {
		SerialUSB.println("Stream Rejected");
		return true;
	}
	SerialUSB.println("Stream Accepted");

	return false;
}

void AdmissionControl::broadcastHeartBeat(const float myDataRate, const XBeeAddress64& broadcastAddress,
		const XBeeAddress64& downStreamNeighbor) {

	HeartbeatMessage message = HeartbeatMessage(myDataRate, downStreamNeighbor);
	uint8_t payload[HEARTBEAT_PAYLOAD_SIZE];
	message.generateBeatMessage(payload);

	Tx64Request tx = Tx64Request(broadcastAddress, payload, sizeof(payload));
	xbee.send(tx);

}

void AdmissionControl::receiveHeartBeat(const Rx64Response& response) {

	HeartbeatMessage message;

	message.transcribeHeartbeatPacket(response);
	updateNeighborHoodTable(message);
	getLocalCapacity(message.getDataRate());

}

void AdmissionControl::updateNeighbor(Neighbor& neighbor, const HeartbeatMessage& heartbeatMessage) {
	neighbor.setDataRate(heartbeatMessage.getDataRate());
	neighbor.setDownStreamNeighbor(heartbeatMessage.getDownStreamNeighbor());
}

void AdmissionControl::updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage) {

	bool found = false;

	for (int i = 0; i < neighborhoodTable.size(); i++) {
		if (neighborhoodTable.at(i).getAddress().equals(heartbeatMessage.getSenderAddress())) {
			updateNeighbor(neighborhoodTable.at(i), heartbeatMessage);
			found = true;
			break;
		}
	}

	if (!found) {
		//Sets timestamp when constructor is called.
		Neighbor neighbor = Neighbor(heartbeatMessage.getDataRate(), heartbeatMessage.getSenderAddress(),
				heartbeatMessage.getDownStreamNeighbor());
		neighborhoodTable.push_back(neighbor);
	}
}

void AdmissionControl::getLocalCapacity(float myDataRate) {

	localCapacity = MAX_FLT;
	float neighborhoodRate = 0;
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		neighborhoodRate += neighborhoodTable.at(i).getDataRate();
		if (neighborhoodTable.at(i).getDownStreamNeighbor().equals(myAddress)) {
			//Account for forwarding any data
			myDataRate += neighborhoodTable.at(i).getDataRate();
		}

	}
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
	neighborhoodRate += myDataRate;
	if (neighborHoodSize > 1) {
		Saturation i = satT[neighborHoodSize - 1];
		localCapacity = i.getRate() - neighborhoodRate;
	}

}

void AdmissionControl::buildSaturationTable() {
	satT[0] = Saturation(2, 120.90);
	satT[1] = Saturation(3, 153.39);
	satT[2] = Saturation(4, 151.2);
	satT[3] = Saturation(5, 154.45);
	satT[4] = Saturation(6, 111.42);

}
