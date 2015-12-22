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
#include <vector>
#include <set>

using namespace std;

class HeartbeatProtocol {

	private:
		vector<Neighbor> neighborhoodTable;
		XBee xbee;
		float seqNum;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		bool routeFlag;
		XBeeAddress64 nextHopAddress;
		uint8_t qualityOfPath;

	public:
		HeartbeatProtocol();
		HeartbeatProtocol(XBeeAddress64 &myAddress, XBee& xbee);
		void broadcastHeartBeat();
		void receiveHeartBeat(const Rx64Response& response);
		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);
		const HeartbeatMessage& transcribeHeartbeatPacket(const Rx64Response& response);
		void calculatePathQualityNextHop();
		void printNeighborHoodTable();
		bool isRouteFlag() const;
		const XBeeAddress64& getNextHopAddress() const;
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
