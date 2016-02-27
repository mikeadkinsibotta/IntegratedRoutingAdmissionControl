/*
 * TraceMessage.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: mike
 */

#include "TraceMessage.h"

TraceMessage::TraceMessage() {
	addressListLength = 0;
}

void TraceMessage::transcribeMessage(const Rx64Response& response) {

	const uint8_t* dataPtr = response.getData() + 5;

	XBeeAddress64 senderAddress = response.getRemoteAddress64();

	addressListLength = *dataPtr;

	//Move to beginning of addresses
	dataPtr++;

	for (int i = 0; i < addressListLength; i++) {
		XBeeAddress64 address;
		address.setMsb(
				(uint32_t(dataPtr[0]) << 24) + (uint32_t(dataPtr[1]) << 16) + (uint16_t(dataPtr[2]) << 8) + dataPtr[3]);

		address.setLsb(
				(uint32_t(dataPtr[4]) << 24) + (uint32_t(dataPtr[5]) << 16) + (uint16_t(dataPtr[6]) << 8) + dataPtr[7]);

		addresses.push_back(address);

		//Move to next address
		dataPtr += 8;

	}

	//Add last sender
	addAddress(senderAddress);

}

void TraceMessage::addAddress(const XBeeAddress64 address) {
	addresses.push_back(address);
	SerialUSB.print("Pushing Back Address:  ");
	address.printAddressASCII(&SerialUSB);
	SerialUSB.println();
	addressListLength = addresses.size();
	SerialUSB.print("Address Length: ");
	SerialUSB.println(addressListLength);
}

const vector<XBeeAddress64>& TraceMessage::getAddresses() const {
	return addresses;
}

void TraceMessage::setAddresses(const vector<XBeeAddress64>& addresses) {
	this->addresses = addresses;
}

uint8_t TraceMessage::getAddressListLength() const {
	return addressListLength;
}

void TraceMessage::setAddressListLength(uint8_t addressListLength) {
	this->addressListLength = addressListLength;
}

void TraceMessage::printTraceMessage() {
	SerialUSB.print("Current Voice Path - Sender Address:  ");
	addresses.at(0).printAddressASCII(&SerialUSB);
	SerialUSB.print("  Path:  ");
	for (int i = 0; i < addressListLength; ++i) {
		addresses.at(i).printAddressASCII(&SerialUSB);
		SerialUSB.print(",  ");
	}
	SerialUSB.println();

}
