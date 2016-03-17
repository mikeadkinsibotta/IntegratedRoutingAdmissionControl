#include "RoutingTableEntry.h"

RoutingTableEntry::RoutingTableEntry() {
	this->destinationAddress = XBeeAddress64();
	this->nextHop = XBeeAddress64();
	this->hopCount = 0;
	this->seqNumDest = 0;
	this->experiationTime = 0;
	this->active = false;
}

RoutingTableEntry::RoutingTableEntry(const XBeeAddress64& destinationAddress, const XBeeAddress64& nextHop,
		uint8_t hopCount, uint32_t seqNum, unsigned long experiationTime) {
	this->destinationAddress = destinationAddress;
	this->nextHop = nextHop;
	this->hopCount = hopCount;
	this->seqNumDest = seqNumDest;
	this->experiationTime = experiationTime;
	this->active = true;
}

const XBeeAddress64& RoutingTableEntry::getDestinationAddress() const {
	return destinationAddress;
}

void RoutingTableEntry::setDestinationAddress(const XBeeAddress64& destinationAddress) {
	this->destinationAddress = destinationAddress;
}

unsigned long RoutingTableEntry::getExperiationTime() const {
	return experiationTime;
}

void RoutingTableEntry::setExperiationTime(const unsigned long experiationTime) {
	this->experiationTime = experiationTime;
}

uint8_t RoutingTableEntry::getHopCount() const {
	return hopCount;
}

void RoutingTableEntry::setHopCount(uint8_t hopCount) {
	this->hopCount = hopCount;
}

const XBeeAddress64& RoutingTableEntry::getNextHop() const {
	return nextHop;
}

void RoutingTableEntry::setNextHop(const XBeeAddress64& nextHop) {
	this->nextHop = nextHop;
}

uint32_t RoutingTableEntry::getSeqNumDest() const {
	return seqNumDest;
}

void RoutingTableEntry::setSeqNumDest(uint32_t seqNumDest) {
	this->seqNumDest = seqNumDest;
}

bool RoutingTableEntry::isActive() const {
	return active;
}

vector<XBeeAddress64>& RoutingTableEntry::getActiveNeighbors() {
	return activeNeighbors;
}

void RoutingTableEntry::setActiveNeighbors(const vector<XBeeAddress64>& activeNeighbors) {
	this->activeNeighbors = activeNeighbors;
}

void RoutingTableEntry::setActive(bool active) {
	this->active = active;
}

void RoutingTableEntry::printRoute() {
	SerialUSB.print('<');
	destinationAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(',');
	nextHop.printAddressASCII(&SerialUSB);
	SerialUSB.print(',');
	SerialUSB.print(hopCount);
	SerialUSB.print(',');
	SerialUSB.print(seqNumDest);
	SerialUSB.print(',');
	SerialUSB.print(experiationTime);
	SerialUSB.print('>');

}

