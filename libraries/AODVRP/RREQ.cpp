#include "RREQ.h"

RREQ::RREQ(const XBeeAddress64& sourceAddr, const uint32_t sourceSeqNum, const uint32_t broadcastId,
		const XBeeAddress64& destAddr, const uint32_t destSeqNum, const uint8_t hopCount) {
	this->destAddr = destAddr;
	this->broadcastId = broadcastId;
	this->sourceAddr = sourceAddr;
	this->hopCount = hopCount;
	this->destSeqNum = destSeqNum;
	this->sourceSeqNum = sourceSeqNum;

}

RREQ::RREQ(uint8_t * dataPtr) {

	sourceAddr.setMsb(
			(uint32_t(dataPtr[1]) << 24) + (uint32_t(dataPtr[2]) << 16) + (uint16_t(dataPtr[3]) << 8) + dataPtr[4]);

	sourceAddr.setLsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);
	sourceSeqNum = dataPtr[9];
	broadcastId = dataPtr[10];

	destAddr.setMsb(
			(uint32_t(dataPtr[11]) << 24) + (uint32_t(dataPtr[12]) << 16) + (uint16_t(dataPtr[13]) << 8) + dataPtr[14]);

	destAddr.setLsb(
			(uint32_t(dataPtr[15]) << 24) + (uint32_t(dataPtr[16]) << 16) + (uint16_t(dataPtr[17]) << 8) + dataPtr[18]);
	destSeqNum = dataPtr[19];
	hopCount = dataPtr[20];

}

uint32_t RREQ::getBroadcastId() const {
	return broadcastId;
}

void RREQ::setBroadcastId(uint32_t broadcastId) {
	this->broadcastId = broadcastId;
}

const XBeeAddress64& RREQ::getDestAddr() const {
	return destAddr;
}

void RREQ::setDestAddr(const XBeeAddress64& destAddr) {
	this->destAddr = destAddr;
}

uint8_t RREQ::getHopCount() const {
	return hopCount;
}

void RREQ::setHopCount(const uint8_t hopCount) {
	this->hopCount = hopCount;
}

uint32_t RREQ::getSourceSeqNum() const {
	return sourceSeqNum;
}

void RREQ::setSourceSeqNum(uint32_t sourceSeqNum) {
	this->sourceSeqNum = sourceSeqNum;
}

const XBeeAddress64& RREQ::getSourceAddr() const {
	return sourceAddr;
}

void RREQ::setSourceAddr(const XBeeAddress64& sourceAddr) {
	this->sourceAddr = sourceAddr;
}

uint32_t RREQ::getDestSeqNum() const {
	return destSeqNum;
}

void RREQ::setDestSeqNum(uint32_t destSeqNum) {
	this->destSeqNum = destSeqNum;
}

void RREQ::incrementHopCount() {
	hopCount++;
}

bool RREQ::compare(const RREQ &j) const {
	if(!sourceAddr.equals(j.sourceAddr))
		return false;
	else if(broadcastId != j.broadcastId)
		return false;
	return true;
}

void RREQ::print(Stream* _serial) const {
	_serial->print("RREQ");
	_serial->print('<');
	sourceAddr.printAddress(_serial);
	_serial->print(',');
	_serial->print(sourceSeqNum);
	_serial->print(',');
	_serial->print(broadcastId);
	_serial->print(',');
	destAddr.printAddress(_serial);
	_serial->print(',');
	_serial->print(destSeqNum);
	_serial->print(',');
	_serial->print(hopCount);
	_serial->print('>');

}
