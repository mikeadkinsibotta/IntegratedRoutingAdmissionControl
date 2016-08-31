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
#include <math.h>

using namespace std;

class HeartbeatMessage {

	private:
		XBeeAddress64 senderAddress;
		XBeeAddress64 sinkAddress;
		XBeeAddress64 myNextHop;
		uint8_t seqNum;
		float dataRate;
		double rssi;
		double relativeDistance;
		uint8_t qualityOfPath;
		float neighborhoodCapacity;
		bool routeFlag;

	public:
		HeartbeatMessage();

		HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& sinkAddress,
				const XBeeAddress64& myNextHop, const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath,
				const float neighborhoodCapacity, const bool routeFlag);

		void generateBeatMessage(uint8_t payload[]);
		void transcribeHeartbeatPacket(const Rx64Response& response);
		void printMessage();

		const XBeeAddress64& getSenderAddress() const;
		void setSenderAddress(const XBeeAddress64& senderAddress);

		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);

		uint8_t getSeqNum() const;
		void setSeqNum(uint8_t seqNum);

		float getDataRate() const;
		void setDataRate(float dataRate);

		uint8_t getQualityOfPath() const;
		void setQualityOfPath(uint8_t qualityOfPath);

		float getNeighborhoodCapacity() const;
		void setNeighborhoodCapacity(float neighborhoodCapacity);

		bool isRouteFlag() const;
		void setRouteFlag(bool routeFlag);

		double getRelativeDistance() const;
		void setRelativeDistance(double relativeDistance);
		double getRssi() const;
		void setRssi(double rssi);
		const XBeeAddress64& getMyNextHop() const;
		void setMyNextHop(const XBeeAddress64& myNextHop);
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_ */
