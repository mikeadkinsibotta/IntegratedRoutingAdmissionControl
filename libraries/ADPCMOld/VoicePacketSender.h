/*
 * VoicePacket.h
 *
 *  Created on: Dec 21, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_ADPCM_VOICEPACKETSENDER_H_
#define LIBRARIES_ADPCM_VOICEPACKETSENDER_H_

#include <pcadpcm.h>
#include <XBee.h>
#include <VoiceStreamsManager.h>
#include <AODV.h>
#include <ThreadController.h>
#include <CompressionTable.h>
#include <VoiceSetting.h>
#include <math.h>
#include "TraceMessage.h"
#include "HeartbeatMessage.h"

class VoicePacketSender {
	private:
		uint8_t codecSetting;
		float dupSetting;
		ADPCM admcpm;
		XBee xbee;
		AODV * aodv;
		VoiceStreamManager * voiceStreamManager;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		XBeeAddress64 myNextHop;

		Compression compressionTable;
		Thread * pathLoss;
		float injectionRate;
		uint8_t payloadSize;
		uint8_t frameId;
		bool justSentDup = false;

		uint8_t* addDestinationToPayload(const XBeeAddress64& packetSource, const XBeeAddress64& packetDestination,
				const uint8_t * payload, const uint8_t sizePayload, uint8_t& resultSize, const uint8_t frameId);
		void updateDataRate(uint8_t dataLoss);
		void sendTracePacket();

	public:
		VoicePacketSender();
		VoicePacketSender(XBee& xbee, AODV * aodv, Thread * pathLoss, VoiceStreamManager * voiceStreamManager,
				const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const uint8_t codecSetting,
				const float dupSetting, const uint8_t payloadSize);
		void generateVoicePacket();
		void handleDataPacket(const Rx64Response &response);

		const XBeeAddress64& getMyNextHop() const;
		void setMyNextHop(const XBeeAddress64& myNextHop);
		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);
		uint8_t getCodecSetting() const;
		void setCodecSetting(uint8_t codecSetting);
		float getDupSetting() const;
		void setDupSetting(float dupSetting);
		void handlePathPacket(const Rx64Response &response);
		void handleTracePacket(const Rx64Response &response);
		void resetFrameID();
		float getInjectionRate() const;
};

#endif /* LIBRARIES_ADPCM_VOICEPACKETSENDER_H_ */
