#include "RREP.h"

RREP::RREP() {
	this->sourceAddr = XBeeAddress64();
	this->destAddr = XBeeAddress64();
	this->destSeqNum = 0;
	this->hopCount = 0;
	this->lifeTime = 0;
}
;

RREP::RREP(XBeeAddress64 sourceAddr, XBeeAddress64 destAddr, uint32_t destSeqNum, uint8_t hopCount, uint8_t lifeTime) {
	this->sourceAddr = sourceAddr;
	this->destAddr = destAddr;
	this->destSeqNum = destSeqNum;
	this->hopCount = hopCount;
	this->lifeTime = lifeTime;
}

RREP::RREP(uint8_t* dataPtr) {
	sourceAddr.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	sourceAddr.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	destAddr.setMsb(
			(uint32_t(dataPtr[13]) << 24) + (uint32_t(dataPtr[14]) << 16) + (uint16_t(dataPtr[15]) << 8) + dataPtr[16]);

	destAddr.setLsb(
			(uint32_t(dataPtr[17]) << 24) + (uint32_t(dataPtr[18]) << 16) + (uint16_t(dataPtr[19]) << 8) + dataPtr[20]);

	destSeqNum = dataPtr[21];
	hopCount = dataPtr[22];
	lifeTime = dataPtr[23];

}

const XBeeAddress64& RREP::getDestAddr() const {
	return destAddr;
}

void RREP::setDestAddr(const XBeeAddress64& destAddr) {
	this->destAddr = destAddr;
}

uint32_t RREP::getDestSeqNum() const {
	return destSeqNum;
}

void RREP::setDestSeqNum(uint32_t destSeqNum) {
	this->destSeqNum = destSeqNum;
}

uint8_t RREP::getHopCount() const {
	return hopCount;
}

void RREP::setHopCount(uint8_t hopCount) {
	this->hopCount = hopCount;
}

uint8_t RREP::getLifeTime() const {
	return lifeTime;
}

void RREP::setLifeTime(uint8_t lifeTime) {
	this->lifeTime = lifeTime;
}

const XBeeAddress64& RREP::getSourceAddr() const {
	return sourceAddr;
}

void RREP::setSourceAddr(const XBeeAddress64& sourceAddr) {
	this->sourceAddr = sourceAddr;
}

void RREP::incrementHopCount() {
	hopCount++;
}

void RREP::print(Stream* _serial) const {
	_serial->print("RREP");
	_serial->print('<');
	sourceAddr.printAddress(_serial);
	_serial->print(',');
	destAddr.printAddress(_serial);
	_serial->print(',');
	_serial->print(destSeqNum);
	_serial->print(',');
	_serial->print(hopCount);
	_serial->print(',');
	_serial->print(lifeTime);
	_serial->print('>');

}

