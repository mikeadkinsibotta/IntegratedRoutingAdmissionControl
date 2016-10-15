/*
 * HeartBeatProtocol.h
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_
#define LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_

#include "Arduino.h"
#include "XBee.h"
#include "Neighbor.h"
#include "HeartbeatMessage.h"
#include <Saturation.h>
#include <map>
#include <set>
#include <cfloat>

using namespace std;

class HeartbeatProtocol {

	private:
		std::map<XBeeAddress64, Neighbor> neighborhoodTable;
		XBee xbee;
		uint8_t seqNum;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		XBeeAddress64 broadcastAddress;
		XBeeAddress64 manipulateAddress;
		bool routeFlag;
		bool manipulate;
		Neighbor nextHop;
		uint8_t qualityOfPath;
		float dataRate;
		float neighborhoodCapacity;
		Saturation satT[5];
		uint8_t hopsToSink;
		unsigned long timeoutLength;
		bool generateData;

		void buildSaturationTable();
		void reCalculateNeighborhoodCapacity();
		void updateNeighbor(Neighbor * neighbor, const HeartbeatMessage& message);
		void noNeighborcalculatePathQualityNextHop();
		void withNeighborcalculatePathQualityNextHop();

	public:
		HeartbeatProtocol();
		HeartbeatProtocol(const XBeeAddress64& broadcastAddress, const XBeeAddress64& manipulateAddress,
				const bool manipulateFlag, const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, XBee& xbee,
				const bool generateData);
		void broadcastHeartBeat();
		void receiveHeartBeat(const Rx64Response& response);
		void purgeNeighborhoodTable();
		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);
		bool isNeighbor(const XBeeAddress64 &address);

		void printNeighborHoodTable();
		float requestToStream(XBee &xbee, const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop,
				const float injectionRate);
		bool isRouteFlag() const;
		const Neighbor& getNextHop() const;
		float getDataRate() const;
		void setDataRate(float dataRate);
		const XBeeAddress64& getBroadcastAddress() const;
		void setBroadcastAddress(const XBeeAddress64& broadcastAddress);
		unsigned long getTimeoutLength() const;
		void setTimeoutLength(unsigned long timeoutLength);
		const XBee& getXbee() const;
		void setXbee(const XBee& xbee);
		float getLocalCapacity();
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
