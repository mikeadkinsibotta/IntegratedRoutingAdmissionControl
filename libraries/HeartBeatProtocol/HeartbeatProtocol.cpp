/*
 * HeartBeatProtocol.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatProtocol.h"

#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define DEBUG false

const uint8_t HEARTBEAT_PAYLOAD_SIZE = 33;
const float MAX_FLT = 9999.0;
const float EPISLON = 0.001;

HeartbeatProtocol::HeartbeatProtocol() {
	seqNum = 0;
	routeFlag = false;
	qualityOfPath = 0;
	dataRate = 0;
	neighborhoodCapacity = MAX_FLT;
	timeoutLength = 0;
	buildSaturationTable();
	nextHop = Neighbor();
	hopsToSink = 0;

}

HeartbeatProtocol::HeartbeatProtocol(const XBeeAddress64& broadcastAddress, const XBeeAddress64& manipulateAddress,
		const bool manipulateFlag, const XBeeAddress64& myAddress, const XBeeAddress64& sinkAdress, XBee& xbee) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sinkAddress = sinkAdress;
	this->routeFlag = false;
	this->broadcastAddress = broadcastAddress;
	this->manipulateAddress = manipulateAddress;
	this->manipulate = manipulateFlag;
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = MAX_FLT;
	timeoutLength = 0;
	nextHop = Neighbor();
	hopsToSink = 0;

	if (myAddress.equals(sinkAddress)) {
		routeFlag = true;
	}
	buildSaturationTable();
}

void HeartbeatProtocol::broadcastHeartBeat() {

	if (DEBUG) {
		printNeighborHoodTable();
	}

	HeartbeatMessage message = HeartbeatMessage(myAddress, sinkAddress, nextHop.getAddress(), seqNum, dataRate,
			qualityOfPath, neighborhoodCapacity, routeFlag, hopsToSink);

	uint8_t payload[HEARTBEAT_PAYLOAD_SIZE];

	message.generateBeatMessage(payload);

	Tx64Request tx = Tx64Request(broadcastAddress, payload, sizeof(payload));

	xbee.send(tx);

	seqNum++;

	if (!myAddress.equals(sinkAddress) && nextHop.equals(Neighbor())) {
		noNeighborcalculatePathQualityNextHop();
	} else if (!myAddress.equals(sinkAddress) && !nextHop.equals(Neighbor())) {
		withNeighborcalculatePathQualityNextHop();
	}

}

void HeartbeatProtocol::reCalculateNeighborhoodCapacity() {
	float neighborhoodRate = 0;
	uint8_t neighborhoodSize = 0;

	for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
			++it) {
		neighborhoodRate += it->second.getDataRate();
		if (abs(it->second.getDataRate() - 0) > EPISLON) {
			neighborhoodSize++;
		}
	}

	//Don't forget to include myself
	neighborhoodRate += dataRate;

	if (abs(dataRate - 0) > EPISLON) {
		neighborhoodSize++;
	}

	if (neighborhoodSize == 1 || neighborhoodSize == 0) {
		neighborhoodCapacity = MAX_FLT;
	} else if (neighborhoodSize > 1) {
		float saturationRate = satT[neighborhoodSize - 2].getRate();
		neighborhoodCapacity = saturationRate - neighborhoodRate;
	}

}

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response) {

	HeartbeatMessage message;
	message.transcribeHeartbeatPacket(response);

	updateNeighborHoodTable(message);
	reCalculateNeighborhoodCapacity();

	if (!myAddress.equals(sinkAddress) && nextHop.equals(Neighbor())) {
		noNeighborcalculatePathQualityNextHop();
	} else if (!myAddress.equals(sinkAddress) && !nextHop.equals(Neighbor())) {
		withNeighborcalculatePathQualityNextHop();
	}
}

void HeartbeatProtocol::updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage) {

	if (neighborhoodTable.find(heartbeatMessage.getSenderAddress()) != neighborhoodTable.end()) {
		Neighbor * neighbor = &neighborhoodTable[heartbeatMessage.getSenderAddress()];
		updateNeighbor(neighbor, heartbeatMessage);
		if (neighbor->getAddress().equals(nextHop.getAddress())) {
			updateNeighbor(&nextHop, heartbeatMessage);
		}
	} else {
		Neighbor neighbor = Neighbor(heartbeatMessage.getSenderAddress(), heartbeatMessage.getMyNextHop(),
				heartbeatMessage.getDataRate(), heartbeatMessage.getSeqNum(), heartbeatMessage.getQualityOfPath(),
				heartbeatMessage.getNeighborhoodCapacity(), heartbeatMessage.isRouteFlag(),
				heartbeatMessage.getSinkAddress(), heartbeatMessage.getRelativeDistance(), heartbeatMessage.getRssi(),
				timeoutLength, heartbeatMessage.getHopsToSink());
		neighborhoodTable.insert(pair<XBeeAddress64, Neighbor>(neighbor.getAddress(), neighbor));
	}

	if (DEBUG) {
		printNeighborHoodTable();
	}

}

bool HeartbeatProtocol::isNeighbor(const XBeeAddress64 &address) {
	return neighborhoodTable.find(address) != neighborhoodTable.end();
}

void HeartbeatProtocol::purgeNeighborhoodTable() {

	if (!neighborhoodTable.empty()) {
		for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
				) {
			if (it->second.timerExpired()) {
				SerialUSB.print("Neighbor: ");
				it->first.printAddressASCII(&SerialUSB);
				SerialUSB.println(" timer has expired and is purged.");
				SerialUSB.print("Nexthop Address: ");
				nextHop.getAddress().printAddressASCII(&SerialUSB);
				SerialUSB.println();
				if (it->first.equals(nextHop.getAddress())) {
					nextHop = Neighbor();
					SerialUSB.println("NextHop Purged");
				}

				neighborhoodTable.erase(it++);

				if (DEBUG) {
					printNeighborHoodTable();
				}

			} else {
				++it;
			}
		}
	}
}

void HeartbeatProtocol::printNeighborHoodTable() {
	SerialUSB.println();
	SerialUSB.print("MyAddress: ");
	myAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", DataRate: ");
	SerialUSB.print(dataRate);
	SerialUSB.print(", SeqNum: ");
	SerialUSB.print(seqNum);
	SerialUSB.print(", HopsToSink: ");
	SerialUSB.print(hopsToSink);
	SerialUSB.print(", QualityOfPath: ");
	SerialUSB.print(qualityOfPath);
	SerialUSB.print(", NeighborhoodCapacity: ");
	SerialUSB.print(neighborhoodCapacity);
	SerialUSB.print(", RouteFlag: ");
	SerialUSB.print(routeFlag);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", NextHopAddress: ");
	nextHop.getAddress().printAddressASCII(&SerialUSB);
	SerialUSB.println();

	for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
			++it) {

		SerialUSB.print("NeighborAddress: ");
		it->first.printAddressASCII(&SerialUSB);
		SerialUSB.print(", DataRate: ");
		SerialUSB.print(it->second.getDataRate());
		SerialUSB.print(", SeqNum: ");
		SerialUSB.print(it->second.getSeqNum());
		SerialUSB.print(", HopsToSink: ");
		SerialUSB.print(it->second.getHopsToSink());
		SerialUSB.print(", QualityOfPath: ");
		SerialUSB.print(it->second.getQualityOfPath());
		SerialUSB.print(", NeighborhoodCapacity: ");
		SerialUSB.print(it->second.getNeighborhoodCapacity());
		SerialUSB.print(", RouteFlag: ");
		SerialUSB.print(it->second.isRouteFlag());
		SerialUSB.print(", SinkAddress: ");
		it->second.getSinkAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", NextHopAddress: ");
		it->second.getNextHop().printAddressASCII(&SerialUSB);
		SerialUSB.print(", RSSI: ");
		SerialUSB.print(it->second.getRssi());
		SerialUSB.print(", RelativeDistanceAvg: ");
		SerialUSB.println(it->second.getRelativeDistanceAvg(), 12);

	}
	SerialUSB.println();
}

void HeartbeatProtocol::noNeighborcalculatePathQualityNextHop() {

//Add 1 to include myself
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
	uint8_t qop = UINT8_MAX;
	Neighbor neighbor;

	if (nextHop.equals(Neighbor())) {
		SerialUSB.println("I need a nextHop!");
		/*If no neighbor is currently selected:
		 * - Pick neighbor with smallest quality of path
		 * - If quality of path is the same, pick with shorter relative distance
		 */
		for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
				++it) {

			if (manipulate) {

				if (it->second.isRouteFlag() && it->first.equals(manipulateAddress)) {
					qualityOfPath = neighborHoodSize + it->second.getQualityOfPath();
					nextHop = it->second;
					routeFlag = true;
					return;
				}
			} else if (it->second.isRouteFlag() && !(it->second.getNextHop().equals(myAddress))) {
				uint8_t path = neighborHoodSize + it->second.getQualityOfPath();

				if (path < qop) {
					qop = path;
					neighbor = it->second;

				} else if (qop == path) {
					double relativeDistanceCurrent = neighbor.getRelativeDistanceAvg();
					double relativeDistanceNew = it->second.getRelativeDistanceAvg();
					if (relativeDistanceCurrent > relativeDistanceNew) {
						neighbor = it->second;
					}
				}
			}
		}
	}

//Make sure route exists
	if (qop != UINT8_MAX) {
		qualityOfPath = qop;
		nextHop = neighbor;
		routeFlag = true;

	} else {
		//reset path if path does not exist
		qualityOfPath = 0;
		nextHop = Neighbor();
		routeFlag = false;
	}

}

void HeartbeatProtocol::withNeighborcalculatePathQualityNextHop() {

	//TODO
	/*Quality of path is not union as described in proposal.  If we have
	 * a path in which two nodes both affect the same neighbor node we count
	 * that neighbor node twice.  We don't union.  Union would eliminate duplicate nodes and only count
	 * the neighbor once.
	 */

//Neighbor is already selected.  Should I make some adjustments?
//SerialUSB.print("Should I adjust my nextHop neighbor?  ");
	unsigned long timeStamp = nextHop.getTimeStamp();
	unsigned long previousTimeStamp = nextHop.getPreviousTimeStamp();
	unsigned long diff = timeStamp - previousTimeStamp;
	diff = diff / 1000.0;
	double distanceDiff = abs(nextHop.getRelativeDistanceAvg() - nextHop.getPreviousRelativeDistance());

	//Check quality of path
	//Add 1 to include myself
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;

	//Get qop for nextHop
	uint8_t nextHopQop = nextHop.getQualityOfPath();

	//Update my own quality of path
	uint8_t currentQop = neighborHoodSize + nextHopQop;

	//TODO check for paths with better qob and check for paths with fewer hops.
	SerialUSB.println("Checking neighbors for better path...");

	//Don't make any changes if manipulate flag is on.
	if (!manipulate) {

		for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
				++it) {

			//check if neighbor has a path first
			if (it->second.isRouteFlag()) {
				SerialUSB.print("Neighbor: ");
				it->first.printAddressASCII(&SerialUSB);
				SerialUSB.println(" has route");

				//check if this route has lower qop
				//if it has a lower qop switch
				uint8_t qopForThisPath = neighborHoodSize + it->second.getQualityOfPath();
				if (qualityOfPath > qopForThisPath) {
					nextHop = it->second;
				} else if (qualityOfPath == qopForThisPath) {

				}
			}
		}
	}

//SerialUSB.print("  Difference from last timeStamp:  ");
//
//SerialUSB.print(diff);
//SerialUSB.print(" seconds   ");

//SerialUSB.print("Difference distance: ");
//SerialUSB.print(distanceDiff);

	/*	if (abs(diff - 0) > EPISLON) {
	 double speed = distanceDiff / diff;
	 SerialUSB.print("   Speed: ");
	 SerialUSB.print(speed, 12);
	 SerialUSB.println(" mps");
	 }*/
}

void HeartbeatProtocol::buildSaturationTable() {
	satT[0] = Saturation(2, 120.90);
	satT[1] = Saturation(3, 153.39);
	satT[2] = Saturation(4, 151.2);
	satT[3] = Saturation(5, 154.45);
	satT[4] = Saturation(6, 111.42);

}

float HeartbeatProtocol::getLocalCapacity() {

	float localCapacity = MAX_FLT;
	for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
			++it) {
		float neighborCapacity = it->second.getNeighborhoodCapacity();
		if (neighborCapacity < localCapacity) {
			localCapacity = neighborCapacity;
		}

	}

	if (neighborhoodCapacity < localCapacity) {
		localCapacity = neighborhoodCapacity;
	}

//	SerialUSB.print("Local Capacity: ");
//	SerialUSB.println(localCapacity);
	return localCapacity;
}

void HeartbeatProtocol::updateNeighbor(Neighbor * neighbor, const HeartbeatMessage& heartbeatMessage) {
	neighbor->setDataRate(heartbeatMessage.getDataRate());
	neighbor->setNextHop(heartbeatMessage.getMyNextHop());
	neighbor->setSeqNum(heartbeatMessage.getSeqNum());
	neighbor->setQualityOfPath(heartbeatMessage.getQualityOfPath());
	neighbor->setNeighborhoodCapacity(heartbeatMessage.getNeighborhoodCapacity());
	neighbor->setRouteFlag(heartbeatMessage.isRouteFlag());
	neighbor->setSinkAddress(heartbeatMessage.getSinkAddress());
	neighbor->addToRelativeDistance(heartbeatMessage.getRelativeDistance());
	neighbor->setRssi(heartbeatMessage.getRssi());
	neighbor->updateTimeStamp();
	neighbor->setHopsToSink(heartbeatMessage.getHopsToSink());

}

bool HeartbeatProtocol::isRouteFlag() const {
	return routeFlag;
}

const Neighbor& HeartbeatProtocol::getNextHop() const {
	return nextHop;
}

float HeartbeatProtocol::getDataRate() const {
	return dataRate;
}

void HeartbeatProtocol::setDataRate(float dataRate) {
	this->dataRate = dataRate;
}

const XBeeAddress64& HeartbeatProtocol::getBroadcastAddress() const {
	return broadcastAddress;
}

void HeartbeatProtocol::setBroadcastAddress(const XBeeAddress64& broadcastAddress) {
	this->broadcastAddress = broadcastAddress;
}

unsigned long HeartbeatProtocol::getTimeoutLength() const {
	return timeoutLength;
}

void HeartbeatProtocol::setTimeoutLength(unsigned long timeoutLength) {
	this->timeoutLength = timeoutLength;
}

const XBee& HeartbeatProtocol::getXbee() const {
	return xbee;
}

void HeartbeatProtocol::setXbee(const XBee& xbee) {
	this->xbee = xbee;
}

