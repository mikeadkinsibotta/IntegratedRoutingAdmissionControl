/*
 * AODV.h
 *
 *  Created on: Jul 24, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_AODV_AODV_H_
#define LIBRARIES_AODV_AODV_H_

#include <XBee.h>
#include <map>
#include "RREQ.h"
#include "RREP.h"
#include "RoutingTableEntry.h"
#include "Neighbor.h"

#define SENDER false

using namespace std;

class AODV {

	private:
		XBeeAddress64 broadcastAddress;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;

		static const uint8_t rreqC[];
		static const uint8_t rrepC[];

		std::map<XBeeAddress64, RoutingTableEntry> routingTable;
		XBee xbee;
		std::map<XBeeAddress64, uint32_t> receivedRREQ;

		uint32_t broadcastId;
		uint32_t sourceSequenceNum;

		bool routeTimerActive = false;
		unsigned long routeTimer = 0;

		uint8_t codecSetting = 0;
		float initialDuplicationSetting = 0;

		void handleRREQ(RREQ& req, const XBeeAddress64& remoteSender);
		void handleRREP(RREP& rep, const XBeeAddress64& remoteSender);
		bool find(const RREQ& req);
		void pathDiscovery(const XBeeAddress64& destination);
	public:
		AODV();
		AODV(const XBee& xbee, const XBeeAddress64& myAddress, const XBeeAddress64& broadcastAddress,
				const XBeeAddress64& sinkAddress, const uint8_t codecSetting, const float initialDuplicationSetting);
		bool routeExists(const XBeeAddress64& address, uint32_t seqNum, uint8_t hopCount, RoutingTableEntry &found);
		void getRoute();
//		const XBeeAddress64& setNextHop();
		void listenForResponses(Rx64Response& response);
		const XBeeAddress64& getNextHop(const XBeeAddress64& destination);
		bool checkRouteTimer();
		void purgeExpiredNeighbors(std::map<XBeeAddress64, Neighbor> &neighbors);
		void printRoutingTable();
		void sendInitPacket(const uint8_t codecSetting, const float dupSetting);
		const XBeeAddress64& getBroadcastAddress() const;
};

#endif /* LIBRARIES_AODV_AODV_H_ */
