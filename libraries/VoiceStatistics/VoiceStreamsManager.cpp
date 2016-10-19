#include "VoiceStreamsManager.h"

/*
 * VoiceStreamsManager.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

VoiceStreamManager::VoiceStreamManager() {
	xbee = XBee();
	payloadSize = 0;
	timeDifference = 0;
}

VoiceStreamManager::VoiceStreamManager(const uint8_t payloadSize) {
	xbee = XBee();
	this->payloadSize = payloadSize;
	timeDifference = 0;
}

VoiceStreamManager::VoiceStreamManager(XBee& xbee, const uint8_t payloadSize) {
	this->xbee = xbee;
	this->payloadSize = payloadSize;
	timeDifference = 0;
}

void VoiceStreamManager::removeStream(const XBeeAddress64& packetSource) {
//	int i = 0;
//	bool found = false;

	if (streams.find(packetSource) != streams.end()) {
		streams[packetSource].printRouteEnd();
		streams.erase(packetSource);
	}

//	while (i < streams.size()) {
//		if (streams.at(i).getSenderAddress().equals(packetSource)) {
//			streams.at(i).printRouteEnd();
//			found = true;
//			break;
//		}
//		++i;
//	}
//
//	if (found) {
//		SerialUSB.println("Stream now erased");
//		streams.erase(streams.begin() + i);
//	}
}

void VoiceStreamManager::updateVoiceLoss(const XBeeAddress64& packetSource, const XBeeAddress64& previousHop,
		const uint8_t * dataPtr) {

	if (streams.find(packetSource) != streams.end()) {
		streams[packetSource].updateVoiceLoss(dataPtr);

	} else {
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop, 76);
		stream.updateVoiceLoss(dataPtr);
		streams.insert(pair<XBeeAddress64, VoiceStreamStats>(packetSource, stream));
	}
}

void VoiceStreamManager::updateStreamsIntermediateNode(const XBeeAddress64& packetSource,
		const XBeeAddress64& previousHop) {

	if (streams.find(packetSource) != streams.end()) {
		streams[packetSource].setUpStreamNeighborAddress(previousHop);
	} else {
		//SerialUSB.println("New Stream");
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop, 76);
		streams.insert(pair<XBeeAddress64, VoiceStreamStats>(packetSource, stream));
	}
}

void VoiceStreamManager::sendPathPacket() {

	for (std::map<XBeeAddress64, VoiceStreamStats>::iterator it = streams.begin(); it != streams.end();) {

		SerialUSB.println("Sending Path packet");
		const XBeeAddress64 &dataSenderAddress = it->second.getSenderAddress();
		const uint8_t dataLoss = it->second.getPacketLoss();
		const uint8_t totalPacketsSent = it->second.getTotalPacketsSent();
		const uint8_t totalPacketsRecieved = it->second.getTotalPacketsRecieved();

		uint8_t payload[16];

		payload[0] = 'P';
		payload[1] = 'A';
		payload[2] = 'T';
		payload[3] = 'H';
		payload[4] = '\0';
		HeartbeatMessage::addAddressToMessage(payload, dataSenderAddress, 5);
		payload[13] = dataLoss;
		payload[14] = totalPacketsSent;
		payload[15] = totalPacketsRecieved;

		Tx64Request tx = Tx64Request(it->second.getUpStreamNeighborAddress(), ACK_OPTION, payload, sizeof(payload),
				DEFAULT_FRAME_ID);
		xbee.send(tx);

		if (it->second.getNumNoPacketReceived() >= 4) {
			SerialUSB.println("Stream Lost.  Removing stream sent by: ");
			it->second.getSenderAddress().printAddressASCII(&SerialUSB);
			SerialUSB.println();
			streams.erase(it++);
		} else {
			it++;
		}
	}
}

void VoiceStreamManager::getStreamPreviousHop(const XBeeAddress64& packetSource, XBeeAddress64& previousHop) {

	if (streams.find(packetSource) != streams.end()) {
		previousHop = streams[packetSource].getUpStreamNeighborAddress();
	}
}

void VoiceStreamManager::calculateThroughput() {
	for (std::map<XBeeAddress64, VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		if (setTimeDifference) {
			timeDifference = (millis() / 1000.0);
			setTimeDifference = false;
		}
		it->second.calculateThroughput(timeDifference);
	}
}

uint8_t VoiceStreamManager::getPayloadSize() const {
	return payloadSize;
}
;
