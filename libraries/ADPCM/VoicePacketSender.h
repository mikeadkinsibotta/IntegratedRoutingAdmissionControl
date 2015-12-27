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
#include <VoicePacketSender.h>
#include <HeartbeatProtocol.h>>
#include <ThreadController.h>
#include <math.h>

class VoicePacketSender {
	private:
		uint8_t codecSetting;
		float dupSetting;
		ADPCM admcpm;
		XBee xbee;
		HeartbeatProtocol * heartbeatProtocol;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		XBeeAddress64 myNextHop;
		VoiceStreamStatManager voiceStreamStatManager;
		Thread * pathLoss;

		uint8_t frameId;
		uint8_t* addDestinationToPayload(const XBeeAddress64& packetSource, const XBeeAddress64& packetDestination,
				const uint8_t * payload, const uint8_t sizePayload, uint8_t& resultSize, const uint8_t frameId);

	public:
		VoicePacketSender();
		VoicePacketSender(XBee& xbee, HeartbeatProtocol * heartbeatProtocol, Thread * pathLoss,
				const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const uint8_t codecSetting,
				const float dupSetting);
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
		void sendPathPacket();
		void handlePathPacket(const Rx64Response &response);
};

#endif /* LIBRARIES_ADPCM_VOICEPACKETSENDER_H_ */