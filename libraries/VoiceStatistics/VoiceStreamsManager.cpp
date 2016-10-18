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
	int i = 0;
	bool found = false;
	while (i < streams.size()) {
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).printRouteEnd();
			found = true;
			break;
		}
		++i;
	}

	if (found) {
		SerialUSB.println("Stream now erased");
		streams.erase(streams.begin() + i);
	}
}

void VoiceStreamManager::updateVoiceLoss(const XBeeAddress64& packetSource, const XBeeAddress64& previousHop,
		const uint8_t * dataPtr) {

	bool found = false;

	for (int i = 0; i < streams.size(); i++) {
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).updateVoiceLoss(dataPtr);
			found = true;
			break;
		}
	}

	if (!found) {
		/*SerialUSB.println("New Stream");*/
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop, 76);
		stream.updateVoiceLoss(dataPtr);
		streams.push_back(stream);
	}

}

void VoiceStreamManager::updateStreamsIntermediateNode(const XBeeAddress64& packetSource,
		const XBeeAddress64& previousHop) {

	bool found = false;

	for (int i = 0; i < streams.size(); i++) {
		//SerialUSB.println("Old Stream");

		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).setUpStreamNeighborAddress(previousHop);
			found = true;
			break;
		}
	}

	if (!found) {
		//SerialUSB.println("New Stream");
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop, 76);
		streams.push_back(stream);
	}

}

void VoiceStreamManager::sendPathPacket() {

	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end();) {

		SerialUSB.println("Sending Path packet");
		const XBeeAddress64 &dataSenderAddress = it->getSenderAddress();
		const uint8_t dataLoss = it->getPacketLoss();
		const uint8_t totalPacketsSent = it->getTotalPacketsSent();
		const uint8_t totalPacketsRecieved = it->getTotalPacketsRecieved();

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

		Tx64Request tx = Tx64Request(it->getUpStreamNeighborAddress(), ACK_OPTION, payload, sizeof(payload),
				DEFAULT_FRAME_ID);
		xbee.send(tx);

		if (it->getNumNoPacketReceived() >= 4) {
			SerialUSB.println("Stream Lost.  Removing stream sent by: ");
			it->getSenderAddress().printAddressASCII(&SerialUSB);
			SerialUSB.println();
			it = streams.erase(it);
		} else {
			it++;
		}

	}
}

void VoiceStreamManager::getStreamPreviousHop(const XBeeAddress64& packetSource, XBeeAddress64& previousHop) {

	for (int i = 0; i < streams.size(); i++) {
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			previousHop = streams.at(i).getUpStreamNeighborAddress();
			break;
		}
	}
}

void VoiceStreamManager::calculateThroughput() {
	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		if (setTimeDifference) {
			timeDifference = (millis() / 1000.0);
			setTimeDifference = false;
		}
		it->calculateThroughput(timeDifference);
	}
}

uint8_t VoiceStreamManager::getPayloadSize() const {
	return payloadSize;
}
;
