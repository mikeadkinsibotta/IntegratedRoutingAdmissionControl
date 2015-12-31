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
		XBeeAddress64 address;
		XBeeAddress64 sinkAddress;
		double relativeDistance;
		float neighborDataRate;
		float neighborhoodCapacity;
		float packetLoss;
		uint8_t seqNum;
		uint8_t qualityOfPath;
		bool routeFlag;

	public:
		Neighbor();
		Neighbor(const XBeeAddress64 &address, float neighborDataRate, uint8_t seqNum, float qosCost,
				float neighborhoodCapacity, bool routeFlag, const XBeeAddress64& sinkAddress, double relativeDistance);

		const XBeeAddress64& getAddress() const;
		void setAddress(const XBeeAddress64& address);

		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);

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

		double getRelativeDistance() const;
		void setRelativeDistance(double relativeDistance);

		bool compare(const Neighbor &b);
		void printNeighbor() const;

};

#endif
