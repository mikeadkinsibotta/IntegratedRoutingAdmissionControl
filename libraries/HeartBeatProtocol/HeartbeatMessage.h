/*
 * HeartbeatMessage.h
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_
#define LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_

#include "Arduino.h"
#include "XBee.h"

union injectRate {
		uint8_t b[4];
		float rate;
};

class HeartbeatMessage {

	private:
		static const XBeeAddress64 broadCastaddr64;
		XBeeAddress64 senderAddress;
		XBeeAddress64 sinkAddress;
		XBeeAddress64 streamSourceAddress;
		uint8_t seqNum;
		float dataRate;
		uint8_t rssi;
		uint8_t qualityOfPath;
		float neighborhoodCapacity;
		bool routeFlag;

	public:

		HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& streamSourceAddress,
				const XBeeAddress64& sinkAddress, const uint8_t rssi, const uint8_t seqNum, const float dataRate,
				const uint8_t qualityOfPath, const float neighborhoodCapacity, const bool routeFlag);

		HeartbeatMessage(const XBeeAddress64& streamSourceAddress, const XBeeAddress64& sinkAddress,
				const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath,
				const float neighborhoodCapacity, const bool routeFlag);

		const XBeeAddress64& getStreamSourceAddress() const;
		void setStreamSourceAddress(const XBeeAddress64& streamSourceAddress);

		const XBeeAddress64& getSenderAddress() const;
		void setSenderAddress(const XBeeAddress64& senderAddress);

		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);

		uint8_t getSeqNum() const;
		void setSeqNum(uint8_t seqNum);

		float getDataRate() const;
		void setDataRate(float dataRate);

		float getRssi() const;
		void setRssi(float rssi);

		uint8_t getQualityOfPath() const;
		void setQualityOfPath(uint8_t qualityOfPath);

		float getNeighborhoodCapacity() const;
		void setNeighborhoodCapacity(float neighborhoodCapacity);

		bool isRouteFlag() const;
		void setRouteFlag(bool routeFlag);

		void sendDataMessage(XBee& xbee);

		void printMessage();

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_ */
