/*
 * Neighbor.h
 *
 *  Created on: Jun 15, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_NEIGHBOR_H_
#define LIBRARIES_NEIGHBOR_H_
#include "Arduino.h"
#include "XBee.h"
class Neighbor {

	private:
		XBeeAddress64 neighborAddress;
		XBeeAddress64 streamSourceAddress;
		XBeeAddress64 sinkAddress;
		float relativeDistance;
		float neighborDataRate;
		float neighborhoodCapacity;
		float packetLoss;
		uint8_t seqNum;
		uint8_t qualityOfPath;
		bool routeFlag;

	public:
		Neighbor();
		Neighbor(const XBeeAddress64 &neighborAddress, const float neighborDataRate, const uint8_t seqNum,
				const float qosCost, const float neighborhoodCapacity, const bool routeFlag,
				const XBeeAddress64& streamSourceAddress, const XBeeAddress64& sinkAddress,
				const float relativeDistance);

		const XBeeAddress64& getNeighborAddress() const;
		void setNeighborAddress(const XBeeAddress64& neighborAddress);

		const XBeeAddress64& getStreamSourceAddress() const;
		void setStreamSourceAddress(const XBeeAddress64& streamSourceAddress);

		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);

		float getRelativeDistance() const;
		void setRelativeDistance(float relativeDistance);

		float getNeighborDataRate() const;
		void setNeighborDataRate(float neighborDataRate);

		float getNeighborhoodCapacity() const;
		void setNeighborhoodCapacity(float neighborhoodCapacity);

		float getPacketLoss() const;
		void setPacketLoss(float packetLoss);

		uint8_t getSeqNum() const;
		void setSeqNum(uint8_t seqNum);

		uint8_t getQualityOfPath() const;
		void setQualityOfPath(uint8_t qualityOfPath);

		bool isRouteFlag() const;
		void setRouteFlag(bool routeFlag);

		bool compare(const Neighbor &b);
		void printNeighbor() const;

};

#endif
