#include "Neighbor.h"

const XBeeAddress64& Neighbor::getNeighborAddress() const {
	return neighborAddress;
}

void Neighbor::setNeighborAddress(const XBeeAddress64& neighborAddress) {
	this->neighborAddress = neighborAddress;
}

float Neighbor::getNeighborDataRate() const {
	return neighborDataRate;
}

void Neighbor::setNeighborDataRate(float neighborDataRate) {
	this->neighborDataRate = neighborDataRate;
}

const XBeeAddress64& Neighbor::getStreamSourceAddress() const {
	return streamSourceAddress;
}

void Neighbor::setStreamSourceAddress(const XBeeAddress64& streamSourceAddress) {
	this->streamSourceAddress = streamSourceAddress;
}

const XBeeAddress64& Neighbor::getSinkAddress() const {
	return sinkAddress;
}

void Neighbor::setSinkAddress(const XBeeAddress64& sinkAddress) {
	this->sinkAddress = sinkAddress;
}

float Neighbor::getRelativeDistance() const {
	return relativeDistance;
}

void Neighbor::setRelativeDistance(float relativeDistance) {
	this->relativeDistance = relativeDistance;
}

float Neighbor::getNeighborhoodCapacity() const {
	return neighborhoodCapacity;
}

void Neighbor::setNeighborhoodCapacity(float neighborhoodCapacity) {
	this->neighborhoodCapacity = neighborhoodCapacity;
}

float Neighbor::getPacketLoss() const {
	return packetLoss;
}

void Neighbor::setPacketLoss(float packetLoss) {
	this->packetLoss = packetLoss;
}

uint8_t Neighbor::getSeqNum() const {
	return seqNum;
}

void Neighbor::setSeqNum(uint8_t seqNum) {
	this->seqNum = seqNum;
}

uint8_t Neighbor::getQosCost() const {
	return qosCost;
}

void Neighbor::setQosCost(uint8_t qosCost) {
	this->qosCost = qosCost;
}

bool Neighbor::isRouteFlag() const {
	return routeFlag;
}

void Neighbor::setRouteFlag(bool routeFlag) {
	this->routeFlag = routeFlag;
}

bool Neighbor::compare(const Neighbor &b) {
	return neighborAddress.equals(b.neighborAddress);

}

void Neighbor::printNeighbor() const {
	Serial.print('<');
	neighborAddress.printAddress(&Serial);
	Serial.print(',');
	Serial.print(neighborDataRate);
	Serial.print(',');
	Serial.print(neighborhoodCapacity);
	Serial.print('>');

}
