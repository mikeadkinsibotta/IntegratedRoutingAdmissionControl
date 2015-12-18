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

class HeartBeatProtcool {

	private:
		vector<Neighbor> neighborhoodTable;
		XBeeAddress64 senderAddress;

		double seqNum;

	public:
		void broadcastHeartBeat();
		void receivedHeartBeat();
		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
