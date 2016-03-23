#include "AODV.h"

#define DEBUG true
#define PRINT_ROUTE_DEBUG true

const uint8_t AODV::rreqC[] = { 'R', 'R', 'E', 'Q', '\0' };
const uint8_t AODV::rrepC[] = { 'R', 'R', 'E', 'P', '\0' };
void AODV::listenForResponses(Rx64Response& response, const char control[]) {

	XBeeAddress64 remoteSender = response.getRemoteAddress64();
	uint8_t* data = response.getData();

	if (!strcmp(control, "RREQ")) {
		//RREQ
		//Serial.print("HandlingRREQ");
		RREQ rreq = RREQ(response.getData());

		handleRREQ(rreq, remoteSender);

	} else if (!strcmp(control, "RREP")) {
		//	Serial.print("HandlingRREP");
		//RREP
		RREP rrep = RREP(response.getData());
		handleRREP(rrep, remoteSender);

	}

}

AODV::AODV() {
	this->broadcastAddress = XBeeAddress64(0x00000000, 0x0000FFFF);
	this->sourceSequenceNum = 0;
	this->broadcastId = 0;
	this->sinkAddress = XBeeAddress64();
	this->codecSetting = 0;
	this->initialDuplicationSetting = 0;
}

AODV::AODV(const XBee& xbee, const XBeeAddress64& myAddress, const XBeeAddress64& broadCastaddr,
		const XBeeAddress64& sinkAddress, const uint8_t codecSetting, const float initialDuplicationSetting) {

	this->broadcastAddress = broadCastaddr;
	this->sinkAddress = sinkAddress;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sourceSequenceNum = 0;
	this->broadcastId = 0;
	this->codecSetting = codecSetting;
	this->initialDuplicationSetting = initialDuplicationSetting;

}

void AODV::getRoute() {
	routeTimer = millis();
	routeTimerActive = true;

	sourceSequenceNum++;
	broadcastId++;
	if (routingTable.count(sinkAddress) == 0) {
		SerialUSB.println("No Route, starting PathDiscovery");
		pathDiscovery (sinkAddress);
	}

}

void AODV::pathDiscovery(const XBeeAddress64& destination) {

	sourceSequenceNum++;
	broadcastId++;

	RREQ request(myAddress, sourceSequenceNum, broadcastId, destination, 0, 0);

	uint8_t payload[] = { rreqC[0], rreqC[1], rreqC[2], rreqC[3], rreqC[4], myAddress.getMsb() >> 24, myAddress.getMsb()
			>> 16, myAddress.getMsb() >> 8, myAddress.getMsb(), myAddress.getLsb() >> 24, myAddress.getLsb() >> 16,
			myAddress.getLsb() >> 8, myAddress.getLsb(), request.getSourceSeqNum(), request.getBroadcastId(),
			destination.getMsb() >> 24, destination.getMsb() >> 16, destination.getMsb() >> 8, destination.getMsb(),
			destination.getLsb() >> 24, destination.getLsb() >> 16, destination.getLsb() >> 8, destination.getLsb(),
			request.getDestSeqNum(), 0 };

	SerialUSB.print("Sending RREQ:  ");
	request.print();
	Tx64Request tx = Tx64Request(broadcastAddress, payload, sizeof(payload));
	xbee.send(tx);

}

void AODV::handleRREQ(RREQ& req, const XBeeAddress64& remoteSender) {
	SerialUSB.print("Receiving RREQ:  ");
	req.print();

	if (routingTable.count(remoteSender) == 0) {
		remoteSender.printAddressASCII(&SerialUSB);
		SerialUSB.println("   Sender not in route table, adding...");

		RoutingTableEntry forwardPathEntry = RoutingTableEntry(remoteSender, remoteSender, 1, 0, millis());

		routingTable.insert(
				std::pair<XBeeAddress64, RoutingTableEntry>(forwardPathEntry.getDestinationAddress(),
						forwardPathEntry));

	} else {
		remoteSender.printAddressASCII(&SerialUSB);
		SerialUSB.println("   Sender in route table, updating...");
		routingTable[remoteSender] = RoutingTableEntry(remoteSender, remoteSender, 1, 0, millis());
	}
	//have I received this RREQ from this source before?
	if (find(req)) {
		SerialUSB.print("I have received RREQ from this source ");
		req.getSourceAddr().printAddressASCII(&SerialUSB);
		SerialUSB.println();

		receivedRREQ[req.getSourceAddr()] = req.getBroadcastId();

		/* First, it first increments the hop count value in the RREQ by one, to
		 * account for the new hop through the intermediate node.
		 */
		req.incrementHopCount();

		/** Check route table for an entry to the source.
		 *  In the event that there is no corresponding entry, an entry is created.
		 */

		/**
		 * Checking for reverse route entry
		 */

		if (routingTable.count(req.getSourceAddr()) == 0) {
			SerialUSB.print("I do not have a reverse route to address:  ");
			req.getSourceAddr().printAddressASCII(&SerialUSB);
			SerialUSB.println("  Need to add reverse route");
			RoutingTableEntry reversePathEntry = RoutingTableEntry(req.getSourceAddr(), remoteSender, req.getHopCount(),
					req.getSourceSeqNum(), millis());

			routingTable.insert(
					std::pair<XBeeAddress64, RoutingTableEntry>(reversePathEntry.getDestinationAddress(),
							reversePathEntry));

		} else {
			SerialUSB.print("I have a reverse route, need to figure out if this route is better");
			SerialUSB.print(" for address:  ");
			req.getSourceAddr().printAddressASCII(&SerialUSB);
			SerialUSB.println();
			/*
			 * the Originator Sequence Number from the RREQ is compared to the
			 * corresponding destination sequence number in the route table entry
			 * and copied if greater than the existing value there. the next hop
			 * in the routing table becomes the node from which the RREQ was received.
			 */
			RoutingTableEntry reverseRoute = routingTable[req.getSourceAddr()];
			if (req.getSourceSeqNum() > reverseRoute.getSeqNumDest()) {
				routingTable[req.getSourceAddr()] = RoutingTableEntry(req.getSourceAddr(), remoteSender,
						req.getHopCount(), req.getSourceSeqNum(), millis());
			}

		}

		/**
		 * Checking for forward route to destination
		 */

		if (req.getDestAddr().equals(myAddress)) {
			if (sourceSequenceNum + 1 == req.getDestSeqNum())
				sourceSequenceNum++;
			SerialUSB.println("I am the sink, I will send a RREP");
			SerialUSB.print("My Address:  ");
			myAddress.printAddressASCII(&SerialUSB);
			SerialUSB.println();

			RREP routeReply = RREP(req.getSourceAddr(), req.getDestAddr(), sourceSequenceNum, 0, 3000);

			uint8_t payload[] = { rrepC[0], rrepC[1], rrepC[2], rrepC[3], rrepC[4], routeReply.getSourceAddr().getMsb()
					>> 24, routeReply.getSourceAddr().getMsb() >> 16, routeReply.getSourceAddr().getMsb() >> 8,
					routeReply.getSourceAddr().getMsb(), routeReply.getSourceAddr().getLsb() >> 24,
					routeReply.getSourceAddr().getLsb() >> 16, routeReply.getSourceAddr().getLsb() >> 8,
					routeReply.getSourceAddr().getLsb(), routeReply.getDestAddr().getMsb() >> 24,
					routeReply.getDestAddr().getMsb() >> 16, routeReply.getDestAddr().getMsb() >> 8,
					routeReply.getDestAddr().getMsb(), routeReply.getDestAddr().getLsb() >> 24,
					routeReply.getDestAddr().getLsb() >> 16, routeReply.getDestAddr().getLsb() >> 8,
					routeReply.getDestAddr().getLsb(), routeReply.getDestSeqNum(), routeReply.getHopCount(),
					routeReply.getLifeTime() };

			RoutingTableEntry rrepRoute = routingTable[req.getSourceAddr()];
			routeReply.print();
			Tx64Request tx = Tx64Request(rrepRoute.getNextHop(), ACK_OPTION, payload, sizeof(payload), 0);
			xbee.send(tx);

		} else if (routingTable.count(req.getDestAddr()) > 0) {
			RoutingTableEntry forwardRoute = routingTable[req.getDestAddr()];
			/*
			 * The intermediate node also updates its route table entry
			 * for the node originating the RREQ by placing the next hop towards the
			 * destination in the precursor list for the reverse route entry
			 */
			RoutingTableEntry reverseRouteEntry = routingTable[req.getSourceAddr()];

			reverseRouteEntry.getActiveNeighbors().push_back(forwardRoute.getNextHop());

			/*
			 * The intermediate node updates the forward route entry by placing the last hop node
			 * into the precursor list for the forward route entry
			 */
			forwardRoute.getActiveNeighbors().push_back(remoteSender);

			if (forwardRoute.getSeqNumDest() >= req.getDestSeqNum()) {
				SerialUSB.println("Here I am an intermediate and I have a route to destination");
				RREP routeReply = RREP(req.getSourceAddr(), req.getDestAddr(), forwardRoute.getSeqNumDest(),
						forwardRoute.getHopCount(), millis() - forwardRoute.getExperiationTime());
				routeReply.print();
				uint8_t payload[] = { rrepC[0], rrepC[1], rrepC[2], rrepC[3], rrepC[4],
						routeReply.getSourceAddr().getMsb() >> 24, routeReply.getSourceAddr().getMsb() >> 16,
						routeReply.getSourceAddr().getMsb() >> 8, routeReply.getSourceAddr().getMsb(),
						routeReply.getSourceAddr().getLsb() >> 24, routeReply.getSourceAddr().getLsb() >> 16,
						routeReply.getSourceAddr().getLsb() >> 8, routeReply.getSourceAddr().getLsb(),
						routeReply.getDestAddr().getMsb() >> 24, routeReply.getDestAddr().getMsb() >> 16,
						routeReply.getDestAddr().getMsb() >> 8, routeReply.getDestAddr().getMsb(),
						routeReply.getDestAddr().getLsb() >> 24, routeReply.getDestAddr().getLsb() >> 16,
						routeReply.getDestAddr().getLsb() >> 8, routeReply.getDestAddr().getLsb(),
						routeReply.getDestSeqNum(), routeReply.getHopCount(), routeReply.getLifeTime() };

				Tx64Request tx = Tx64Request(remoteSender, ACK_OPTION, payload, sizeof(payload), 0);
				xbee.send(tx);

			}

		} else {
			//Serial.print("NoRouteRebroadcast");
			SerialUSB.println("Damn, no route found.  Rebroadcast");
			req.print();

			uint8_t payload[] =
					{ rreqC[0], rreqC[1], rreqC[2], rreqC[3], rreqC[4], req.getSourceAddr().getMsb() >> 24,
							req.getSourceAddr().getMsb() >> 16, req.getSourceAddr().getMsb() >> 8,
							req.getSourceAddr().getMsb(), req.getSourceAddr().getLsb() >> 24,
							req.getSourceAddr().getLsb() >> 16, req.getSourceAddr().getLsb() >> 8,
							req.getSourceAddr().getLsb(), req.getSourceSeqNum(), req.getBroadcastId(),
							req.getDestAddr().getMsb() >> 24, req.getDestAddr().getMsb() >> 16,
							req.getDestAddr().getMsb() >> 8, req.getDestAddr().getMsb(), req.getDestAddr().getLsb()
									>> 24, req.getDestAddr().getLsb() >> 16, req.getDestAddr().getLsb() >> 8,
							req.getDestAddr().getLsb(), req.getDestSeqNum(), req.getHopCount() };

			Tx64Request tx = Tx64Request(broadcastAddress, ACK_OPTION, payload, sizeof(payload), 0);
			xbee.send(tx);

		}
	} else {
		//Serial.print("AlreadyRREQ");
	}
}

void AODV::handleRREP(RREP& routeReply, const XBeeAddress64& remoteSender) {
	RoutingTableEntry foundRoute;

	/* When a node receives a RREP message, it searches for a route to the previous hop.  */
	if (routingTable.count(remoteSender) == 0) {
		SerialUSB.println("I received a RREP, but I don't have a route to the previous hop need to add to my table.");
		//No Route Exists in table for forward pointing route

		RoutingTableEntry forwardPathEntry = RoutingTableEntry(remoteSender, remoteSender, 1, 0, millis());

		//set up precursor list
		vector < XBeeAddress64 > activeNeighbors;
		activeNeighbors.push_back(remoteSender);
		forwardPathEntry.setActiveNeighbors(activeNeighbors);

		routingTable.insert(std::pair<XBeeAddress64, RoutingTableEntry>(remoteSender, forwardPathEntry));

	} else {
		SerialUSB.println("I have a route to the sender");
		foundRoute = routingTable[remoteSender];

		routingTable[remoteSender] = RoutingTableEntry(remoteSender, remoteSender, 1, routeReply.getDestSeqNum(),
				millis());
	}

	/*
	 * the node then increments the hop count
	 * value in the RREP by one, to account for the new hop through the
	 * intermediate node.
	 */
	routeReply.incrementHopCount();

	if (routingTable.count(routeReply.getDestAddr()) == 0) {
		//No Route Exists in table for forward pointing route
		//Now Check for route to RREP Destination

		RoutingTableEntry forwardPathEntry = RoutingTableEntry(routeReply.getDestAddr(), remoteSender,
				routeReply.getHopCount(), routeReply.getDestSeqNum(), millis());

		routingTable.insert(
				std::pair<XBeeAddress64, RoutingTableEntry>(forwardPathEntry.getDestinationAddress(),
						forwardPathEntry));

	} else {

		foundRoute = routingTable[routeReply.getDestAddr()];

		// Route does exist, but the next hop may be different.  Is this new possible route fresher?

		/*  Otherwise, the node compares the Destination Sequence Number in
		 * the message with its own stored destination sequence number for
		 * the Destination IP Address in the RREP message.  Upon comparison,
		 * the existing entry is updated only in the following circumstances:
		 *
		 * the Destination Sequence Number in the RREP is greater than
		 * the node's copy of the destination sequence number and the
		 * known value is valid, or
		 *
		 * the sequence numbers are the same, but the route is is
		 * marked as inactive, or
		 *
		 * the sequence numbers are the same, and the New Hop Count is
		 * smaller than the hop count in route table entry.
		 *
		 *
		 *
		 */

		if ((routeReply.getDestSeqNum() > foundRoute.getSeqNumDest())
				|| (routeReply.getDestSeqNum() == foundRoute.getSeqNumDest()
						&& routeReply.getHopCount() < foundRoute.getHopCount())) {

			/**
			 * the next hop in the route entry is assigned to be the node from which the RREP is received,
			 * which is indicated by the source  address field in the 802.15.4 header,
			 *
			 * -  the hop count is set to the value of the New Hop Count,
			 *
			 * -  the expiry time is set to the current time plus the value of the Lifetime in the RREP message,
			 * -  and the destination sequence number is the Destination Sequence Number in the RREP message.
			 */

			//Update the old route
			foundRoute = RoutingTableEntry(routeReply.getDestAddr(), remoteSender, routeReply.getHopCount(),
					routeReply.getDestSeqNum(), millis() + routeReply.getLifeTime());

		}
	}

	/*
	 * If the current node is not the node indicated by the Originator
	 * Address in the RREP message AND a forward route has been created or
	 * updated as described above, the node consults its route table entry
	 * for the originating node to determine the next hop for the RREP
	 * packet, and then forwards the RREP towards the originator using the
	 * information in that route table entry.
	 */

//OK forward route pointer figured out.  Now where to I send the RREP message?
//Fist, check to see if I need to foward at all.  This node may be the original node looking for a route
	if (!myAddress.equals(routeReply.getSourceAddr())) {
		XBeeAddress64 reverseRouteNextHop;

		//Look in the routing table for the reverse route that was created through the RREQ.
		//to get the next hop.  This is where the RREP will be sent

		RoutingTableEntry reverseRouteEntry = routingTable[routeReply.getSourceAddr()];
		reverseRouteNextHop = reverseRouteEntry.getNextHop();

		/* When any node transmits a RREP, the precursor list for the
		 * corresponding destination node is updated by adding to it the next
		 * hop node to which the RREP is forwarded.
		 */

		RoutingTableEntry forwardRouteEntry = routingTable[routeReply.getDestAddr()];
		reverseRouteEntry.getActiveNeighbors().push_back(reverseRouteNextHop);

		/*
		 * Finally, the precursor list for the next hop towards the destination
		 * is updated to contain the next hop towards the source.
		 */
		RoutingTableEntry next = routingTable[routingTable[routeReply.getDestAddr()].getNextHop()];
		next.getActiveNeighbors().push_back(reverseRouteNextHop);

		uint8_t payload[] = { rrepC[0], rrepC[1], rrepC[2], rrepC[3], rrepC[4], routeReply.getSourceAddr().getMsb()
				>> 24, routeReply.getSourceAddr().getMsb() >> 16, routeReply.getSourceAddr().getMsb() >> 8,
				routeReply.getSourceAddr().getMsb(), routeReply.getSourceAddr().getLsb() >> 24,
				routeReply.getSourceAddr().getLsb() >> 16, routeReply.getSourceAddr().getLsb() >> 8,
				routeReply.getSourceAddr().getLsb(), routeReply.getDestAddr().getMsb() >> 24,
				routeReply.getDestAddr().getMsb() >> 16, routeReply.getDestAddr().getMsb() >> 8,
				routeReply.getDestAddr().getMsb(), routeReply.getDestAddr().getLsb() >> 24,
				routeReply.getDestAddr().getLsb() >> 16, routeReply.getDestAddr().getLsb() >> 8,
				routeReply.getDestAddr().getLsb(), routeReply.getDestSeqNum(), routeReply.getHopCount(),
				routeReply.getLifeTime() };

		Tx64Request tx = Tx64Request(reverseRouteNextHop, ACK_OPTION, payload, sizeof(payload), 0);
		xbee.send(tx);
	} else {
		//Route has been setup Can transmit data
		SerialUSB.println("Route has been setup Can transmit data");

		//admissionControl->sendInitPacket(CODEC_SETTTING, INITAL_DUPLICATION_SETTING);
		sendInitPacket(codecSetting, initialDuplicationSetting);
	}
}

bool AODV::find(const RREQ& req) {

	if (req.getSourceAddr().equals(myAddress))
		//make sure to ignore my own RREQ
		return false;

//check if this <source_addr, broadcast_id> has been received by this node
	if (receivedRREQ.count(req.getSourceAddr()) > 0) {
		//rreq from this address has been received
		uint32_t broadcast_id = receivedRREQ[req.getSourceAddr()];
		if (broadcast_id >= req.getBroadcastId()) {
			//broadcast_id in table is greater than or equal to this req ignore
			return false;
		}

	}

//new RREQ handle it.
	return true;
}

void AODV::purgeExpiredRoutes() {

	std::map < XBeeAddress64, RoutingTableEntry > newRoutingTable;

	vector < XBeeAddress64 > v;
	for (std::map<XBeeAddress64, RoutingTableEntry>::iterator it = routingTable.begin(); it != routingTable.end();
			++it) {
		v.push_back(it->first);
	}

	uint8_t size = routingTable.size();
	for (int i = 0; i < routingTable.size(); ++i) {

		if (millis() - routingTable[v.at(i)].getExperiationTime() > 3000) {
			routingTable[v.at(i)].setActive(false);
		}
	}

}

bool AODV::checkRouteTimer() {
	if (routeTimerActive && millis() - routeTimer < 3000) {
		return false;
	}
	routeTimerActive = false;
	routeTimer = 0;
	return true;

}

//const XBeeAddress64& AODV::setNextHop() {
//	if (routingTable.count(sinkAddress) == 0) {
//		return XBeeAddress64();
//	}
//
//	RoutingTableEntry entry = routingTable[sinkAddress];
//	nextHop = entry.getNextHop();
//
//}

void AODV::printRoutingTable() {

	SerialUSB.println("PrintRoutingTable");
	for (std::map<XBeeAddress64, RoutingTableEntry>::const_iterator it = routingTable.begin(); it != routingTable.end();
			it++) {
		XBeeAddress64 key = it->first;
		RoutingTableEntry value = it->second;

		SerialUSB.print("<DestinationAddress: ");
		value.getDestinationAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", NextHop: ");
		value.getNextHop().printAddressASCII(&SerialUSB);
		SerialUSB.print(", HopCount: ");
		SerialUSB.print(value.getHopCount());
		SerialUSB.print(", SeqNumDest: ");
		SerialUSB.print(value.getSeqNumDest());
		SerialUSB.println('>');
	}
}

void AODV::sendInitPacket(const uint8_t codecSetting, const float dupSetting) {
	//TODO fixed next hop
	RoutingTableEntry entry = routingTable[sinkAddress];
	XBeeAddress64 nextHop = entry.getNextHop();

	if (!nextHop.equals(XBeeAddress64())) {
		SerialUSB.println("Sending Init");

		float injectionRate = 64.00 * (codecSetting / 16.00) * (1.00 + dupSetting);
		uint8_t * injectionRateP = (uint8_t *) &injectionRate;

		SerialUSB.print("Injection Rate: ");
		SerialUSB.println(injectionRate);
		uint8_t payloadBroadCast[25];

		payloadBroadCast[0] = 'I';
		payloadBroadCast[1] = 'N';
		payloadBroadCast[2] = 'I';
		payloadBroadCast[3] = 'T';
		payloadBroadCast[4] = '\0';
		payloadBroadCast[5] = (myAddress.getMsb() >> 24) & 0xff;
		payloadBroadCast[6] = (myAddress.getMsb() >> 16) & 0xff;
		payloadBroadCast[7] = (myAddress.getMsb() >> 8) & 0xff;
		payloadBroadCast[8] = myAddress.getMsb() & 0xff;
		payloadBroadCast[9] = (myAddress.getLsb() >> 24) & 0xff;
		payloadBroadCast[10] = (myAddress.getLsb() >> 16) & 0xff;
		payloadBroadCast[11] = (myAddress.getLsb() >> 8) & 0xff;
		payloadBroadCast[12] = myAddress.getLsb() & 0xff;
		payloadBroadCast[13] = (nextHop.getMsb() >> 24) & 0xff;
		payloadBroadCast[14] = (nextHop.getMsb() >> 16) & 0xff;
		payloadBroadCast[15] = (nextHop.getMsb() >> 8) & 0xff;
		payloadBroadCast[16] = nextHop.getMsb() & 0xff;
		payloadBroadCast[17] = (nextHop.getLsb() >> 24) & 0xff;
		payloadBroadCast[18] = (nextHop.getLsb() >> 16) & 0xff;
		payloadBroadCast[19] = (nextHop.getLsb() >> 8) & 0xff;
		payloadBroadCast[20] = nextHop.getLsb() & 0xff;
		payloadBroadCast[21] = injectionRateP[0];
		payloadBroadCast[22] = injectionRateP[1];
		payloadBroadCast[23] = injectionRateP[2];
		payloadBroadCast[24] = injectionRateP[3];

		Tx64Request tx = Tx64Request(broadcastAddress, payloadBroadCast, sizeof(payloadBroadCast));
		// send the command
		xbee.send(tx);
	}
}

const XBeeAddress64& AODV::getNextHop(const XBeeAddress64& destination) {
	RoutingTableEntry entry = routingTable[destination];
	return entry.getNextHop();

}
