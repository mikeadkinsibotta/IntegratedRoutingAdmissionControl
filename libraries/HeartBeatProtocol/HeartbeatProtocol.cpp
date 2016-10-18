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

const uint8_t HEARTBEAT_PAYLOAD_SIZE = 34;
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
		const bool manipulateFlag, const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, XBee& xbee,
		const bool generateData) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->routeFlag = false;
	this->broadcastAddress = broadcastAddress;
	this->manipulateAddress = manipulateAddress;
	this->manipulate = manipulateFlag;
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = MAX_FLT;
	this->generateData = generateData;
	timeoutLength = 0;
	nextHop = Neighbor();
	digitalWrite(13, HIGH);
	hopsToSink = 0;

	if (myAddress.equals(sinkAddress)) {
		routeFlag = true;
		digitalWrite(13, LOW);
	}
	buildSaturationTable();
}

void HeartbeatProtocol::broadcastHeartBeat() {

	if (DEBUG) {
		printNeighborHoodTable();
	}

	HeartbeatMessage message = HeartbeatMessage(myAddress, sinkAddress, nextHop.getAddress(), seqNum, dataRate,
			qualityOfPath, neighborhoodCapacity, routeFlag, hopsToSink, generateData);

	uint8_t payload[HEARTBEAT_PAYLOAD_SIZE];

	message.generateBeatMessage(payload);

	Tx64Request tx = Tx64Request(broadcastAddress, payload, sizeof(payload));

	xbee.send(tx);

	seqNum++;

//	if (!myAddress.equals(sinkAddress) && nextHop.equals(Neighbor())) {
//		noNeighborcalculatePathQualityNextHop();
//	} else if (!myAddress.equals(sinkAddress) && !nextHop.equals(Neighbor())) {
//		withNeighborcalculatePathQualityNextHop();
//	}

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

	if (message.getSenderAddress().equals(nextHop.getAddress())) {
		hopsToSink = message.getHopsToSink() + 1;
	}

	updateNeighborHoodTable(message);
	reCalculateNeighborhoodCapacity();

	if (manipulate) {
		manipulateRoute();
	} else if (!myAddress.equals(sinkAddress) && nextHop.equals(Neighbor())) {
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
				timeoutLength, heartbeatMessage.getHopsToSink(), heartbeatMessage.isGenerateData());
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

				if (it->first.equals(nextHop.getAddress())) {
					SerialUSB.print("Nexthop Address: ");
					nextHop.getAddress().printAddressASCII(&SerialUSB);
					SerialUSB.println();
					nextHop = Neighbor();
					routeFlag = false;
					SerialUSB.println("NextHop Purged");
					digitalWrite(13, HIGH);
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
	SerialUSB.print(", GenerateData: ");
	SerialUSB.print(generateData);
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
		SerialUSB.print(", GenerateData: ");
		SerialUSB.print(it->second.isGenerateData());
		SerialUSB.print(", RelativeDistanceAvg: ");
		SerialUSB.println(it->second.getRelativeDistanceAvg(), 4);

	}
	SerialUSB.println();
}

void HeartbeatProtocol::noNeighborcalculatePathQualityNextHop() {

//Add 1 to include myself
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
	uint8_t qop = UINT8_MAX;
	Neighbor neighbor;
	std::vector < Neighbor > filterTable;
	std::vector < Neighbor > generateTable;
	std::vector < Neighbor > noGenerateTable;

	digitalWrite(13, HIGH);
	SerialUSB.println("I need a nextHop!");
	/* If no neighbor is currently selected:
	 * - Pick neighbor with smallest quality of path
	 * - If quality of path is the same, pick with shorter relative distance
	 */
	for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
			++it) {
		if (it->second.isRouteFlag() && !(it->second.getNextHop().equals(myAddress))) {
			filterTable.push_back(it->second);
		}
	}

	//check if any neighbors have routes
	if (filterTable.size() > 0) {

		for (std::vector<Neighbor>::iterator it = filterTable.begin(); it != filterTable.end(); ++it) {
			if (it->isGenerateData()) {
				generateTable.push_back(*it);
			} else {
				noGenerateTable.push_back(*it);
			}
		}

		//Avoid generating data neighbors if possible
		if (noGenerateTable.size() > 0) {
			filterTable = generateTable;
		} else {
			filterTable = noGenerateTable;
		}

		for (std::vector<Neighbor>::iterator it = filterTable.begin(); it != filterTable.end(); ++it) {
			uint8_t path = neighborHoodSize + it->getQualityOfPath();
			if (path < qop) {
				qop = path;
				if (neighbor.equals(Neighbor())) {
					neighbor = *it;
				}

			} else if (qop == path && nextHop.getHopsToSink() > it->getHopsToSink()) {
				double relativeDistanceCurrent = neighbor.getRelativeDistanceAvg();
				double relativeDistanceNew = it->getRelativeDistanceAvg();
				if (relativeDistanceCurrent > relativeDistanceNew) {
					neighbor = *it;
				}
			}
		}

		//Make sure route exists
		if (qop != UINT8_MAX) {
			qualityOfPath = qop;
			nextHop = neighbor;
			double timepoint = millis() / 1000.0;
			SerialUSB.print("Nexthop switch1: ");
			SerialUSB.println(timepoint);
			nextHopSwitchList.push_back(timepoint);
			routeFlag = true;
			digitalWrite(13, LOW);

		} else {
			//reset path if path does not exist
			qualityOfPath = 0;
			nextHop = Neighbor();
			double timepoint = millis() / 1000.0;
			SerialUSB.print("Nexthop switch2: ");
			SerialUSB.println(timepoint);
			nextHopSwitchList.push_back(timepoint);
			routeFlag = false;
			digitalWrite(13, HIGH);
		}
	}
}

void HeartbeatProtocol::manipulateRoute() {

	if (neighborhoodTable.find(manipulateAddress) != neighborhoodTable.end()) {
		Neighbor neighbor = neighborhoodTable[manipulateAddress];
		if (neighbor.isRouteFlag() && !neighbor.equals(nextHop)) {
			uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
			qualityOfPath = neighborHoodSize + neighbor.getQualityOfPath();
			nextHop = neighbor;
			routeFlag = true;
			double timepoint = millis() / 1000.0;
			SerialUSB.print("Nexthop switch: ");
			SerialUSB.println(timepoint);
			nextHopSwitchList.push_back(timepoint);
			digitalWrite(13, LOW);
		}
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
//SerialUSB.println("Checking neighbors for better path...");

//Don't make any changes if manipulate flag is on.
	if (!manipulate) {

		for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
				++it) {

			//check if neighbor has a path first
			//make sure that this next hop is also not generating data
			if (it->second.isRouteFlag() && (!it->second.isGenerateData() && nextHop.isGenerateData())) {
				//SerialUSB.print("Neighbor: ");
				//it->first.printAddressASCII(&SerialUSB);
				//SerialUSB.println(" has route");

				//check if this route has lower qop
				//if it has a lower qop switch
				//if the qop is the same pick the neighbor with the lower number of hops
				uint8_t qopForThisPath = neighborHoodSize + it->second.getQualityOfPath();
				if (qualityOfPath > qopForThisPath) {
					nextHop = it->second;
					double timepoint = millis() / 1000.0;
					SerialUSB.print("Nexthop switch: ");
					SerialUSB.println(timepoint);
					nextHopSwitchList.push_back(timepoint);
				} else if (qualityOfPath == qopForThisPath && nextHop.getHopsToSink() > it->second.getHopsToSink()) {
					nextHop = it->second;
					double timepoint = millis() / 1000.0;
					SerialUSB.print("Nexthop switch: ");
					SerialUSB.println(timepoint);
					nextHopSwitchList.push_back(timepoint);
				}
			}
		}
	}

//	SerialUSB.print("  Difference from last timeStamp:  ");
//	SerialUSB.print(diff);
//	SerialUSB.print(" seconds   ");
//	SerialUSB.print("Difference distance: ");
//	SerialUSB.print(distanceDiff);
//
//	if (abs(diff - 0) > EPISLON) {
//		double speed = distanceDiff / diff;
//		SerialUSB.print("   Speed: ");
//		SerialUSB.print(speed, 12);
//		SerialUSB.println(" mps");
//	}
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

void HeartbeatProtocol::sendEndMessage() {
	SerialUSB.println("Sending End Message");

	uint8_t length = nextHopSwitchList.size();

	uint8_t * payload = (uint8_t*) malloc(22 + (length * 8));
	payload[0] = 'E';
	payload[1] = 'N';
	payload[2] = 'D';
	payload[3] = 'M';
	payload[4] = '\0';

	payload[21] = length;

	HeartbeatMessage::addAddressToMessage(payload, myAddress, 5);
	HeartbeatMessage::addAddressToMessage(payload, sinkAddress, 13);

	int index = 0;

	SerialUSB.print("Num End Points: ");
	SerialUSB.println(length);

	for (uint8_t i = 22; i < 22 + (length * 8);) {
		SerialUSB.print("TimePoint:  ");
		double timePoint = nextHopSwitchList.at(index);
		SerialUSB.println(timePoint);
		const uint8_t * timeP = reinterpret_cast<uint8_t*>(&timePoint);

		payload[i] = timeP[0];
		payload[i + 1] = timeP[1];
		payload[i + 2] = timeP[2];
		payload[i + 3] = timeP[3];
		payload[i + 4] = timeP[4];
		payload[i + 5] = timeP[5];
		payload[i + 6] = timeP[6];
		payload[i + 7] = timeP[7];
		index++;
		i += 8;
	}

	Tx64Request tx = Tx64Request(nextHop.getAddress(), payload, 22 + (length * 8));
	xbee.send(tx);
	free(payload);
}

void HeartbeatProtocol::handleEndPacket(const Rx64Response &response) {

//Extract the packet's final destination

//check to see if the packet final destination is this node's address
//If not setup another request to forward it.

	uint8_t * dataPtr = response.getData();

	XBeeAddress64 packetDestination, packetSource;

	HeartbeatMessage::setAddress(dataPtr, packetSource, 5);
	HeartbeatMessage::setAddress(dataPtr, packetDestination, 13);

	if (!myAddress.equals(packetDestination)) {

		//need to forward to next hop
		Tx64Request tx = Tx64Request(nextHop.getAddress(), response.getData(), response.getDataLength());
		xbee.send(tx);

	} else {
		SerialUSB.print("End Message From: ");
		packetSource.printAddressASCII(&SerialUSB);
		SerialUSB.print("  ");
		SerialUSB.print("Timepoints: ");

		uint8_t length = dataPtr[21];
		double timepoint;

		for (uint8_t i = 22; i < 22 + (length * 8);) {
			memcpy(&timepoint, response.getData() + i, sizeof(double));
			SerialUSB.print(timepoint);
			SerialUSB.print(",  ");
			i += 8;
		}

		SerialUSB.println();

	}

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
	neighbor->setGenerateData(heartbeatMessage.isGenerateData());

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

