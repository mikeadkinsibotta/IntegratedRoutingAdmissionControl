/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

HeartbeatMessage::HeartbeatMessage() {
	dataRate = 0.0;
	downStreamNeighbor = XBeeAddress64();
	senderAddress = XBeeAddress64();

}

HeartbeatMessage::HeartbeatMessage(const float dataRate, const XBeeAddress64& downStreamNeighbor) {
	this->dataRate = dataRate;
	this->downStreamNeighbor = downStreamNeighbor;
	senderAddress = XBeeAddress64();

}

void HeartbeatMessage::generateBeatMessage(uint8_t payload[]) {

	const uint8_t * dataRateP = reinterpret_cast<uint8_t*>(&dataRate);

	payload[0] = 'B';
	payload[1] = 'E';
	payload[2] = 'A';
	payload[3] = 'T';
	payload[4] = '\0';
	addFloat(payload, dataRateP, 5);
	addAddressToMessage(payload, downStreamNeighbor, 9);

}

void HeartbeatMessage::transcribeHeartbeatPacket(const Rx64Response& response) {

	const uint8_t* dataPtr = response.getData();

	senderAddress = response.getRemoteAddress64();
	memcpy(&dataRate, dataPtr + 5, sizeof(float));
	setAddress(dataPtr, downStreamNeighbor, 9);

}

void HeartbeatMessage::addAddressToMessage(uint8_t payload[], const XBeeAddress64& address, const uint8_t i) {

	payload[i] = (address.getMsb() >> 24) & 0xff;
	payload[i + 1] = (address.getMsb() >> 16) & 0xff;
	payload[i + 2] = (address.getMsb() >> 8) & 0xff;
	payload[i + 3] = address.getMsb() & 0xff;
	payload[i + 4] = (address.getLsb() >> 24) & 0xff;
	payload[i + 5] = (address.getLsb() >> 16) & 0xff;
	payload[i + 6] = (address.getLsb() >> 8) & 0xff;
	payload[i + 7] = address.getLsb() & 0xff;

}

void HeartbeatMessage::addFloat(uint8_t payload[], const uint8_t * num, const uint8_t i) {
	payload[i] = num[0];
	payload[i + 1] = num[1];
	payload[i + 2] = num[2];
	payload[i + 3] = num[3];

}

void HeartbeatMessage::setAddress(const uint8_t * dataPtr, XBeeAddress64& address, const uint8_t i) {

	address.setMsb(
			(uint32_t(dataPtr[i]) << 24) + (uint32_t(dataPtr[i + 1]) << 16) + (uint16_t(dataPtr[i + 2]) << 8)
					+ dataPtr[i + 3]);

	address.setLsb(
			(uint32_t(dataPtr[i + 4]) << 24) + (uint32_t(dataPtr[i + 5]) << 16) + (uint16_t(dataPtr[i + 6]) << 8)
					+ dataPtr[i + 7]);

}

const XBeeAddress64& HeartbeatMessage::getSenderAddress() const {
	return senderAddress;
}

const XBeeAddress64& HeartbeatMessage::getDownStreamNeighbor() const {
	return downStreamNeighbor;
}

void HeartbeatMessage::setDownStreamNeighbor(const XBeeAddress64& downStreamNeighbor) {
	this->downStreamNeighbor = downStreamNeighbor;
}

void HeartbeatMessage::print() {

	SerialUSB.print("<SenderAddress: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", DataRate: ");
	SerialUSB.print(dataRate);
	SerialUSB.println('>');
}

float HeartbeatMessage::getDataRate() const {
	return dataRate;
}
