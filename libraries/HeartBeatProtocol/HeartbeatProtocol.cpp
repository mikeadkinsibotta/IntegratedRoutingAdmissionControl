/*
 * HeartBeatProtocol.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatProtocol.h"

#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define DEBUG true

const uint8_t HEARTBEAT_PAYLOAD_SIZE = 24;
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
}

HeartbeatProtocol::HeartbeatProtocol(const XBeeAddress64& broadcastAddress, const XBeeAddress64& myAddress,
		const XBeeAddress64& sinkAdress, XBee& xbee) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sinkAddress = sinkAdress;
	this->routeFlag = false;
	this->broadcastAddress = broadcastAddress;
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = MAX_FLT;
	timeoutLength = 0;
	nextHop = Neighbor();

	if (myAddress.equals(sinkAddress)) {
		routeFlag = true;
	}
	buildSaturationTable();
}

void HeartbeatProtocol::broadcastHeartBeat() {

	if (DEBUG) {
		printNeighborHoodTable();
	}

	HeartbeatMessage message = HeartbeatMessage(sinkAddress, seqNum, dataRate, qualityOfPath, neighborhoodCapacity,
			routeFlag);

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
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		neighborhoodRate += neighborhoodTable.at(i).getDataRate();
		if (abs(neighborhoodTable.at(i).getDataRate() - 0) > EPISLON) {
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

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response, bool ignoreHeartBeatFlag) {

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

	bool found = false;

	for (int i = 0; i < neighborhoodTable.size(); i++) {
		if (neighborhoodTable.at(i).getAddress().equals(heartbeatMessage.getSenderAddress())) {
			updateNeighbor(neighborhoodTable.at(i), heartbeatMessage);
			if (neighborhoodTable.at(i).getAddress().equals(nextHop.getAddress())) {
				updateNeighbor(nextHop, heartbeatMessage);
			}
			found = true;
			break;
		}
	}

	if (!found) {
		//Sets timestamp when constructor is called.
		Neighbor neighbor = Neighbor(heartbeatMessage.getSenderAddress(), heartbeatMessage.getDataRate(),
				heartbeatMessage.getSeqNum(), heartbeatMessage.getQualityOfPath(),
				heartbeatMessage.getNeighborhoodCapacity(), heartbeatMessage.isRouteFlag(),
				heartbeatMessage.getSinkAddress(), heartbeatMessage.getRelativeDistance(), heartbeatMessage.getRssi(),
				timeoutLength);
		neighborhoodTable.push_back(neighbor);

		if (DEBUG) {
			printNeighborHoodTable();
		}
	}
}

bool HeartbeatProtocol::isNeighbor(const XBeeAddress64 &address) {
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		if (neighborhoodTable.at(i).getAddress().equals(address)) {
			return true;
		}
	}

	return false;
}

void HeartbeatProtocol::purgeNeighborhoodTable() {

	if (!neighborhoodTable.empty()) {
		for (vector<Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();) {
			if (it->timerExpired()) {
				SerialUSB.print("Neighbor: ");
				it->getAddress().printAddressASCII(&SerialUSB);
				SerialUSB.println(" timer has expired and is purged.");
				SerialUSB.print("Nexthop Address: ");
				nextHop.getAddress().printAddressASCII(&SerialUSB);
				SerialUSB.println();
				if (it->getAddress().equals(nextHop.getAddress())) {
					nextHop = Neighbor();
					SerialUSB.println("NextHop Purged");
				}
				it = neighborhoodTable.erase(it);

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

	for (int i = 0; i < neighborhoodTable.size(); i++) {

		SerialUSB.print("NeighborAddress: ");
		neighborhoodTable.at(i).getAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", DataRate: ");
		SerialUSB.print(neighborhoodTable.at(i).getDataRate());
		SerialUSB.print(", SeqNum: ");
		SerialUSB.print(neighborhoodTable.at(i).getSeqNum());
		SerialUSB.print(", QualityOfPath: ");
		SerialUSB.print(neighborhoodTable.at(i).getQualityOfPath());
		SerialUSB.print(", NeighborhoodCapacity: ");
		SerialUSB.print(neighborhoodTable.at(i).getNeighborhoodCapacity());
		SerialUSB.print(", RouteFlag: ");
		SerialUSB.print(neighborhoodTable.at(i).isRouteFlag());
		SerialUSB.print(", SinkAddress: ");
		neighborhoodTable.at(i).getSinkAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", RSSI: ");
		SerialUSB.print(neighborhoodTable.at(i).getRssi());
		SerialUSB.print(", RelativeDistanceAvg: ");
		SerialUSB.println(neighborhoodTable.at(i).getRelativeDistanceAvg(), 12);

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
		for (int i = 0; i < neighborhoodTable.size(); i++) {

			if (neighborhoodTable.at(i).isRouteFlag()) {
				uint8_t path = neighborHoodSize + neighborhoodTable.at(i).getQualityOfPath();

				if (path < qop) {
					qop = path;
					neighbor = neighborhoodTable.at(i);

				} else if (qop == path) {
					double relativeDistanceCurrent = neighbor.getRelativeDistanceAvg();
					double relativeDistanceNew = neighborhoodTable.at(i).getRelativeDistanceAvg();
					if (relativeDistanceCurrent > relativeDistanceNew) {
						neighbor = neighborhoodTable.at(i);
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

//Add 1 to include myself

//Neighbor is already selected.  Should I make some adjustments?
	//SerialUSB.print("Should I adjust my nextHop neighbor?  ");
	unsigned long timeStamp = nextHop.getTimeStamp();
	unsigned long previousTimeStamp = nextHop.getPreviousTimeStamp();
	unsigned long diff = timeStamp - previousTimeStamp;
	diff = diff / 1000.0;
	double distanceDiff = abs(nextHop.getRelativeDistanceAvg() - nextHop.getPreviousRelativeDistance());

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

float HeartbeatProtocol::getLocalCapacity() const {

	float localCapacity = MAX_FLT;
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		float neighborCapacity = neighborhoodTable.at(i).getNeighborhoodCapacity();
		if (neighborCapacity < localCapacity) {
			localCapacity = neighborCapacity;
		}

	}

	if (neighborhoodCapacity < localCapacity) {
		localCapacity = neighborhoodCapacity;
	}

	SerialUSB.print("Local Capacity: ");
	SerialUSB.println(localCapacity);
	return localCapacity;
}

void HeartbeatProtocol::updateNeighbor(Neighbor& neighbor, const HeartbeatMessage& heartbeatMessage) {
	neighbor.setDataRate(heartbeatMessage.getDataRate());
	neighbor.setSeqNum(heartbeatMessage.getSeqNum());
	neighbor.setQualityOfPath(heartbeatMessage.getQualityOfPath());
	neighbor.setNeighborhoodCapacity(heartbeatMessage.getNeighborhoodCapacity());
	neighbor.setRouteFlag(heartbeatMessage.isRouteFlag());
	neighbor.setSinkAddress(heartbeatMessage.getSinkAddress());
	neighbor.addToRelativeDistance(heartbeatMessage.getRelativeDistance());
	neighbor.setRssi(heartbeatMessage.getRssi());
	neighbor.updateTimeStamp();
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

