/*
 * HeartBeatProtocol.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatProtocol.h"

#define SINK_ADDRESS 0x40b519cc

HeartbeatProtocol::HeartbeatProtocol(XBee& xbee) {
	this->seqNum = 0;
	this->xbee = xbee;

}

void HeartbeatProtocol::broadcastHeartBeat() {

	XBeeAddress64 sink = XBeeAddress64();

	HeartbeatMessage message = HeartbeatMessage(sink, 0.0, seqNum, 0.0, 0, 0, 0);
	message.sendMessage(xbee);


}

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response) {
	HeartbeatMessage message = transcribeHeartbeatPacket(response);
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
	dataRateU.b[0] = dataPtr[14];

	neighborhoodCapacityRateU.b[0] = dataPtr[18];

	uint8_t qualityOfPath = dataPtr[22];
	uint8_t routeFlag = dataPtr[23];

	HeartbeatMessage message = HeartbeatMessage(senderAddress, sinkAddress, rssi, seqNum, dataRateU.rate, qualityOfPath,
			neighborhoodCapacityRateU.rate, routeFlag);

	return message;

}

