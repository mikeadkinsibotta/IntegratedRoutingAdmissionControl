/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

const double MILLIWATTS = 0.000001258925;
const double DISTANCE = 1;
const double N_P = 3.16557667218;

HeartbeatMessage::HeartbeatMessage() {
	senderAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	relativeDistance = 0.0;
	seqNum = 0;
	dataRate = 0.0;
	qualityOfPath = 0;
	neighborhoodCapacity = 0.0;
	routeFlag = false;
	rssi = 0;
}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& sinkAddress, const uint8_t seqNum, const float dataRate,
		const uint8_t qualityOfPath, const float neighborhoodCapacity, const bool routeFlag) {

	this->senderAddress = XBeeAddress64();
	this->sinkAddress = sinkAddress;
	this->relativeDistance = 0.0;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;
	rssi = 0;

}

void HeartbeatMessage::printMessage() {

	SerialUSB.print("<SenderAddress: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
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
	SerialUSB.print(", RSSI: ");
	SerialUSB.print(rssi);
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
	payload[13] = seqNum;
	payload[14] = qualityOfPath;
	payload[15] = routeFlag;
	payload[16] = dataRateP[0];
	payload[17] = dataRateP[1];
	payload[18] = dataRateP[2];
	payload[19] = dataRateP[3];
	payload[20] = neighborhoodCapacityP[0];
	payload[21] = neighborhoodCapacityP[1];
	payload[22] = neighborhoodCapacityP[2];
	payload[23] = neighborhoodCapacityP[3];
}

void HeartbeatMessage::transcribeHeartbeatPacket(const Rx64Response& response) {

	const uint8_t* dataPtr = response.getData() + 5;

	senderAddress = response.getRemoteAddress64();

	rssi = response.getRssi() * -1;
	const double milliWatts = pow(10.0, (rssi / 10.0));
	relativeDistance = DISTANCE * pow((milliWatts / MILLIWATTS), (-1.0 / N_P));

	sinkAddress.setMsb(
			(uint32_t(dataPtr[0]) << 24) + (uint32_t(dataPtr[1]) << 16) + (uint16_t(dataPtr[2]) << 8) + dataPtr[3]);

	sinkAddress.setLsb(
			(uint32_t(dataPtr[4]) << 24) + (uint32_t(dataPtr[5]) << 16) + (uint16_t(dataPtr[6]) << 8) + dataPtr[7]);

	seqNum = dataPtr[8];
	qualityOfPath = dataPtr[9];
	routeFlag = dataPtr[10];

	float * dataRateP = (float*) (dataPtr + 11);
	dataRate = *dataRateP;

	dataRateP = (float*) (dataPtr + 15);
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
