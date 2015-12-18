/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

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

double HeartbeatMessage::getDataRate() const {
	return dataRate;
}

void HeartbeatMessage::setDataRate(double dataRate) {
	this->dataRate = dataRate;
}

double HeartbeatMessage::getRssi() const {
	return rssi;
}

void HeartbeatMessage::setRssi(double rssi) {
	this->rssi = rssi;
}

double HeartbeatMessage::getNeighborhoodCapacity() const {
	return neighborhoodCapacity;
}

void HeartbeatMessage::setNeighborhoodCapacity(double neighborhoodCapacity) {
	this->neighborhoodCapacity = neighborhoodCapacity;
}

double HeartbeatMessage::getQualityOfPath() const {
	return qualityOfPath;
}

void HeartbeatMessage::setQualityOfPath(double qualityOfPath) {
	this->qualityOfPath = qualityOfPath;
}

bool HeartbeatMessage::isRouteFlag() const {
	return routeFlag;
}

void HeartbeatMessage::setRouteFlag(bool routeFlag) {
	this->routeFlag = routeFlag;
}

