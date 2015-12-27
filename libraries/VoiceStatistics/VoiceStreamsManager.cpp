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
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop);
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
		VoiceStreamStats stream = VoiceStreamStats(packetSource, previousHop);
		streams.push_back(stream);
	}

}

void VoiceStreamStatManager::sendPathPacket() {
	int i = 0;

	for (vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		Serial.print("SENDPATH");
		const XBeeAddress64 &dataSenderAddress = streams.at(i).getSenderAddress();
		const uint8_t dataLoss = streams.at(i).getPacketLoss();

		uint8_t payload[] = { 'P', 'A', 'T', 'H', '\0', (dataSenderAddress.getMsb() >> 24) & 0xff,
				(dataSenderAddress.getMsb() >> 16) & 0xff, (dataSenderAddress.getMsb() >> 8) & 0xff,
				dataSenderAddress.getMsb() & 0xff, (dataSenderAddress.getLsb() >> 24) & 0xff,
				(dataSenderAddress.getLsb() >> 16) & 0xff, (dataSenderAddress.getLsb() >> 8) & 0xff,
				dataSenderAddress.getLsb() & 0xff, dataLoss };
		Tx64Request tx = Tx64Request(streams.at(i).getUpStreamNeighborAddress(), payload, sizeof(payload));
		xbee.send(tx);

		i++;
	}
}

const XBeeAddress64& VoiceStreamStatManager::getStreamPreviousHop(const XBeeAddress64& packetSource) {

	XBeeAddress64 previousHop;
	for (int i = 0; i < streams.size(); i++) {
		if (streams.at(i).getSenderAddress().equals(packetSource)) {
			previousHop = streams.at(i).getUpStreamNeighborAddress();
			break;
		}
	}

	return previousHop;
}

const vector<VoiceStreamStats> & VoiceStreamStatManager::getStreams() const {
	return streams;
}

void VoiceStreamStatManager::setStreams(const vector<VoiceStreamStats>& streams) {
	this->streams = streams;
}

