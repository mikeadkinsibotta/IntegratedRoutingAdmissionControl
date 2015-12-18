#include "Neighbor.h"

const XBeeAddress64& Neighbor::getNeighborAddress() const {
	return neighborAddress;
}

void Neighbor::setNeighborAddress(const XBeeAddress64& neighborAddress) {
	this->neighborAddress = neighborAddress;
}

double Neighbor::getNeighborDataRate() const {
	return neighborDataRate;
}

void Neighbor::setNeighborDataRate(double neighborDataRate) {
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

double Neighbor::getRelativeDistance() const {
	return relativeDistance;
}

void Neighbor::setRelativeDistance(double relativeDistance) {
	this->relativeDistance = relativeDistance;
}

double Neighbor::getNeighborhoodCapacity() const {
	return neighborhoodCapacity;
}

void Neighbor::setNeighborhoodCapacity(double neighborhoodCapacity) {
	this->neighborhoodCapacity = neighborhoodCapacity;
}

double Neighbor::getPacketLoss() const {
	return packetLoss;
}

void Neighbor::setPacketLoss(double packetLoss) {
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
