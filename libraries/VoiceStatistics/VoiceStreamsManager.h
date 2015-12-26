/*
 * VoiceStreamsManager.h
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_VOICESTATISTICS_VOICESTREAMSMANAGER_H_
#define LIBRARIES_VOICESTATISTICS_VOICESTREAMSMANAGER_H_

#include <vector>
#include "VoiceStreamStats.h"
#include "XBee.h"

using namespace std;

class VoiceStreamStatManager {

	private:
		vector<VoiceStreamStats> streams;
		XBee xbee;

	public:
		VoiceStreamStatManager();
		VoiceStreamStatManager(XBee& xbee);
		void calcuateThroughput(const XBeeAddress64& packetSource);
		void updateVoiceLoss(const XBeeAddress64& packetSource, const uint8_t * dataPtr);
		bool removeStream(const XBeeAddress64& packetSource);
		const vector<VoiceStreamStats> & getStreams() const;
		void setStreams(const vector<VoiceStreamStats>& streams);
		void sendPathPacket();

};

#endif /* LIBRARIES_VOICESTATISTICS_VOICESTREAMSMANAGER_H_ */
