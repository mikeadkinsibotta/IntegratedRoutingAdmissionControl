/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

const XBeeAddress64 HeartbeatMessage::broadCastaddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);

HeartbeatMessage::HeartbeatMessage(XBeeAddress64& senderAddress, XBeeAddress64& sinkAddress, uint8_t rssi,
		uint8_t seqNum, float dataRate, uint8_t qualityOfPath, float neighborhoodCapacity, bool routeFlag) {

	this->senderAddress = senderAddress;
	this->sinkAddress = sinkAddress;
	this->rssi = rssi;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

}

HeartbeatMessage::HeartbeatMessage(XBeeAddress64& sinkAddress, uint8_t rssi, uint8_t seqNum, float dataRate,
		uint8_t qualityOfPath, float neighborhoodCapacity, bool routeFlag) {

	this->sinkAddress = sinkAddress;
	this->rssi = rssi;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

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

	Serial.print('<');
	senderAddress.printAddressASCII(&Serial);
	Serial.print(',');
	sinkAddress.printAddressASCII(&Serial);
	Serial.print(',');
	Serial.print(seqNum);
	Serial.print(',');
	Serial.print(dataRate);
	Serial.print(',');
	Serial.print(neighborhoodCapacity);
	Serial.print(',');
	Serial.print(qualityOfPath);
	Serial.print(',');
	Serial.print(routeFlag);
	Serial.println('>');
}

void HeartbeatMessage::sendDataMessage(XBee& xbee) {

	uint8_t * dataRateP = (uint8_t *) &dataRate;
	uint8_t * neighborhoodCapacityP = (uint8_t *) &neighborhoodCapacity;

	uint8_t payload[] = { 'B', 'E', 'A', 'T', '\0', (sinkAddress.getMsb() >> 24) & 0xff, (sinkAddress.getMsb() >> 16)
			& 0xff, (sinkAddress.getMsb() >> 8) & 0xff, sinkAddress.getMsb() & 0xff, (sinkAddress.getLsb() >> 24)
			& 0xff, (sinkAddress.getLsb() >> 16) & 0xff, (sinkAddress.getLsb() >> 8) & 0xff, sinkAddress.getLsb()
			& 0xff, seqNum, qualityOfPath, routeFlag, dataRateP[0], dataRateP[1], dataRateP[2], dataRateP[3],
			neighborhoodCapacityP[0], neighborhoodCapacityP[1], neighborhoodCapacityP[2], neighborhoodCapacityP[3] };

	Tx64Request tx = Tx64Request(broadCastaddr64, 0, payload, sizeof(payload), 0);
	xbee.send(tx);

}

