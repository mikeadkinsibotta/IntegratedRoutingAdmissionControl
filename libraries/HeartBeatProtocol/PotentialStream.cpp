/*
 * PotentialStream.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include <PotentialStream.h>

PotentialStream::PotentialStream(XBeeAddress64 sourceAddress, XBeeAddress64 upStreamNeighbor,
		unsigned long grantTimeoutLength) {
	this->sourceAddress = sourceAddress;
	this->upStreamNeighbor = upStreamNeighbor;
	grantTimer = Timer(grantTimeoutLength);
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
	SerialUSB.println();

}
