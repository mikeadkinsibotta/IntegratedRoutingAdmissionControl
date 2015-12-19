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
#include <vector>
#include "RREQ.h"
#include "RREP.h"
#include "RoutingTableEntry.h"
#include <climits>

using namespace std;

class AODV {

	private:
		static const XBeeAddress64 broadCastaddr64;
		static const XBeeAddress64 broadCastaddr641;
		static const uint8_t rreqC[];
		static const uint8_t rrepC[];
		XBeeAddress64 myAddress;
		std::map<XBeeAddress64, RoutingTableEntry> routingTable;
		XBee xbee;
		std::map<XBeeAddress64, uint32_t> receivedRREQ;

		uint32_t broadcastId;
		uint32_t sourceSequenceNum;

		void handleRREQ(RREQ& req, const XBeeAddress64& remoteSender);
		void handleRREP(RREP& rep, const XBeeAddress64& remoteSender);
		bool find(const RREQ& req);

	public:
		AODV();
		AODV(const XBee& xbee, const XBeeAddress64& myAddress);
		bool routeExists(const XBeeAddress64& address, uint32_t seqNum, uint8_t hopCount, RoutingTableEntry &found);
		void getRoute(const XBeeAddress64& destination, uint8_t hopCount, RoutingTableEntry &foundEntry);
		void pathDiscovery(const XBeeAddress64& destination);
		void listenForResponses();
		void purgeExpiredRoutes();
		void printRoutingTable();

};

#endif /* LIBRARIES_AODV_AODV_H_ */
