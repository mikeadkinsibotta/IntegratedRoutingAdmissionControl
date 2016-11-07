#include "Neighbor.h"
const uint8_t MOVING_AVERAGE_SIZE = 5;
Neighbor::Neighbor() {

	address = XBeeAddress64();
	downStreamNeighbor = XBeeAddress64();
	dataRate = 0;
	timeStamp = 0;
	timeoutLength = 0;

}

Neighbor::Neighbor(const float dataRate, const XBeeAddress64& address, const XBeeAddress64& downStreamNeighbor,
		unsigned long timeoutLength) {

	this->address = address;
	this->downStreamNeighbor = downStreamNeighbor;
	this->dataRate = dataRate;
	timeStamp = millis();
	this->timeoutLength = timeoutLength;
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

const XBeeAddress64& Neighbor::getDownStreamNeighbor() const {
	return downStreamNeighbor;
}

void Neighbor::setDownStreamNeighbor(const XBeeAddress64& downStreamNeighbor) {
	this->downStreamNeighbor = downStreamNeighbor;
}

unsigned long Neighbor::getTimeStamp() const {
	return timeStamp;
}

bool Neighbor::timerExpired() {
	if (millis() - timeStamp > timeoutLength) {
		return true;
	}
	return false;
}

void Neighbor::updateTimeStamp() {
	timeStamp = millis();
}

void Neighbor::printNeighbor() const {
	SerialUSB.print('<');
	address.printAddressASCII(&SerialUSB);
	SerialUSB.print(", ");
	SerialUSB.print(dataRate);
	SerialUSB.print(", ");
	SerialUSB.print("DownStreamNeighbor:  ");
	downStreamNeighbor.printAddressASCII(&SerialUSB);
	SerialUSB.println('>');
}
