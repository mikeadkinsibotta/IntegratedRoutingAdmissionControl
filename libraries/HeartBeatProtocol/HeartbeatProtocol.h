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
#include <vector>
#include <set>
#include <cfloat>

using namespace std;

class HeartbeatProtocol {

	private:
		vector<Neighbor> neighborhoodTable;
		XBee xbee;
		uint8_t seqNum;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		XBeeAddress64 broadcastAddress;
		bool routeFlag;
		Neighbor nextHop;
		uint8_t qualityOfPath;
		float dataRate;
		float neighborhoodCapacity;
		Saturation satT[4];
		unsigned long timeoutLength;

		void buildSaturationTable();
		void reCalculateNeighborhoodCapacity();
		void updateNeighbor(Neighbor& neighbor, const HeartbeatMessage& message);
		void noNeighborcalculatePathQualityNextHop();
		void withNeighborcalculatePathQualityNextHop();

	public:
		HeartbeatProtocol();
		HeartbeatProtocol(const XBeeAddress64& broadcastAddress, const XBeeAddress64& myAddress,
				const XBeeAddress64& sinkAdress, XBee& xbee);
		void broadcastHeartBeat();
		void receiveHeartBeat(const Rx64Response& response, bool ignoreHeartBeatFlag);
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
		float getLocalCapacity() const;
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
