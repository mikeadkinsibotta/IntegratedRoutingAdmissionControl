/*
 * VoiceStreamStats.h
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_VOICESTATISTICS_VOICESTREAMSTATS_H_
#define LIBRARIES_VOICESTATISTICS_VOICESTREAMSTATS_H_

#include "XBee.h"

class VoiceStreamStats {

	private:
		double rValueArray[8] = { 0 };
		uint8_t packetLossArray[8] = { 0 };
		int rValueHead = 0;
		uint8_t packetLossHead = 0;
		XBeeAddress64 senderAddress;
		XBeeAddress64 upStreamNeighborAddress;
		double throughput;
		uint8_t totalPacketsRecieved;
		uint8_t expectedFrameId;
		uint8_t packetLoss;
		uint8_t receivedFrame;
		uint8_t intervalStartFrame;
		double timeStamp;
		uint8_t totalPacketsSent;
		double voiceQuality;
		uint8_t duplicateFrame;
		uint8_t payloadSize;
		uint8_t codecSetting;
		uint8_t numNoPacketReceived;

	public:
		VoiceStreamStats(const XBeeAddress64& senderAddress, const uint8_t payloadSize);
		VoiceStreamStats(const XBeeAddress64& senderAddress, const XBeeAddress64& upStreamNeighborAddress,
				const uint8_t payloadSize);
		const XBeeAddress64& getSenderAddress() const;
		double getThroughput() const;
		void calculateThroughput(const double timeDifference);
		void updateVoiceLoss(const uint8_t * dataPtr);
		void printRouteEnd() const;
		void startStream() const;
		uint8_t getPacketLoss() const;
		const XBeeAddress64& getUpStreamNeighborAddress() const;
		void setUpStreamNeighborAddress(const XBeeAddress64& upStreamNeighborAddress);
		uint8_t getTotalPacketsRecieved() const;
		void setTotalPacketsRecieved(uint8_t totalPacketsRecieved);
		uint8_t getTotalPacketsSent() const;
		void setTotalPacketsSent(uint8_t totalPacketsSent);
		uint8_t getNumNoPacketReceived() const;
};

#endif /* LIBRARIES_VOICESTATISTICS_VOICESTREAMSTATS_H_ */
