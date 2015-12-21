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

	HeartbeatMessage message = HeartbeatMessage(sinkAddress, 0.0, seqNum, 0.0, 0, 0, routeFlag);
	message.sendDataMessage(xbee);
	seqNum++;

}

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response) {
	HeartbeatMessage message = transcribeHeartbeatPacket(response);

	if (!myAddress.equals(sinkAddress)) {
		SerialUSB.print("UpdatingFlag");
		routeFlag = message.isRouteFlag();
	}

	message.printMessage();
}

void HeartbeatProtocol::updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage) {

}

HeartbeatMessage HeartbeatProtocol::transcribeHeartbeatPacket(const Rx64Response& response) {

	uint8_t* dataPtr = response.getData();

	XBeeAddress64 senderAddress = response.getRemoteAddress64();

	uint8_t rssi = response.getRssi();
	XBeeAddress64 sinkAddress;

	sinkAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	sinkAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	uint8_t seqNum = dataPtr[13];
	uint8_t qualityOfPath = dataPtr[14];
	uint8_t routeFlag = dataPtr[15];

	float * dataRateP = (float*) dataPtr + 16;
	float dataRate = *dataPtr;

	dataPtr = dataPtr + 20;
	dataRateP = (float*) dataPtr;
	float neighborhoodCapacity = *dataRateP;

	HeartbeatMessage message = HeartbeatMessage(senderAddress, sinkAddress, rssi, seqNum, dataRate, qualityOfPath,
			neighborhoodCapacity, routeFlag);

	return message;

}

