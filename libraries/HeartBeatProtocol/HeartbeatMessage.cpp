/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

const XBeeAddress64 HeartbeatMessage::broadCastaddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& streamSourceAddress,
		const XBeeAddress64& sinkAddress, const uint8_t rssi, const uint8_t seqNum, const float dataRate,
		const uint8_t qualityOfPath, const float neighborhoodCapacity, const bool routeFlag) {

	this->senderAddress = senderAddress;
	this->streamSourceAddress = streamSourceAddress;
	this->sinkAddress = sinkAddress;
	this->rssi = rssi;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& streamSourceAddress, const XBeeAddress64& sinkAddress,
		const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath, const float neighborhoodCapacity,
		const bool routeFlag) {

	this->senderAddress = XBeeAddress64();
	this->streamSourceAddress = streamSourceAddress;
	this->sinkAddress = sinkAddress;
	this->rssi = 0;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

}

const XBeeAddress64& HeartbeatMessage::getStreamSourceAddress() const {
	return streamSourceAddress;
}

void HeartbeatMessage::setStreamSourceAddress(const XBeeAddress64& streamSourceAddress) {
	this->streamSourceAddress = streamSourceAddress;
}

const XBeeAddress64& HeartbeatMessage::getSenderAddress() const {
	return senderAddress;
}

void HeartbeatMessage::setSenderAddress(const XBeeAddress64& senderAddress) {
	this->senderAddress = senderAddress;
}

const XBeeAddress64& HeartbeatMessage::getSinkAddress() const {
	return sinkAddress;
}

void HeartbeatMessage::setSinkAddress(const XBeeAddress64& sinkAddress) {
	this->sinkAddress = sinkAddress;
}

uint8_t HeartbeatMessage::getSeqNum() const {
	return seqNum;
}

void HeartbeatMessage::setSeqNum(uint8_t seqNum) {
	this->seqNum = seqNum;
}

float HeartbeatMessage::getDataRate() const {
	return dataRate;
}

void HeartbeatMessage::setDataRate(float dataRate) {
	this->dataRate = dataRate;
}

float HeartbeatMessage::getRssi() const {
	return rssi;
}

void HeartbeatMessage::setRssi(float rssi) {
	this->rssi = rssi;
}

float HeartbeatMessage::getNeighborhoodCapacity() const {
	return neighborhoodCapacity;
}

void HeartbeatMessage::setNeighborhoodCapacity(float neighborhoodCapacity) {
	this->neighborhoodCapacity = neighborhoodCapacity;
}

uint8_t HeartbeatMessage::getQualityOfPath() const {
	return qualityOfPath;
}

void HeartbeatMessage::setQualityOfPath(uint8_t qualityOfPath) {
	this->qualityOfPath = qualityOfPath;
}

bool HeartbeatMessage::isRouteFlag() const {
	return routeFlag;
}

void HeartbeatMessage::setRouteFlag(bool routeFlag) {
	this->routeFlag = routeFlag;
}

void HeartbeatMessage::printMessage() {

	SerialUSB.print("<SenderAddress: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", StreamSourceAddress: ");
	streamSourceAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", seqNum: ");
	SerialUSB.print(seqNum);
	SerialUSB.print(", RSSI: ");
	SerialUSB.print(rssi);
	SerialUSB.print(", DataRate: ");
	SerialUSB.print(dataRate);
	SerialUSB.print(", NeighborhoodCapacity: ");
	SerialUSB.print(neighborhoodCapacity);
	SerialUSB.print(", QualityOfPath: ");
	SerialUSB.print(qualityOfPath);
	SerialUSB.print(", RouteFlag: ");
	SerialUSB.print(routeFlag);
	SerialUSB.println('>');
}

void HeartbeatMessage::sendDataMessage(XBee& xbee) {

	SerialUSB.print("DataRateSend ");
	SerialUSB.println(dataRate);

	uint8_t * dataRateP = (uint8_t *) &dataRate;
	uint8_t * neighborhoodCapacityP = (uint8_t *) &neighborhoodCapacity;

	uint8_t payload[] = { 'B', 'E', 'A', 'T', '\0', (sinkAddress.getMsb() >> 24) & 0xff, (sinkAddress.getMsb() >> 16)
			& 0xff, (sinkAddress.getMsb() >> 8) & 0xff, sinkAddress.getMsb() & 0xff, (sinkAddress.getLsb() >> 24)
			& 0xff, (sinkAddress.getLsb() >> 16) & 0xff, (sinkAddress.getLsb() >> 8) & 0xff, sinkAddress.getLsb()
			& 0xff, (streamSourceAddress.getMsb() >> 24) & 0xff, (streamSourceAddress.getMsb() >> 16) & 0xff,
			(streamSourceAddress.getMsb() >> 8) & 0xff, streamSourceAddress.getMsb() & 0xff,
			(streamSourceAddress.getLsb() >> 24) & 0xff, (streamSourceAddress.getLsb() >> 16) & 0xff,
			(streamSourceAddress.getLsb() >> 8) & 0xff, streamSourceAddress.getLsb() & 0xff, seqNum, qualityOfPath,
			routeFlag, dataRateP[0], dataRateP[1], dataRateP[2], dataRateP[3], neighborhoodCapacityP[0],
			neighborhoodCapacityP[1], neighborhoodCapacityP[2], neighborhoodCapacityP[3] };

	Tx64Request tx = Tx64Request(broadCastaddr64, 0, payload, sizeof(payload), 0);
	xbee.send(tx);

}

