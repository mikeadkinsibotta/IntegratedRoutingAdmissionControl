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

const uint8_t HEARTBEAT_PAYLOAD_SIZE = 35;
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
	is_sink = false;
}

HeartbeatProtocol::HeartbeatProtocol(const XBeeAddress64& broadcastAddress, const XBeeAddress64& manipulateAddress,
		const bool manipulateFlag, const XBeeAddress64& myAddress, XBee& xbee, const bool generateData,
		const float distanceDifference, const bool is_sink) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->routeFlag = false;
	this->broadcastAddress = broadcastAddress;
	this->manipulateAddress = manipulateAddress;
	this->manipulate = manipulateFlag;
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = MAX_FLT;
	this->generateData = generateData;
	this->distanceDifference = distanceDifference;
	this->is_sink = is_sink;
	timeoutLength = 0;
	nextHop = Neighbor();
	digitalWrite(13, HIGH);
	hopsToSink = 0;

	if (is_sink) {
		routeFlag = true;
		digitalWrite(13, LOW);
		this->sinkAddress = myAddress;
	}
	buildSaturationTable();
}

void HeartbeatProtocol::broadcastHeartBeat() {

	if (DEBUG) {
		printNeighborHoodTable();
	}

	HeartbeatMessage message = HeartbeatMessage(myAddress, sinkAddress, nextHop.getAddress(), seqNum, dataRate,
			qualityOfPath, neighborhoodCapacity, routeFlag, hopsToSink, generateData, is_sink);

	uint8_t payload[HEARTBEAT_PAYLOAD_SIZE];

	message.generateBeatMessage(payload);

	Tx64Request tx = Tx64Request(broadcastAddress, payload, sizeof(payload));

	xbee.send(tx);

	seqNum++;

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

	if (message.getSenderAddress().equals(nextHop.getAddress())) {
		hopsToSink = message.getHopsToSink() + 1;
	}

	if (message.isIsSink() && !nextHop.equals(Neighbor())) {
		//Check if we should go to another sink?
		switchSinks(message);
	} else {

		if (manipulate) {
			manipulateRoute();
		} else if (!is_sink && nextHop.equals(Neighbor())) {
			noNeighborcalculatePathQualityNextHop();
		} else if (!is_sink && !nextHop.equals(Neighbor())) {
			withNeighborcalculatePathQualityNextHop();
		}
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
					unsigned long timepoint = millis();
					NextHopSwitch nextHopSwitch = NextHopSwitch(timepoint, nextHop.getAddress());
					nextHopSwitchList.push_back(nextHopSwitch);
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

void HeartbeatProtocol::noNeighborcalculatePathQualityNextHop() {

	vector < Neighbor > filterTable;

	digitalWrite(13, HIGH);
	SerialUSB.println("I need a nextHop!");

	for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
			++it) {
		if (it->second.isRouteFlag() && !(it->second.getNextHop().equals(myAddress))) {
			filterTable.push_back(it->second);
		}
	}

	//check if any neighbors have routes
	if (filterTable.size() > 0) {
		Neighbor neighbor;
		neighbor.setHopsToSink(UINT8_MAX);
		uint8_t qop = UINT8_MAX;

		lookForBetterHop(neighbor, filterTable, qop);

		qualityOfPath = qop;
		nextHop = neighbor;
		sinkAddress = neighbor.getSinkAddress();
		unsigned long timepoint = millis();
		NextHopSwitch nextHopSwitch = NextHopSwitch(timepoint, nextHop.getAddress());
		nextHopSwitchList.push_back(nextHopSwitch);
		routeFlag = true;
		digitalWrite(13, LOW);

	} else {
		//reset path if path does not exist
		qualityOfPath = 0;
		nextHop = Neighbor();
		unsigned long timepoint = millis();
		NextHopSwitch nextHopSwitch = NextHopSwitch(timepoint, nextHop.getAddress());
		nextHopSwitchList.push_back(nextHopSwitch);
		routeFlag = false;
		digitalWrite(13, HIGH);
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
//	unsigned long timeStamp = nextHop.getTimeStamp();
//	unsigned long previousTimeStamp = nextHop.getPreviousTimeStamp();
//	unsigned long diff = timeStamp - previousTimeStamp;
//	diff = diff / 1000.0;
//	double distanceDiff = abs(nextHop.getRelativeDistanceAvg() - nextHop.getPreviousRelativeDistance());
	std::vector < Neighbor > filterTable;

	for (std::map<XBeeAddress64, Neighbor>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();
			++it) {
		if (it->second.isRouteFlag() && !(it->second.getNextHop().equals(myAddress)) && !(it->second.equals(nextHop))) {
			filterTable.push_back(it->second);
		}
	}

	//Check if any other neighbors have next hops, if not don't do anything
	if (filterTable.size() > 0) {
		uint8_t qop = nextHop.getQualityOfPath();
		Neighbor neighbor = nextHop;
		lookForBetterHop(neighbor, filterTable, qop);

		// Only need to make switch if we found neighbor with better route
		if (!neighbor.equals(nextHop)) {

			qualityOfPath = qop;
			nextHop = neighbor;
			sinkAddress = neighbor.getSinkAddress();
			unsigned long timepoint = millis();
			NextHopSwitch nextHopSwitch = NextHopSwitch(timepoint, nextHop.getAddress());
			nextHopSwitchList.push_back(nextHopSwitch);
			routeFlag = true;
			digitalWrite(13, LOW);

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

void HeartbeatProtocol::lookForBetterHop(Neighbor& neighbor, vector<Neighbor>& filterTable, uint8_t& qop) const {

	vector < Neighbor > secondaryChoiceTable;
	vector < Neighbor > primaryChoiceTable;

	for (vector<Neighbor>::iterator it = filterTable.begin(); it != filterTable.end(); ++it) {

		if (it->isGenerateData()) {
			secondaryChoiceTable.push_back(*it);
		} else {
			primaryChoiceTable.push_back(*it);
		}
	}

	//Avoid generating data neighbors or neighbors with not much difference in distance if possible
	filterTable = primaryChoiceTable.size() > 0 ? primaryChoiceTable : secondaryChoiceTable;

	for (std::vector<Neighbor>::iterator it = filterTable.begin(); it != filterTable.end(); ++it) {
		const uint8_t path = neighborhoodTable.size() + 1 + it->getQualityOfPath();

		/*
		 *1.  Look for neighbor with fewest hops
		 *2.  If hops are the same, check if neighbor has smaller qop
		 *3.  If hops the same, qop the same, then choose based on smaller relative distance, only switch though if its a large difference
		 */

		if (neighbor.getHopsToSink() > it->getHopsToSink()) {
			neighbor = *it;
			qop = path;
		} else if (neighbor.getHopsToSink() == it->getHopsToSink() && path < qop) {
			qop = path;
			neighbor = *it;
		} else if (neighbor.getHopsToSink() == it->getHopsToSink() && qop == path
				&& neighbor.getRelativeDistanceAvg() > it->getRelativeDistanceAvg()) {

			//We are in this block cause only relative distance is left for criteria for switching hops.
			//But we don't want to use this too much cause it will cause a nexthop bounce effect.

			double differenceDistance = abs(neighbor.getRelativeDistanceAvg() - it->getRelativeDistanceAvg());
			if (differenceDistance > distanceDifference) {
				SerialUSB.print("Difference:  ");
				SerialUSB.println(differenceDistance);
				neighbor = *it;
				qop = path;
			}
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
			unsigned long timepoint = millis();
			NextHopSwitch nextHopSwitch = NextHopSwitch(timepoint, nextHop.getAddress());
			nextHopSwitchList.push_back(nextHopSwitch);
			digitalWrite(13, LOW);
		}
	}
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

void HeartbeatProtocol::sendEndMessage(const uint8_t nextHopSwitchIndex) {

	uint8_t payload[33];
	memset(payload, 0, sizeof(payload));

	NextHopSwitch nextHopSwitch = nextHopSwitchList.at(nextHopSwitchIndex);

	unsigned long timePoint = nextHopSwitch.getTimePoint();

	memset(payload, 0, sizeof(payload));

	payload[0] = 'E';
	payload[1] = 'N';
	payload[2] = 'D';
	payload[3] = 'M';
	payload[4] = '\0';

	HeartbeatMessage::addAddressToMessage(payload, myAddress, 5);
	HeartbeatMessage::addAddressToMessage(payload, sinkAddress, 13);
	HeartbeatMessage::addAddressToMessage(payload, nextHopSwitch.getNewNextHop(), 21);

	const uint8_t * timeP = reinterpret_cast<uint8_t*>(&timePoint);

	payload[29] = timeP[0];
	payload[30] = timeP[1];
	payload[31] = timeP[2];
	payload[32] = timeP[3];

	Tx64Request tx = Tx64Request(nextHop.getAddress(), ACK_OPTION, payload, 33, DEFAULT_FRAME_ID);
	xbee.send(tx);

}

void HeartbeatProtocol::handleEndPacket(const Rx64Response &response) {

//Extract the packet's final destination

//check to see if the packet final destination is this node's address
//If not setup another request to forward it.

	uint8_t * dataPtr = response.getData();

	XBeeAddress64 packetDestination, packetSource, newNextHop, previousHop;

	previousHop = response.getRemoteAddress64();

	HeartbeatMessage::setAddress(dataPtr, packetSource, 5);
	HeartbeatMessage::setAddress(dataPtr, packetDestination, 13);
	HeartbeatMessage::setAddress(dataPtr, newNextHop, 21);

	unsigned long timepoint;
	memcpy(&timepoint, response.getData() + 29, sizeof(unsigned long));

	if (!myAddress.equals(packetDestination)) {

		//need to forward to next hop
		Tx64Request tx = Tx64Request(nextHop.getAddress(), ACK_OPTION, response.getData(), response.getDataLength(),
				DEFAULT_FRAME_ID);
		xbee.send(tx);

	} else {
		SerialUSB.print("#");
		packetSource.printAddressASCII(&SerialUSB);
		SerialUSB.print("   #");
		newNextHop.printAddressASCII(&SerialUSB);
		SerialUSB.print("   ");
		SerialUSB.println(timepoint / 1000.0);

		SerialUSB.print("End Message From: ");
		packetSource.printAddressASCII(&SerialUSB);
		SerialUSB.print("  ");
		SerialUSB.print("Next Hop Switch: ");
		newNextHop.printAddressASCII(&SerialUSB);
		SerialUSB.print("  ");
		SerialUSB.print("Timepoint: ");
		SerialUSB.println(timepoint / 1000.0);
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

void HeartbeatProtocol::switchSinks(const HeartbeatMessage& heartbeatMessage) {

	double differenceDistance = abs(nextHop.getRelativeDistanceAvg() - heartbeatMessage.getRelativeDistance());
	if (differenceDistance > distanceDifference) {
		SerialUSB.println("Switching sink...");
		sinkAddress = heartbeatMessage.getSinkAddress();
	}

}

void HeartbeatProtocol::printNeighborHoodTable() {
	SerialUSB.println("Neighborhood Table:");

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

const vector<NextHopSwitch>& HeartbeatProtocol::getNextHopSwitchList() const {
	return nextHopSwitchList;
}

const XBeeAddress64& HeartbeatProtocol::getSinkAddress() const {
	return sinkAddress;
}
