#include "VoiceStreamsManager.h"

/*
 * VoiceStreamsManager.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

VoiceStreamStatManager::VoiceStreamStatManager() {
	xbee = XBee();
	payloadSize = 0;
	timeDifference = 0;
}

VoiceStreamStatManager::VoiceStreamStatManager(const uint8_t payloadSize) {
	xbee = XBee();
	this->payloadSize = payloadSize;
	timeDifference = 0;
}

VoiceStreamStatManager::VoiceStreamStatManager(XBee& xbee, const uint8_t payloadSize) {
	this->xbee = xbee;
	this->payloadSize = payloadSize;
	timeDifference = 0;
}

bool VoiceStreamStatManager::removeStream(const XBeeAddress64& packetSource) {
	int i = 0;
	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).printRouteEnd();
			streams.erase(it);
			return true;
		}
		++i;
	}
	return false;
}

void VoiceStreamStatManager::updateVoiceLoss(const XBeeAddress64& packetSource, const XBeeAddress64& previousHop,
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

void VoiceStreamStatManager::updateStreamsIntermediateNode(const XBeeAddress64& packetSource,
		const XBeeAddress64& previousHop) {

	bool found = false;

	for (int i = 0; i < streams.size(); i++) {
		//SerialUSB.println("Old Stream");

		if (streams.at(i).getSenderAddress().equals(packetSource)) {
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

void VoiceStreamStatManager::sendPathPacket() {

	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end();) {

		const XBeeAddress64 &dataSenderAddress = it->getSenderAddress();
		const uint8_t dataLoss = it->getPacketLoss();
		const uint8_t totalPacketsSent = it->getTotalPacketsSent();
		const uint8_t totalPacketsRecieved = it->getTotalPacketsRecieved();

		uint8_t payload[] = { 'P', 'A', 'T', 'H', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
				(dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
				dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff,
				(dataSenderAddress.getLsb() >> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff,
				dataSenderAddress.getLsb() & 0xff, dataLoss, totalPacketsSent, totalPacketsRecieved };
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

void VoiceStreamStatManager::getStreamPreviousHop(const XBeeAddress64& packetSource, XBeeAddress64& previousHop) {

	for (int i = 0; i < streams.size(); i++) {
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			previousHop = streams.at(i).getUpStreamNeighborAddress();
			break;
		}
	}
}

void VoiceStreamStatManager::calculateThroughput() {
	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		if (setTimeDifference) {
			timeDifference = (millis() / 1000.0);
			setTimeDifference = false;
		}
		it->calculateThroughput(timeDifference);
	}
}

const vector<VoiceStreamStats> & VoiceStreamStatManager::getStreams() const {
	return streams;
}

void VoiceStreamStatManager::setStreams(const vector<VoiceStreamStats>& streams) {
	this->streams = streams;
}

uint8_t VoiceStreamStatManager::getPayloadSize() const {
	return payloadSize;
}
