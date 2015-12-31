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
		float dataRate;
		float neighborhoodCapacity;
		Saturation satT[4];
		void buildSaturationTable();
		void reCalculateNeighborhoodCapacity();
	public:
		HeartbeatProtocol();
		HeartbeatProtocol(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAdress, XBee& xbee);
		void broadcastHeartBeat(const XBeeAddress64& heartbeatAddress);
		void receiveHeartBeat(const Rx64Response& response, bool ignoreHeartBeatFlag);

		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);
		void calculatePathQualityNextHop();
		void printNeighborHoodTable();
		bool isRouteFlag() const;
		const XBeeAddress64& getNextHopAddress() const;
		float getDataRate() const;
		void setDataRate(float dataRate);
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATPROTOCOL_H_ */
