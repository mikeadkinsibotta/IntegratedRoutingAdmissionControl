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
}

HeartbeatProtocol::HeartbeatProtocol(XBeeAddress64 &myAddress, XBee& xbee) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sinkAddress = XBeeAddress64(SINK_ADDRESS_1, SINK_ADDRESS_2);
	this->routeFlag = false;

	if (myAddress.equals(sinkAddress)) {
		routeFlag = true;
	}
}

void HeartbeatProtocol::broadcastHeartBeat() {

	HeartbeatMessage message = HeartbeatMessage(XBeeAddress64(), sinkAddress, seqNum, 0.0, 0, 0.0, routeFlag);
	message.sendDataMessage(xbee);
	seqNum++;

}

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response) {
	HeartbeatMessage message = transcribeHeartbeatPacket(response);

	if (!myAddress.equals(sinkAddress)) {
		routeFlag = message.isRouteFlag();
	}

	message.printMessage();
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

HeartbeatMessage HeartbeatProtocol::transcribeHeartbeatPacket(const Rx64Response& response) {

	uint8_t* dataPtr = response.getData();

	XBeeAddress64 senderAddress = response.getRemoteAddress64();

	uint8_t rssi = response.getRssi();
	XBeeAddress64 sinkAddress;
	XBeeAddress64 sourceStreamAddress;

	sinkAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	sinkAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	sourceStreamAddress.setMsb(
			(uint32_t(dataPtr[13]) << 24) + (uint32_t(dataPtr[14]) << 16) + (uint16_t(dataPtr[15]) << 8) + dataPtr[16]);

	sourceStreamAddress.setLsb(
			(uint32_t(dataPtr[17]) << 24) + (uint32_t(dataPtr[18]) << 16) + (uint16_t(dataPtr[19]) << 8) + dataPtr[20]);

	uint8_t seqNum = dataPtr[21];
	uint8_t qualityOfPath = dataPtr[22];
	uint8_t routeFlag = dataPtr[23];

	float * dataRateP = (float*) dataPtr[24];
	float dataRate = *dataPtr;

	SerialUSB.print("DataRateTranscribe ");
	SerialUSB.println(dataRate);

	dataRateP = (float*) dataPtr[28];
	float neighborhoodCapacity = *dataRateP;

	HeartbeatMessage message = HeartbeatMessage(senderAddress, sourceStreamAddress, sinkAddress, rssi, seqNum, dataRate,
			qualityOfPath, neighborhoodCapacity, routeFlag);

	return message;

}

