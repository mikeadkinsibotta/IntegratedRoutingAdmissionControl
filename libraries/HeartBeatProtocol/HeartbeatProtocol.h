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

union conversion {
		uint8_t b[4];
		float rate;
};

class HeartbeatProtocol {

	private:
		vector<Neighbor> neighborhoodTable;
		XBee xbee;
		float seqNum;
		conversion dataRateU;
		conversion neighborhoodCapacityRateU;

	public:
		HeartbeatProtocol(XBee& xbee);
		void broadcastHeartBeat();
		void receiveHeartBeat(const Rx64Response& response);
		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);
		HeartbeatMessage transcribeHeartbeatPacket(const Rx64Response& response);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
