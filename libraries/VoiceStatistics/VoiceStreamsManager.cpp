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

}

VoiceStreamStatManager::VoiceStreamStatManager(const uint8_t payloadSize) {
	xbee = XBee();
	this->payloadSize = payloadSize;

}

VoiceStreamStatManager::VoiceStreamStatManager(XBee& xbee, const uint8_t payloadSize) {
	this->xbee = xbee;
	this->payloadSize = payloadSize;
}

void VoiceStreamStatManager::calcuateThroughput(const XBeeAddress64& packetSource) {
	bool found = false;

	for (int i = 0; i < streams.size(); i++) {
		//SerialUSB.println("Old Stream");
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).calculateThroughput();
			found = true;
			break;
		}
	}

	if (!found) {
		//SerialUSB.println("New Stream");
		VoiceStreamStats stream = VoiceStreamStats(packetSource, 76);
		stream.startStream();
		stream.calculateThroughput();
		streams.push_back(stream);
	}

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
		//SerialUSB.println("Old Stream");

		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).updateVoiceLoss(dataPtr);

			found = true;
			break;
		}
	}

	if (!found) {
		//SerialUSB.println("New Stream");
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop, 76);
		stream.startStream();
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
	int i = 0;

	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {

		const XBeeAddress64 &dataSenderAddress = streams.at(i).getSenderAddress();
		const uint8_t dataLoss = streams.at(i).getPacketLoss();
		const uint8_t totalPacketsSent = streams.at(i).getTotalPacketsSent();
		const uint8_t totalPacketsRecieved = streams.at(i).getTotalPacketsRecieved();

		uint8_t payload[] = { 'P', 'A', 'T', 'H', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
				(dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
				dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff,
				(dataSenderAddress.getLsb() >> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff,
				dataSenderAddress.getLsb() & 0xff, dataLoss, totalPacketsSent, totalPacketsRecieved };
		Tx64Request tx = Tx64Request(streams.at(i).getUpStreamNeighborAddress(), payload, sizeof(payload));
		xbee.send(tx);

		i++;

		streams.at(i).calculateThroughput();

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

void VoiceStreamStatManager::handleStreamRestart(const Rx64Response& response) {

	uint8_t * dataPtr = response.getData();

	XBeeAddress64 packetSource;
	packetSource.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	packetSource.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	removeStream(packetSource);
}

const vector<VoiceStreamStats> & VoiceStreamStatManager::getStreams() const {
	return streams;
}

void VoiceStreamStatManager::setStreams(const vector<VoiceStreamStats>& streams) {
	this->streams = streams;
}

