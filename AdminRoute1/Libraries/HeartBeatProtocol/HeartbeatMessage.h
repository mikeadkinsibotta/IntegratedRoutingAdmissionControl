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

class HeartbeatMessage {

	private:
		static const XBeeAddress64 broadCastaddr64;
		XBeeAddress64 senderAddress;
		XBeeAddress64 sinkAddress;
		uint8_t seqNum;
		float dataRate;
		uint8_t rssi;
		uint8_t qualityOfPath;
		float neighborhoodCapacity;
		bool routeFlag;

	public:

		HeartbeatMessage(XBeeAddress64& sinkAddress, uint8_t rssi, uint8_t seqNum, float dataRate,
				uint8_t qualityOfPath, float neighborhoodCapacity, bool routeFlag);

		HeartbeatMessage(XBeeAddress64& senderAddress, XBeeAddress64& sinkAddress, uint8_t rssi, uint8_t seqNum,
				float dataRate, uint8_t qualityOfPath, float neighborhoodCapacity, bool routeFlag);

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

		Tx64Request generatePacket();

		void printMessage();

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_ */
