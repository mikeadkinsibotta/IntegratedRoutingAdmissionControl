#include "VoiceStreamsManager.h"

/*
 * VoiceStreamsManager.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

VoiceStreamStatManager::VoiceStreamStatManager() {
	xbee = XBee();
}

VoiceStreamStatManager::VoiceStreamStatManager(XBee& xbee) {
	this->xbee = xbee;
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
		VoiceStreamStats stream = VoiceStreamStats(packetSource);
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

void VoiceStreamStatManager::updateVoiceLoss(const XBeeAddress64& packetSource, const uint8_t * dataPtr) {

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
		VoiceStreamStats stream = VoiceStreamStats(packetSource);
		stream.startStream();
		stream.updateVoiceLoss(dataPtr);
		streams.push_back(stream);
	}

}

void VoiceStreamStatManager::sendPathPacket() {
	int i = 0;

	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		Serial.print("SENDPATH");
		const XBeeAddress64 &dataSenderAddress = streams.at(i).getSenderAddress();
		const uint8_t dataLoss = streams.at(i).getPacketLoss();

		streams.at(i).calculateThroughput();

		uint8_t payload[] = { 'P', 'A', 'T', 'H', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
				(dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
				dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff,
				(dataSenderAddress.getLsb() >> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff,
				dataSenderAddress.getLsb() & 0xff, dataLoss };
		Tx64Request tx = Tx64Request(streams.at(i).getUpStreamNeighborAddress(), ACK_OPTION, payload, sizeof(payload),
				0);
		xbee.send(tx);

		i++;
	}
}

const vector<VoiceStreamStats> & VoiceStreamStatManager::getStreams() const {
	return streams;
}

void VoiceStreamStatManager::setStreams(const vector<VoiceStreamStats>& streams) {
	this->streams = streams;
}

