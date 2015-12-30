/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

const double MILLIWATTS = 0.0000000199526231;
const double DISTANCE = 5.5;
const double N_P = -1.095;

HeartbeatMessage::HeartbeatMessage() {
	senderAddress = XBeeAddress64();
	streamSourceAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	relativeDistance = 0.0;
	seqNum = 0;
	dataRate = 0.0;
	qualityOfPath = 0;
	neighborhoodCapacity = 0.0;
	routeFlag = false;
}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& streamSourceAddress,
		const XBeeAddress64& sinkAddress, const double relativeDistance, const uint8_t seqNum, const float dataRate,
		const uint8_t qualityOfPath, const float neighborhoodCapacity, const bool routeFlag) {

	this->senderAddress = senderAddress;
	this->streamSourceAddress = streamSourceAddress;
	this->sinkAddress = sinkAddress;
	this->relativeDistance = relativeDistance;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& streamSourceAddress, const XBeeAddress64& sinkAddress,
		const uint8_t seqNum, const float dataRate, const uint8_t qualityOfPath, const float neighborhoodCapacity,
		const bool routeFlag) {

	this->senderAddress = XBeeAddress64();
	this->streamSourceAddress = streamSourceAddress;
	this->sinkAddress = sinkAddress;
	this->relativeDistance = 0.0;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

}

void HeartbeatMessage::printMessage() {

	SerialUSB.print("<SenderAddress: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
	//SerialUSB.print(", StreamSourceAddress: ");
	//streamSourceAddress.printAddressASCII(&SerialUSB);
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
	SerialUSB.print(", RouteFlag: ");
	SerialUSB.print(routeFlag);
	SerialUSB.println('>');
}

void HeartbeatMessage::generateBeatMessage(uint8_t payload[]) {

	uint8_t * dataRateP = (uint8_t *) &dataRate;
	uint8_t * neighborhoodCapacityP = (uint8_t *) &neighborhoodCapacity;

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
	payload[13] = (streamSourceAddress.getMsb() >> 24) & 0xff;
	payload[14] = (streamSourceAddress.getMsb() >> 16) & 0xff;
	payload[15] = (streamSourceAddress.getMsb() >> 8) & 0xff;
	payload[16] = streamSourceAddress.getMsb() & 0xff;
	payload[17] = (streamSourceAddress.getLsb() >> 24) & 0xff;
	payload[18] = (streamSourceAddress.getLsb() >> 16) & 0xff;
	payload[19] = (streamSourceAddress.getLsb() >> 8) & 0xff;
	payload[20] = streamSourceAddress.getLsb() & 0xff;
	payload[21] = seqNum;
	payload[22] = qualityOfPath;
	payload[23] = routeFlag;
	payload[24] = dataRateP[0];
	payload[25] = dataRateP[1];
	payload[26] = dataRateP[2];
	payload[27] = dataRateP[3];
	payload[28] = neighborhoodCapacityP[0];
	payload[29] = neighborhoodCapacityP[1];
	payload[30] = neighborhoodCapacityP[2];
	payload[31] = neighborhoodCapacityP[3];
}

void HeartbeatMessage::transcribeHeartbeatPacket(const Rx64Response& response) {

	uint8_t* dataPtr = response.getData() + 5;

	senderAddress = response.getRemoteAddress64();

	double rssi = response.getRssi() * -1;

	double milliWatts = pow(10.0, (rssi / 10.0));
	relativeDistance = DISTANCE * pow((MILLIWATTS / DISTANCE), (-1.0 / N_P));

	sinkAddress.setMsb(
			(uint32_t(dataPtr[0]) << 24) + (uint32_t(dataPtr[1]) << 16) + (uint16_t(dataPtr[2]) << 8) + dataPtr[3]);

	sinkAddress.setLsb(
			(uint32_t(dataPtr[4]) << 24) + (uint32_t(dataPtr[5]) << 16) + (uint16_t(dataPtr[6]) << 8) + dataPtr[7]);

	streamSourceAddress.setMsb(
			(uint32_t(dataPtr[8]) << 24) + (uint32_t(dataPtr[9]) << 16) + (uint16_t(dataPtr[10]) << 8) + dataPtr[11]);

	streamSourceAddress.setLsb(
			(uint32_t(dataPtr[12]) << 24) + (uint32_t(dataPtr[13]) << 16) + (uint16_t(dataPtr[14]) << 8) + dataPtr[15]);

	seqNum = dataPtr[16];
	qualityOfPath = dataPtr[17];
	routeFlag = dataPtr[18];

	float * dataRateP = (float*) (dataPtr + 19);
	dataRate = *dataRateP;

	dataRateP = (float*) (dataPtr + 23);
	neighborhoodCapacity = *dataRateP;

}

const XBeeAddress64& HeartbeatMessage::getStreamSourceAddress() const {
	return streamSourceAddress;
}

void HeartbeatMessage::setStreamSourceAddress(const XBeeAddress64& streamSourceAddress) {
	this->streamSourceAddress = streamSourceAddress;
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
