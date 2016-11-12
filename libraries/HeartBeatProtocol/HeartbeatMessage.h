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
		uint8_t hopsToSink;
		bool generateData;
		bool is_sink;

	public:
		HeartbeatMessage();

		HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& sinkAddress,
				const XBeeAddress64& myNextHop, const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath,
				const float neighborhoodCapacity, const bool routeFlag, const uint8_t hopsToSink,
				const bool generateData, const bool is_sink);

		void generateBeatMessage(uint8_t payload[]);
		void transcribeHeartbeatPacket(const Rx64Response& response);
		void printMessage();
		const XBeeAddress64& getSenderAddress() const;
		const XBeeAddress64& getSinkAddress() const;
		uint8_t getSeqNum() const;
		float getDataRate() const;
		uint8_t getQualityOfPath() const;
		float getNeighborhoodCapacity() const;
		bool isRouteFlag() const;
		double getRelativeDistance() const;
		double getRssi() const;
		const XBeeAddress64& getMyNextHop() const;
		uint8_t getHopsToSink() const;
		void setHopsToSink(uint8_t hopsToSink);
		bool isGenerateData() const;
		static void addAddressToMessage(uint8_t payload[], const XBeeAddress64& address, const uint8_t i);
		static void addFloat(uint8_t payload[], const uint8_t * num, const uint8_t i);
		static void setAddress(const uint8_t * dataPtr, XBeeAddress64& address, const uint8_t i);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_ */
