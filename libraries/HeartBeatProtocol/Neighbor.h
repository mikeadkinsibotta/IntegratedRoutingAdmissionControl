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
#include <deque>

using namespace std;

class Neighbor {

	private:
		XBeeAddress64 address;
		XBeeAddress64 sinkAddress;
		double relativeDistance;
		double previousRelativeDistance;
		float dataRate;
		float neighborhoodCapacity;
		float packetLoss;
		uint8_t seqNum;
		uint8_t qualityOfPath;
		bool routeFlag;
		deque<double> rssiQueue;
		double rssiAvg;
		unsigned long previousTimeStamp;
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
		bool timerExpired();
		bool compare(const Neighbor &b);
		void printNeighbor() const;
		void addToRssi(const double rssi);
		unsigned long getTimeStamp() const;
		unsigned long getPreviousTimeStamp() const;
		bool equals(const Neighbor& neighbor) const;
		double getPreviousRelativeDistance() const;
		double getRssiAvg() const;

};

#endif
