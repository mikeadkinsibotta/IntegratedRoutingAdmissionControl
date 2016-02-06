#include "Neighbor.h"
const uint8_t MOVING_AVERAGE_SIZE = 5;
Neighbor::Neighbor() {

	address = XBeeAddress64();
	dataRate = 0;
	seqNum = 0;
	qualityOfPath = 0;
	neighborhoodCapacity = 0;
	routeFlag = false;
	sinkAddress = XBeeAddress64();
	relativeDistance = 0;
	packetLoss = 0;
	relativeDistanceAvg = 0;
	rssi = 0;
	previousTimeStamp = 0;
	timeStamp = 0;
	timeoutLength = 0;
	previousRelativeDistance = 0;

}

Neighbor::Neighbor(const XBeeAddress64& address, float neighborDataRate, uint8_t seqNum, float qualityOfPath,
		float neighborhoodCapacity, bool routeFlag, const XBeeAddress64& sinkAddress, double relativeDistance,
		double rssi, unsigned long timeoutLength) {

	this->address = address;
	this->dataRate = dataRate;
	this->seqNum = seqNum;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;
	this->sinkAddress = sinkAddress;
	this->rssi = rssi;
	this->packetLoss = 0;
	addToRelativeDistance(relativeDistance);
	timeStamp = millis();
	previousTimeStamp = timeStamp;
	this->timeoutLength = timeoutLength;
	previousRelativeDistance = 0;
	relativeDistanceAvg = 0;
}

void Neighbor::addToRelativeDistance(const double relativeDistance) {
	if (relativeDistanceQueue.size() < MOVING_AVERAGE_SIZE) {
		relativeDistanceQueue.push_front(relativeDistance);
	} else {
		previousRelativeDistance = relativeDistanceQueue.back();
		relativeDistanceQueue.pop_back();

		double total = 0;
		for (int i = 0; i < MOVING_AVERAGE_SIZE; ++i) {
			total += relativeDistanceQueue[i];
		}

		relativeDistanceAvg = total / MOVING_AVERAGE_SIZE;

	}

}

const XBeeAddress64& Neighbor::getAddress() const {
	return address;
}

void Neighbor::setAddress(const XBeeAddress64& address) {
	this->address = address;
}

float Neighbor::getDataRate() const {
	return dataRate;
}

void Neighbor::setDataRate(float dataRate) {
	this->dataRate = dataRate;
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

uint8_t Neighbor::getQualityOfPath() const {
	return qualityOfPath;
}

void Neighbor::setQualityOfPath(uint8_t qualityOfPath) {
	this->qualityOfPath = qualityOfPath;
}

bool Neighbor::isRouteFlag() const {
	return routeFlag;
}

void Neighbor::setRouteFlag(bool routeFlag) {
	this->routeFlag = routeFlag;
}

uint8_t Neighbor::getSeqNum() const {
	return seqNum;
}

void Neighbor::setSeqNum(uint8_t seqNum) {
	this->seqNum = seqNum;
}

const XBeeAddress64& Neighbor::getSinkAddress() const {
	return sinkAddress;
}

void Neighbor::setSinkAddress(const XBeeAddress64& sinkAddress) {
	this->sinkAddress = sinkAddress;
}

void Neighbor::updateTimeStamp() {
	previousTimeStamp = timeStamp;
	timeStamp = millis();
}

bool Neighbor::timerExpired() {
	if (millis() - timeStamp > timeoutLength) {
		return true;
	}
	return false;
}

bool Neighbor::compare(const Neighbor &b) {
	return address.equals(b.address);
}

unsigned long Neighbor::getPreviousTimeStamp() const {
	return previousTimeStamp;
}

unsigned long Neighbor::getTimeStamp() const {
	return timeStamp;
}

bool Neighbor::equals(const Neighbor& neighbor) const {
	return address.equals(neighbor.getAddress());
}

double Neighbor::getPreviousRelativeDistance() const {
	return previousRelativeDistance;
}

double Neighbor::getRelativeDistanceAvg() const {
	return relativeDistanceAvg;
}

double Neighbor::getRssi() const {
	return rssi;
}

void Neighbor::setRssi(double rssi) {
	this->rssi = rssi;
}

void Neighbor::printNeighbor() const {
	Serial.print('<');
	address.printAddress(&Serial);
	Serial.print(',');
	Serial.print(dataRate);
	Serial.print(',');
	Serial.print(neighborhoodCapacity);
	Serial.print('>');
}
