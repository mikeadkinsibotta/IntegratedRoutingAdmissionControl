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

const double MILLIWATTS = 0.0000000199526231;
const double DISTANCE = 5.5;
const double N_P = -1.095;
const uint8_t HEARTBEAT_PAYLOAD_SIZE = 24;
const float MAX_FLT = 9999.0;

HeartbeatProtocol::HeartbeatProtocol() {
	seqNum = 0;
	routeFlag = false;
	qualityOfPath = 0;
	dataRate = 0;
	neighborhoodCapacity = MAX_FLT;
	timeoutLength = 0;
	buildSaturationTable();
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

}

void HeartbeatProtocol::reCalculateNeighborhoodCapacity() {
	float neighborhoodRate = 0;
	uint8_t neighborhoodSize = 0;
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		neighborhoodRate += neighborhoodTable.at(i).getDataRate();
		if (abs(neighborhoodTable.at(i).getDataRate() - 0) > 0.001) {
			neighborhoodSize++;
		}
	}

	//Don't forget to include myself
	neighborhoodRate += dataRate;

	if (abs(dataRate - 0) > 0.001) {
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

	if (!myAddress.equals(sinkAddress)) {
		routeFlag = message.isRouteFlag();
	}

	updateNeighborHoodTable(message);
	reCalculateNeighborhoodCapacity();

	if (!myAddress.equals(sinkAddress)) {
		calculatePathQualityNextHop();
	}
}

void HeartbeatProtocol::updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage) {

	bool found = false;

	for (int i = 0; i < neighborhoodTable.size(); i++) {
		if (neighborhoodTable.at(i).getAddress().equals(heartbeatMessage.getSenderAddress())) {
			neighborhoodTable.at(i).setDataRate(heartbeatMessage.getDataRate());
			neighborhoodTable.at(i).setSeqNum(heartbeatMessage.getSeqNum());
			neighborhoodTable.at(i).setQualityOfPath(heartbeatMessage.getQualityOfPath());
			neighborhoodTable.at(i).setNeighborhoodCapacity(heartbeatMessage.getNeighborhoodCapacity());
			neighborhoodTable.at(i).setRouteFlag(heartbeatMessage.isRouteFlag());
			neighborhoodTable.at(i).setSinkAddress(heartbeatMessage.getSinkAddress());
			neighborhoodTable.at(i).setRelativeDistance(heartbeatMessage.getRelativeDistance());
			neighborhoodTable.at(i).setRssi(heartbeatMessage.getRssi());
			neighborhoodTable.at(i).updateTimeStamp();
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
	}

}

void HeartbeatProtocol::purgeNeighborhoodTable() {

	if (!neighborhoodTable.empty()) {
		for (vector<Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();) {
			if (it->timerExpired()) {
				SerialUSB.print("Neighbor: ");
				it->getAddress().printAddressASCII(&SerialUSB);
				SerialUSB.println(" timer has expired and is purged.");
				it = neighborhoodTable.erase(it);
			} else {
				++it;
			}
		}
	}
}

void HeartbeatProtocol::printNeighborHoodTable() {

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
	nextHopAddress.printAddressASCII(&SerialUSB);
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
		SerialUSB.print(", RelativeDistance: ");
		SerialUSB.println(neighborhoodTable.at(i).getRelativeDistance(), 12);

	}

}

void HeartbeatProtocol::calculatePathQualityNextHop() {

	//Add 1 to include myself
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
	uint8_t qop = UINT8_MAX;
	Neighbor neighbor;

	for (int i = 0; i < neighborhoodTable.size(); i++) {

		if (neighborhoodTable.at(i).isRouteFlag()) {
			uint8_t path = neighborHoodSize + neighborhoodTable.at(i).getQualityOfPath();

			if (path < qop) {
				qop = path;
				neighbor = neighborhoodTable.at(i);

			} else if (qop == path) {
				double relativeDistanceCurrent = neighbor.getRelativeDistance();
				double relativeDistanceNew = neighborhoodTable.at(i).getRelativeDistance();
				if (relativeDistanceCurrent > relativeDistanceNew) {
					neighbor = neighborhoodTable.at(i);
				}
			}
		}
	}

	//Make sure route exists
	if (qop != UINT8_MAX) {

		qualityOfPath = qop;
		nextHopAddress = neighbor.getAddress();
		routeFlag = true;

	} else {
		//reset path if path does not exist
		qualityOfPath = 0;
		nextHopAddress = XBeeAddress64();
		routeFlag = false;

	}

	/*SerialUSB.print("QualityOfPath ");
	 SerialUSB.println(qualityOfPath);

	 SerialUSB.print("NextHopAddress ");
	 nextHopAddress.printAddressASCII(&SerialUSB);
	 SerialUSB.println();*/

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

bool HeartbeatProtocol::isRouteFlag() const {
	return routeFlag;
}

const XBeeAddress64& HeartbeatProtocol::getNextHopAddress() const {
	return nextHopAddress;
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

