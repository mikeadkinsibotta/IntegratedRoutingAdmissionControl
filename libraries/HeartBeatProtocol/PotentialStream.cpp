/*
 * PotentialStream.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include <PotentialStream.h>

PotentialStream::PotentialStream(XBeeAddress64 sourceAddress, XBeeAddress64 upStreamNeighbor) {
	this->sourceAddress = sourceAddress;
	this->upStreamNeighbor = upStreamNeighbor;
}

Timer& PotentialStream::getGrantTimer() {
	return grantTimer;
}

const XBeeAddress64& PotentialStream::getSourceAddress() const {
	return sourceAddress;
}

void PotentialStream::setSourceAddress(const XBeeAddress64& sourceAddress) {
	this->sourceAddress = sourceAddress;
}

const XBeeAddress64& PotentialStream::getUpStreamNeighbor() const {
	return upStreamNeighbor;
}

void PotentialStream::setUpStreamNeighbor(const XBeeAddress64& upStreamNeighbor) {
	this->upStreamNeighbor = upStreamNeighbor;
}

