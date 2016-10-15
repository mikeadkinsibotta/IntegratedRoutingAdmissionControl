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
}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& sinkAddress,
		const XBeeAddress64& myNextHop, const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath,
		const float neighborhoodCapacity, const bool routeFlag, const uint8_t hopsToSink, const bool generateData) {

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

	const uint8_t * dataRateP = (uint8_t *) &dataRate;
	const uint8_t * neighborhoodCapacityP = (uint8_t *) &neighborhoodCapacity;

	payload[0] = 'B';
	payload[1] = 'E';
	payload[2] = 'A';
	payload[3] = 'T';
	payload[4] = '\0';
	payload[5] = (sinkAddress.getMsb() >> 24) & 0xff;
	payload[6] = (sinkAddress.getMsb() >> 16) & 0xff;
	payload[7] = (sinkAddress.getMsb() >> 8) & 0xff;
	payload[8] = sinkAddress.getMsb() & 0xff;
	payload[9] = (sinkAddress.getLsb() >> 24) & 0xff;
	payload[10] = (sinkAddress.getLsb() >> 16) & 0xff;
	payload[11] = (sinkAddress.getLsb() >> 8) & 0xff;
	payload[12] = sinkAddress.getLsb() & 0xff;
	payload[13] = (myNextHop.getMsb() >> 24) & 0xff;
	payload[14] = (myNextHop.getMsb() >> 16) & 0xff;
	payload[15] = (myNextHop.getMsb() >> 8) & 0xff;
	payload[16] = myNextHop.getMsb() & 0xff;
	payload[17] = (myNextHop.getLsb() >> 24) & 0xff;
	payload[18] = (myNextHop.getLsb() >> 16) & 0xff;
	payload[19] = (myNextHop.getLsb() >> 8) & 0xff;
	payload[20] = myNextHop.getLsb() & 0xff;
	payload[21] = seqNum;
	payload[22] = qualityOfPath;
	payload[23] = routeFlag;
	payload[24] = hopsToSink;
	payload[25] = generateData;
	payload[26] = dataRateP[0];
	payload[27] = dataRateP[1];
	payload[28] = dataRateP[2];
	payload[29] = dataRateP[3];
	payload[30] = neighborhoodCapacityP[0];
	payload[31] = neighborhoodCapacityP[1];
	payload[32] = neighborhoodCapacityP[2];
	payload[33] = neighborhoodCapacityP[3];
}

void HeartbeatMessage::transcribeHeartbeatPacket(const Rx64Response& response) {

	const uint8_t* dataPtr = response.getData() + 5;

	senderAddress = response.getRemoteAddress64();

	relativeDistance = response.getRelativeDistance();
	rssi = response.getRssi() * -1.0;

	sinkAddress.setMsb(
			(uint32_t(dataPtr[0]) << 24) + (uint32_t(dataPtr[1]) << 16) + (uint16_t(dataPtr[2]) << 8) + dataPtr[3]);

	sinkAddress.setLsb(
			(uint32_t(dataPtr[4]) << 24) + (uint32_t(dataPtr[5]) << 16) + (uint16_t(dataPtr[6]) << 8) + dataPtr[7]);

	myNextHop.setMsb(
			(uint32_t(dataPtr[8]) << 24) + (uint32_t(dataPtr[9]) << 16) + (uint16_t(dataPtr[10]) << 8) + dataPtr[11]);

	myNextHop.setLsb(
			(uint32_t(dataPtr[12]) << 24) + (uint32_t(dataPtr[13]) << 16) + (uint16_t(dataPtr[14]) << 8) + dataPtr[15]);

	seqNum = dataPtr[16];
	qualityOfPath = dataPtr[17];
	routeFlag = dataPtr[18];
	hopsToSink = dataPtr[19];
	generateData = dataPtr[20];

	float * dataRateP = (float*) (dataPtr + 21);
	dataRate = *dataRateP;

	dataRateP = (float*) (dataPtr + 25);
	neighborhoodCapacity = *dataRateP;

}

const XBeeAddress64& HeartbeatMessage::getSenderAddress() const {
	return senderAddress;
}

void HeartbeatMessage::setSenderAddress(const XBeeAddress64& senderAddress) {
	this->senderAddress = senderAddress;
}

const XBeeAddress64& HeartbeatMessage::getSinkAddress() const {
	return sinkAddress;
}

void HeartbeatMessage::setSinkAddress(const XBeeAddress64& sinkAddress) {
	this->sinkAddress = sinkAddress;
}

uint8_t HeartbeatMessage::getSeqNum() const {
	return seqNum;
}

void HeartbeatMessage::setSeqNum(uint8_t seqNum) {
	this->seqNum = seqNum;
}

float HeartbeatMessage::getDataRate() const {
	return dataRate;
}

void HeartbeatMessage::setDataRate(float dataRate) {
	this->dataRate = dataRate;
}

float HeartbeatMessage::getNeighborhoodCapacity() const {
	return neighborhoodCapacity;
}

void HeartbeatMessage::setNeighborhoodCapacity(float neighborhoodCapacity) {
	this->neighborhoodCapacity = neighborhoodCapacity;
}

uint8_t HeartbeatMessage::getQualityOfPath() const {
	return qualityOfPath;
}

void HeartbeatMessage::setQualityOfPath(uint8_t qualityOfPath) {
	this->qualityOfPath = qualityOfPath;
}

bool HeartbeatMessage::isRouteFlag() const {
	return routeFlag;
}

void HeartbeatMessage::setRouteFlag(bool routeFlag) {
	this->routeFlag = routeFlag;
}

double HeartbeatMessage::getRelativeDistance() const {
	return relativeDistance;
}

void HeartbeatMessage::setRelativeDistance(double relativeDistance) {
	this->relativeDistance = relativeDistance;
}

double HeartbeatMessage::getRssi() const {
	return rssi;
}

void HeartbeatMessage::setRssi(double rssi) {
	this->rssi = rssi;
}

const XBeeAddress64& HeartbeatMessage::getMyNextHop() const {
	return myNextHop;
}

void HeartbeatMessage::setMyNextHop(const XBeeAddress64& myNextHop) {
	this->myNextHop = myNextHop;
}

uint8_t HeartbeatMessage::getHopsToSink() const {
	return hopsToSink;
}

void HeartbeatMessage::setHopsToSink(uint8_t hopsToSink) {
	this->hopsToSink = hopsToSink;
}

bool HeartbeatMessage::isGenerateData() const {
	return generateData;
}
