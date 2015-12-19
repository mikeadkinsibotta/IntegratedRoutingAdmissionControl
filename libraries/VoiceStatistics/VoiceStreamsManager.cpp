#include "VoiceStreamsManager.h"

/*
 * VoiceStreamsManager.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

void VoiceStreamStatManager::calcuateThroughput(const XBeeAddress64& packetSource) {
	bool found = false;

	for(int i = 0; i < streams.size(); i++) {
		//SerialUSB.println("Old Stream");
		if(streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).calculateThroughput();
			found = true;
			break;
		}
	}

	if(!found) {
		//SerialUSB.println("New Stream");
		VoiceStreamStats stream = VoiceStreamStats(packetSource);
		stream.startStream();
		stream.calculateThroughput();
		streams.push_back(stream);
	}

}

bool VoiceStreamStatManager::removeStream(const XBeeAddress64& packetSource) {
	int i = 0;
	for(vector<VoiceStreamStats>::iterator it = streams.begin(); it != streams.end(); ++it) {
		if(streams.at(i).getSenderAddress().equals(packetSource)) {
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

	for(int i = 0; i < streams.size(); i++) {
		//SerialUSB.println("Old Stream");

		if(streams.at(i).getSenderAddress().equals(packetSource)) {
			streams.at(i).updateVoiceLoss(dataPtr);

			found = true;
			break;
		}
	}

	if(!found) {
		//SerialUSB.println("New Stream");
		VoiceStreamStats stream = VoiceStreamStats(packetSource);
		stream.startStream();
		stream.updateVoiceLoss(dataPtr);
		streams.push_back(stream);
	}

}

const vector<VoiceStreamStats> & VoiceStreamStatManager::getStreams() const {
	return streams;
}

void VoiceStreamStatManager::setStreams(const vector<VoiceStreamStats>& streams) {
	this->streams = streams;
}
