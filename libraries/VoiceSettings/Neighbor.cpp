#include "Neighbor.h"

Neighbor::Neighbor() {
	dataRate = 0;
	contentionDomainRate = 0;
	contentionDomainSize = 0;
}
Neighbor::Neighbor(const XBeeAddress64 &address, float dataRate) {
	this->address = address;
	this->dataRate = dataRate;
	contentionDomainRate = 0;
	contentionDomainSize = 0;
}

Neighbor::Neighbor(const XBeeAddress64 &address, const float dataRate, const float contentionDomainRate,
		const uint8_t contentionDomainSize) {
	this->address = address;
	this->dataRate = dataRate;
	this->contentionDomainRate = contentionDomainRate;
	this->contentionDomainSize = contentionDomainSize;
}

XBeeAddress64 Neighbor::getAddress() const {
	return address;
}

void Neighbor::setAddress(const XBeeAddress64 &address) {
	this->address = address;
}

float Neighbor::getDataRate() const {
	return dataRate;
}

void Neighbor::setDataRate(float dataRate) {
	this->dataRate = dataRate;
}

float Neighbor::getContentionDomainRate() const {
	return contentionDomainRate;
}

void Neighbor::setContentionDomainRate(float contentionDomainRate) {
	this->contentionDomainRate = contentionDomainRate;
}

uint8_t Neighbor::getContentionDomainSize() const {
	return contentionDomainSize;
}

void Neighbor::setContentionDomainSize(uint8_t contentionDomainSize) {
	this->contentionDomainSize = contentionDomainSize;
}

bool Neighbor::compare(const Neighbor &b) {
	return address.equals(b.address);

}

void Neighbor::printNeighbor() const {
	Serial.print('<');
	address.printAddress(&Serial);
	Serial.print(',');
	Serial.print(dataRate);
	Serial.print(',');
	Serial.print(contentionDomainRate);
	Serial.print(',');
	Serial.print(contentionDomainSize);
	Serial.print('>');

}
