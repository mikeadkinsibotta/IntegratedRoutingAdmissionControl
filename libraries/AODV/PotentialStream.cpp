/*
 * PotentialStream.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include <PotentialStream.h>

PotentialStream::PotentialStream() {
	this->sourceAddress = XBeeAddress64();
	this->upStreamNeighbor = XBeeAddress64();
	this->increasedDataRate = 0;
	grantTimer = Timer();
	rejcTimer = Timer();
	onPath = false;
}

PotentialStream::PotentialStream(const XBeeAddress64& sourceAddress, const XBeeAddress64& upStreamNeighbor,
		const unsigned long grantTimeoutLength, const unsigned long rejcTimeoutLength, const float increasedDataRate) {
	this->sourceAddress = sourceAddress;
	this->upStreamNeighbor = upStreamNeighbor;
	this->increasedDataRate = increasedDataRate;
	grantTimer = Timer(grantTimeoutLength);
	rejcTimer = Timer(rejcTimeoutLength);
	onPath = false;

}

PotentialStream& PotentialStream::operator =(const PotentialStream &obj) {
	sourceAddress = obj.sourceAddress;
	upStreamNeighbor = obj.upStreamNeighbor;
	increasedDataRate = obj.increasedDataRate;
	grantTimer = obj.grantTimer;
	rejcTimer = obj.rejcTimer;
	onPath = obj.onPath;
	return *this;
}

Timer& PotentialStream::getGrantTimer() {
	return grantTimer;
}

const XBeeAddress64& PotentialStream::getSourceAddress() const {
	return sourceAddress;
}

const XBeeAddress64& PotentialStream::getUpStreamNeighbor() const {
	return upStreamNeighbor;
}

void PotentialStream::printPotentialStream() const {
	SerialUSB.print("Source Address: ");
	sourceAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(" Upstream Neighbor: ");
	upStreamNeighbor.printAddressASCII(&SerialUSB);
	SerialUSB.print(" OnPath ");
	SerialUSB.print(onPath);
	SerialUSB.print("  GrantTimerStatus: ");
	SerialUSB.print(grantTimer.isActive());
	SerialUSB.println();

}

void PotentialStream::increaseDataRate(const float increasedDataRate) {
	this->increasedDataRate += increasedDataRate;
}

Timer& PotentialStream::getRejcTimer() {
	return rejcTimer;
}

float PotentialStream::getIncreasedDataRate() const {
	return increasedDataRate;
}

bool PotentialStream::isOnPath() const {
	return onPath;
}

void PotentialStream::setOnPath(bool onPath) {
	this->onPath = onPath;
}

void PotentialStream::setGrantTimer(const Timer& grantTimer) {
	this->grantTimer = grantTimer;
}

void PotentialStream::setRejcTimer(const Timer& rejcTimer) {
	this->rejcTimer = rejcTimer;
}
