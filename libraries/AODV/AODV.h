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

#define SENDER false

using namespace std;

class AODV {

	private:
		XBeeAddress64 broadCastaddr;
		static const uint8_t rreqC[];
		static const uint8_t rrepC[];
		XBeeAddress64 myAddress;
		std::map<XBeeAddress64, RoutingTableEntry> routingTable;
		XBee xbee;
		std::map<XBeeAddress64, uint32_t> receivedRREQ;

		uint32_t broadcastId;
		uint32_t sourceSequenceNum;

		bool routeTimerActive = false;
		unsigned long routeTimer = 0;

		void handleRREQ(RREQ& req, const XBeeAddress64& remoteSender);
		void handleRREP(RREP& rep, const XBeeAddress64& remoteSender);
		bool find(const RREQ& req);

	public:
		AODV();
		AODV(const XBee& xbee, const XBeeAddress64& myAddress, const XBeeAddress64& broadCastaddr);
		bool routeExists(const XBeeAddress64& address, uint32_t seqNum, uint8_t hopCount, RoutingTableEntry &found);
		void getRoute(const XBeeAddress64& destination);
		void pathDiscovery(const XBeeAddress64& destination);
		void listenForResponses(Rx64Response& response, const char control[]);
		bool hasRoute(const XBeeAddress64& destination, XBeeAddress64& nextHop);
		bool checkRouteTimer();
		void purgeExpiredRoutes();
		void printRoutingTable();

};

#endif /* LIBRARIES_AODV_AODV_H_ */
