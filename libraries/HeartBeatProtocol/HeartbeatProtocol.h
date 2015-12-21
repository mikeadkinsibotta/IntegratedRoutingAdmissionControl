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

using namespace std;

class HeartbeatProtocol {

	private:
		vector<Neighbor> neighborhoodTable;
		XBee xbee;
		float seqNum;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		bool routeFlag;

	public:
		HeartbeatProtocol();
		HeartbeatProtocol(XBeeAddress64 &myAddress, XBee& xbee);
		void broadcastHeartBeat();
		void receiveHeartBeat(const Rx64Response& response);
		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);
		HeartbeatMessage transcribeHeartbeatPacket(const Rx64Response& response);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
