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
		float dataRate;
		float neighborhoodCapacity;
		float packetLoss;
		uint8_t seqNum;
		uint8_t qualityOfPath;
		bool routeFlag;
		double rssi;
		unsigned long timeStamp;
		unsigned long timeoutLength;

	public:
		Neighbor();
		Neighbor(const XBeeAddress64 &address, float neighborDataRate, uint8_t seqNum, float qosCost,
				float neighborhoodCapacity, bool routeFlag, const XBeeAddress64& sinkAddress, double relativeDistance,
				double rssi, unsigned long timeoutLength);

		const XBeeAddress64& getAddress() const;
		void setAddress(const XBeeAddress64& address);
		float getDataRate() const;
		void setDataRate(float dataRate);
		float getNeighborhoodCapacity() const;
		void setNeighborhoodCapacity(float neighborhoodCapacity);
		float getPacketLoss() const;
		void setPacketLoss(float packetLoss);
		uint8_t getQualityOfPath() const;
		void setQualityOfPath(uint8_t qualityOfPath);
		double getRelativeDistance() const;
		void setRelativeDistance(double relativeDistance);
		bool isRouteFlag() const;
		void setRouteFlag(bool routeFlag);
		uint8_t getSeqNum() const;
		void setSeqNum(uint8_t seqNum);
		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);
		void updateTimeStamp();
		bool checkTimer();
		bool compare(const Neighbor &b);
		void printNeighbor() const;
		double getRssi() const;
		void setRssi(double rssi);

};

#endif
