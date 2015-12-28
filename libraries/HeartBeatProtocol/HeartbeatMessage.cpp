/*
 * HeartbeatMessage.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatMessage.h"

HeartbeatMessage::HeartbeatMessage() {
	senderAddress = XBeeAddress64();
	streamSourceAddress = XBeeAddress64();
	sinkAddress = XBeeAddress64();
	rssi = 0;
	seqNum = 0;
	dataRate = 0.0;
	qualityOfPath = 0;
	neighborhoodCapacity = 0.0;
	routeFlag = false;
}

HeartbeatMessage::HeartbeatMessage(const XBeeAddress64& senderAddress, const XBeeAddress64& streamSourceAddress,
		const XBeeAddress64& sinkAddress, const uint8_t rssi, const uint8_t seqNum, const float dataRate,
		const uint8_t qualityOfPath, const float neighborhoodCapacity, const bool routeFlag) {

	this->senderAddress = senderAddress;
	this->streamSourceAddress = streamSourceAddress;
	this->sinkAddress = sinkAddress;
	this->rssi = rssi;
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
	this->rssi = 0;
	this->seqNum = seqNum;
	this->dataRate = dataRate;
	this->qualityOfPath = qualityOfPath;
	this->neighborhoodCapacity = neighborhoodCapacity;
	this->routeFlag = routeFlag;

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

float HeartbeatMessage::getRssi() const {
	return rssi;
}

void HeartbeatMessage::setRssi(float rssi) {
	this->rssi = rssi;
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

void HeartbeatMessage::printMessage() {

	SerialUSB.print("<SenderAddress: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", StreamSourceAddress: ");
	streamSourceAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", seqNum: ");
	SerialUSB.print(seqNum);
	SerialUSB.print(", RSSI: ");
	SerialUSB.print(rssi);
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

	//XBeeAddress64 broadcast = XBeeAddress64(HEARTBEAT_ADDRESS_1, 0x40B31805);

	//Serial.print("InjectionRateB");
	//Serial.print(dataRate);

	uint8_t * dataRateP = (uint8_t *) &dataRate;
	uint8_t * neighborhoodCapacityP = (uint8_t *) &neighborhoodCapacity;

	//float * dataRateFloat = (float *) dataRateP;
	//float dataRateFloatt = *dataRateFloat;

	//Serial.print("InjectionRateC");
	//Serial.print(dataRateFloatt);

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

	/*uint8_t * dataRateP2 = payload + 24;
	 float * dataRateNeighborP = (float *) dataRateP2;
	 float dataRateNeighbor = *dataRateNeighborP;

	 //Serial.print("InjectionRateD");
	 //Serial.print(dataRateNeighbor);
	 /*uint8_t payload[] = { 'B', 'E', 'A', 'T', '\0', (sinkAddress.getMsb() >> 24) & 0xff, (sinkAddress.getMsb() >> 16)
	 & 0xff, (sinkAddress.getMsb() >> 8) & 0xff, sinkAddress.getMsb() & 0xff, (sinkAddress.getLsb() >> 24)
	 & 0xff, (sinkAddress.getLsb() >> 16) & 0xff, (sinkAddress.getLsb() >> 8) & 0xff, sinkAddress.getLsb()
	 & 0xff, (streamSourceAddress.getMsb() >> 24) & 0xff, (streamSourceAddress.getMsb() >> 16) & 0xff,
	 (streamSourceAddress.getMsb() >> 8) & 0xff, streamSourceAddress.getMsb() & 0xff,
	 (streamSourceAddress.getLsb() >> 24) & 0xff, (streamSourceAddress.getLsb() >> 16) & 0xff,
	 (streamSourceAddress.getLsb() >> 8) & 0xff, streamSourceAddress.getLsb() & 0xff, seqNum, qualityOfPath,
	 routeFlag, dataRateP[0], dataRateP[1], dataRateP[2], dataRateP[3], neighborhoodCapacityP[0],
	 neighborhoodCapacityP[1], neighborhoodCapacityP[2], neighborhoodCapacityP[3] };*/

	//xbee.send(tx);
}

