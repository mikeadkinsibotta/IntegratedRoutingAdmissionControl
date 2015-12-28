/*
 * HeartBeatProtocol.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatProtocol.h"

#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC

HeartbeatProtocol::HeartbeatProtocol() {
	this->seqNum = 0;
	this->myAddress = XBeeAddress64();
	this->routeFlag = false;
	this->sinkAddress = XBeeAddress64();
	this->nextHopAddress = XBeeAddress64();
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = 0;
	//Serial.print("HasRouteD");
	//Serial.print(routeFlag);
}

HeartbeatProtocol::HeartbeatProtocol(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAdress, XBee& xbee) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sinkAddress = sinkAdress;
	this->routeFlag = false;
	this->nextHopAddress = XBeeAddress64();
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = 0;

	if (myAddress.equals(sinkAddress)) {
		routeFlag = true;
	}

	//Serial.print("HasRouteE");
	//Serial.print(routeFlag);
}

void HeartbeatProtocol::broadcastHeartBeat(const XBeeAddress64& heartbeatAddress) {

	HeartbeatMessage message = HeartbeatMessage(XBeeAddress64(), sinkAddress, seqNum, dataRate, qualityOfPath,
			neighborhoodCapacity, routeFlag);

	//message.printMessage();
	//Serial.print("InjectionRateA");
	//Serial.print(dataRate);

	uint8_t payload[32];

	message.generateBeatMessage(payload);

	Tx64Request tx = Tx64Request(heartbeatAddress, payload, sizeof(payload));

	xbee.send(tx);

	seqNum++;

}

void HeartbeatProtocol::reCalculateNeighborhoodCapacity() {
	float neighborhoodRate = 0;

	for (int i = 0; i < neighborhoodTable.size(); i++) {
		neighborhoodRate += neighborhoodTable.at(i).getNeighborDataRate();
	}

	//Don't forget to include myself
	neighborhoodRate += dataRate;

	uint8_t neighborhoodSize = neighborhoodTable.size();

	if (dataRate > 0) {
		neighborhoodSize++;
	}

	if (neighborhoodSize == 1 || neighborhoodSize == 0) {
		neighborhoodCapacity = UINT32_MAX;
	} else if (neighborhoodSize > 1) {
		float saturationRate = satT[neighborhoodSize - 2].getRate();
		neighborhoodCapacity = saturationRate;
	}
}

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response) {
	HeartbeatMessage message;

	transcribeHeartbeatPacket(response, message);

	if (!myAddress.equals(sinkAddress)) {
		routeFlag = message.isRouteFlag();

	}
	updateNeighborHoodTable(message);
	printNeighborHoodTable();

	if (!myAddress.equals(sinkAddress)) {
		calculatePathQualityNextHop();
	}
}

void HeartbeatProtocol::updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage) {

	bool found = false;

	for (int i = 0; i < neighborhoodTable.size(); i++) {
		if (neighborhoodTable.at(i).getNeighborAddress().equals(heartbeatMessage.getSenderAddress())) {
			neighborhoodTable.at(i).setNeighborDataRate(heartbeatMessage.getDataRate());
			neighborhoodTable.at(i).setSeqNum(heartbeatMessage.getSeqNum());
			neighborhoodTable.at(i).setQualityOfPath(heartbeatMessage.getQualityOfPath());
			neighborhoodTable.at(i).setNeighborhoodCapacity(heartbeatMessage.getNeighborhoodCapacity());
			neighborhoodTable.at(i).setRouteFlag(heartbeatMessage.isRouteFlag());
			neighborhoodTable.at(i).setStreamSourceAddress(heartbeatMessage.getStreamSourceAddress());
			neighborhoodTable.at(i).setSinkAddress(heartbeatMessage.getSinkAddress());
			neighborhoodTable.at(i).setRelativeDistance(heartbeatMessage.getRssi());

			found = true;
			break;
		}
	}

	if (!found) {
		Neighbor neighbor = Neighbor(heartbeatMessage.getSenderAddress(), heartbeatMessage.getDataRate(),
				heartbeatMessage.getSeqNum(), heartbeatMessage.getQualityOfPath(),
				heartbeatMessage.getNeighborhoodCapacity(), heartbeatMessage.isRouteFlag(),
				heartbeatMessage.getStreamSourceAddress(), heartbeatMessage.getSinkAddress(),
				heartbeatMessage.getRssi());
		neighborhoodTable.push_back(neighbor);
	}

}

void HeartbeatProtocol::printNeighborHoodTable() {

	SerialUSB.println("Print Neighbor Table");
	for (int i = 0; i < neighborhoodTable.size(); i++) {

		SerialUSB.print("NeighborAddress: ");
		neighborhoodTable.at(i).getNeighborAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", NeighborDataRate: ");
		SerialUSB.print(neighborhoodTable.at(i).getNeighborDataRate());
		SerialUSB.print(", SeqNum: ");
		SerialUSB.print(neighborhoodTable.at(i).getSeqNum());
		SerialUSB.print(", QualityOfPath: ");
		SerialUSB.print(neighborhoodTable.at(i).getQualityOfPath());
		SerialUSB.print(", NeighborhoodCapacity: ");
		SerialUSB.print(neighborhoodTable.at(i).getNeighborhoodCapacity());
		SerialUSB.print(", RouteFlag: ");
		SerialUSB.print(neighborhoodTable.at(i).isRouteFlag());
		SerialUSB.print(", StreamSourceAddress: ");
		neighborhoodTable.at(i).getStreamSourceAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", SinkAddress: ");
		neighborhoodTable.at(i).getSinkAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", RelativeDistance: ");
		SerialUSB.println(neighborhoodTable.at(i).getRelativeDistance());

	}

}

void HeartbeatProtocol::transcribeHeartbeatPacket(const Rx64Response& response, HeartbeatMessage& message) {

	uint8_t* dataPtr = response.getData() + 5;

	XBeeAddress64 senderAddress = response.getRemoteAddress64();

	uint8_t rssi = response.getRssi();
	XBeeAddress64 sinkAddress;
	XBeeAddress64 sourceStreamAddress;

	sinkAddress.setMsb(
			(uint32_t(dataPtr[0]) << 24) + (uint32_t(dataPtr[1]) << 16) + (uint16_t(dataPtr[2]) << 8) + dataPtr[3]);

	sinkAddress.setLsb(
			(uint32_t(dataPtr[4]) << 24) + (uint32_t(dataPtr[5]) << 16) + (uint16_t(dataPtr[6]) << 8) + dataPtr[7]);

	sourceStreamAddress.setMsb(
			(uint32_t(dataPtr[8]) << 24) + (uint32_t(dataPtr[9]) << 16) + (uint16_t(dataPtr[10]) << 8) + dataPtr[11]);

	sourceStreamAddress.setLsb(
			(uint32_t(dataPtr[12]) << 24) + (uint32_t(dataPtr[13]) << 16) + (uint16_t(dataPtr[14]) << 8) + dataPtr[15]);

	uint8_t seqNum = dataPtr[16];
	uint8_t qualityOfPath = dataPtr[17];
	uint8_t routeFlag = dataPtr[18];

	float * dataRateP = (float*) (dataPtr + 19);
	float dataRateNeighbor = *dataRateP;

	dataRateP = (float*) (dataPtr + 23);
	float neighborhoodCapacity = *dataRateP;

	Serial.print("NeighborRate");
	Serial.print(dataRateNeighbor);

	message = HeartbeatMessage(senderAddress, sourceStreamAddress, sinkAddress, rssi, seqNum, dataRateNeighbor,
			qualityOfPath, neighborhoodCapacity, routeFlag);

}

void HeartbeatProtocol::calculatePathQualityNextHop() {
	//Add 1 to include myself
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
	uint8_t qop = UINT8_MAX;
	XBeeAddress64 neighbor;

	for (int i = 0; i < neighborhoodTable.size(); i++) {

		if (neighborhoodTable.at(i).isRouteFlag()) {
			uint8_t path = neighborHoodSize + neighborhoodTable.at(i).getQualityOfPath();

			if (path < qop) {
				qop = path;
				neighbor = neighborhoodTable.at(i).getNeighborAddress();
			}
		}
	}

	//Make sure route exists
	if (qop != UINT8_MAX) {

		qualityOfPath = qop;
		nextHopAddress = neighbor;
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
