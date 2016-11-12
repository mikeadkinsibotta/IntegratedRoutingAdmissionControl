/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

HeartbeatMessage::HeartbeatMessage() {
	senderAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	myNextHop = XBeeAddress64();
	relativeDistance = 0.0;
	seqNum = 0;
	dataRate = 0.0;
	qualityOfPath = 0;
	neighborhoodCapacity = 0.0;
	routeFlag = false;
	rssi = 0;
	hopsToSink = 0;
	generateData = false;
	is_sink = false;
}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& sinkAddress,
		const XBeeAddress64& myNextHop, const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath,
		const float neighborhoodCapacity, const bool routeFlag, const uint8_t hopsToSink, const bool generateData,
		const bool is_sink) {

	this->senderAddress = senderAddress;
	this->sinkAddress = sinkAddress;
	this->myNextHop = myNextHop;
	this->relativeDistance = 0.0;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;
	this->hopsToSink = hopsToSink;
	this->generateData = generateData;
	this->is_sink = is_sink;
	rssi = 0;

}

void HeartbeatMessage::printMessage() {

	SerialUSB.print("<SenderAddress: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", MyNextHop: ");
	myNextHop.printAddressASCII(&SerialUSB);
	SerialUSB.print(", seqNum: ");
	SerialUSB.print(seqNum);
	SerialUSB.print(", Relative Distance: ");
	SerialUSB.print(relativeDistance, 12);
	SerialUSB.print(", DataRate: ");
	SerialUSB.print(dataRate);
	SerialUSB.print(", NeighborhoodCapacity: ");
	SerialUSB.print(neighborhoodCapacity);
	SerialUSB.print(", QualityOfPath: ");
	SerialUSB.print(qualityOfPath);
	SerialUSB.print(", HopsToSink: ");
	SerialUSB.print(hopsToSink);
	SerialUSB.print(", RSSI: ");
	SerialUSB.print(rssi);
	SerialUSB.print(", GenerateData: ");
	SerialUSB.print(generateData);
	SerialUSB.print(", RouteFlag: ");
	SerialUSB.print(routeFlag);
	SerialUSB.println('>');

	SerialUSB.println();
	SerialUSB.println();
}

void HeartbeatMessage::generateBeatMessage(uint8_t payload[]) {

	const uint8_t * dataRateP = reinterpret_cast<uint8_t*>(&dataRate);
	const uint8_t * neighborhoodCapacityP = reinterpret_cast<uint8_t*>(&neighborhoodCapacity);

	payload[0] = 'B';
	payload[1] = 'E';
	payload[2] = 'A';
	payload[3] = 'T';
	payload[4] = '\0';
	payload[21] = seqNum;
	payload[22] = qualityOfPath;
	payload[23] = routeFlag;
	payload[24] = hopsToSink;
	payload[25] = generateData;
	payload[26] = is_sink;

	addAddressToMessage(payload, sinkAddress, 5);
	addAddressToMessage(payload, myNextHop, 13);
	addFloat(payload, dataRateP, 27);
	addFloat(payload, neighborhoodCapacityP, 31);

}

void HeartbeatMessage::addFloat(uint8_t payload[], const uint8_t * num, const uint8_t i) {
	payload[i] = num[0];
	payload[i + 1] = num[1];
	payload[i + 2] = num[2];
	payload[i + 3] = num[3];

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

void HeartbeatMessage::setAddress(const uint8_t * dataPtr, XBeeAddress64& address, const uint8_t i) {

	address.setMsb(
			(uint32_t(dataPtr[i]) << 24) + (uint32_t(dataPtr[i + 1]) << 16) + (uint16_t(dataPtr[i + 2]) << 8)
					+ dataPtr[i + 3]);

	address.setLsb(
			(uint32_t(dataPtr[i + 4]) << 24) + (uint32_t(dataPtr[i + 5]) << 16) + (uint16_t(dataPtr[i + 6]) << 8)
					+ dataPtr[i + 7]);

}

void HeartbeatMessage::transcribeHeartbeatPacket(const Rx64Response& response) {

	const uint8_t* dataPtr = response.getData() + 5;

	senderAddress = response.getRemoteAddress64();

	relativeDistance = response.getRelativeDistance();
	rssi = response.getRssi() * -1.0;

	setAddress(dataPtr, sinkAddress, 0);
	setAddress(dataPtr, myNextHop, 8);

	seqNum = dataPtr[16];
	qualityOfPath = dataPtr[17];
	routeFlag = dataPtr[18];
	hopsToSink = dataPtr[19];
	generateData = dataPtr[20];
	is_sink = dataPtr[21];

	memcpy(&dataRate, dataPtr + 22, sizeof(float));
	memcpy(&neighborhoodCapacity, dataPtr + 26, sizeof(float));

}

const XBeeAddress64& HeartbeatMessage::getSenderAddress() const {
	return senderAddress;
}

const XBeeAddress64& HeartbeatMessage::getSinkAddress() const {
	return sinkAddress;
}

uint8_t HeartbeatMessage::getSeqNum() const {
	return seqNum;
}

float HeartbeatMessage::getDataRate() const {
	return dataRate;
}

float HeartbeatMessage::getNeighborhoodCapacity() const {
	return neighborhoodCapacity;
}

uint8_t HeartbeatMessage::getQualityOfPath() const {
	return qualityOfPath;
}

bool HeartbeatMessage::isRouteFlag() const {
	return routeFlag;
}

double HeartbeatMessage::getRelativeDistance() const {
	return relativeDistance;
}

double HeartbeatMessage::getRssi() const {
	return rssi;
}

const XBeeAddress64& HeartbeatMessage::getMyNextHop() const {
	return myNextHop;
}

uint8_t HeartbeatMessage::getHopsToSink() const {
	return hopsToSink;
}

bool HeartbeatMessage::isGenerateData() const {
	return generateData;
}
