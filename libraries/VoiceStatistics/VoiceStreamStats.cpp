#include "VoiceStreamStats.h"

/*
 * VoiceStreamStats.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

VoiceStreamStats::VoiceStreamStats(const XBeeAddress64& senderAddress, const XBeeAddress64& upStreamNeighborAddress) {
	this->upStreamNeighborAddress = upStreamNeighborAddress;
	this->senderAddress = senderAddress;
	throughput = 0;
	totalPacketsRecieved = 0;
	expectedFrameId = 1;
	packetLoss = 0;
	receivedFrame = 0;
	intervalStartFrame = 1;
	totalPacketsSent = 0;
	timeStamp = (millis() / 1000.0);
	voiceQuality = 0;
	duplicateFrame = 0;

}

VoiceStreamStats::VoiceStreamStats(const XBeeAddress64& senderAddress) {
	this->upStreamNeighborAddress = XBeeAddress64();
	this->senderAddress = senderAddress;
	throughput = 0;
	totalPacketsRecieved = 0;
	expectedFrameId = 1;
	packetLoss = 0;
	receivedFrame = 0;
	intervalStartFrame = 1;
	totalPacketsSent = 0;
	timeStamp = (millis() / 1000.0);
	voiceQuality = 0;
	duplicateFrame = 0;

}

const XBeeAddress64& VoiceStreamStats::getSenderAddress() const {
	return senderAddress;
}

double VoiceStreamStats::getThroughput() const {
	return throughput;
}

void VoiceStreamStats::calculateThroughput() {

	double timepoint = millis() / 1000.0;
	double timeDiff = timepoint - timeStamp;
	timeStamp = timepoint;

	unsigned long totaldata = totalPacketsRecieved * 100 * 8;
	double inKb = totaldata / 1000.0;
	throughput = inKb / timeDiff;

	SerialUSB.print("Sender: ");
	senderAddress.printAddressASCII(&SerialUSB);

	SerialUSB.print(" Time Point ");
	SerialUSB.print(timepoint);

	SerialUSB.print(" ThroughputRate: ");
	SerialUSB.print(throughput);
	SerialUSB.print(" kbps ");

	SerialUSB.print(" TotalPacketsSent: ");
	SerialUSB.print(totalPacketsSent);

	SerialUSB.print(" TotalPacketRecieved: ");
	SerialUSB.print(totalPacketsRecieved);

	voiceQuality = (voiceQuality / totalPacketsRecieved);
	SerialUSB.print(" Average R-Quality ");
	SerialUSB.println(voiceQuality);

	voiceQuality = 0;

	//Rematch expectedFrameId
	expectedFrameId = receivedFrame + 1;

	//Restart Interval
	intervalStartFrame = receivedFrame;

	//Reset Throughput
	totalPacketsRecieved = 0;

}

void VoiceStreamStats::updateVoiceLoss(const uint8_t * dataPtr) {

	receivedFrame = dataPtr[21];
	const uint8_t codecSetting = dataPtr[22];

	//Ignore duplicate if we received the original packet
	if (receivedFrame != duplicateFrame) {

		duplicateFrame = receivedFrame;

		uint8_t lostPackets = receivedFrame - expectedFrameId;

		totalPacketsSent = receivedFrame - intervalStartFrame;

		expectedFrameId++;
		totalPacketsRecieved++;

		double packetLRatio = ((double) lostPackets / totalPacketsSent) * 100.0;

		SerialUSB.print("Packet Loss: ");
		SerialUSB.println(packetLRatio);

		uint8_t rounded = packetLRatio < 0 ? packetLRatio - 0.5 : packetLRatio + 0.5;

		packetLoss = rounded;

		double x = 0;
		double b = 0;
		double X = 0;
		double compression = 0;

		switch (codecSetting) {

			case 5:
				x = 0.16;
				b = 8.78;
				X = 8.21;
				compression = 0.3125;
				break;

			case 4:
				compression = .25;
				x = 6.50;
				b = 8.28;
				X = 5.21;
				break;

			case 3:
				compression = 0.1875;
				x = 18.58;
				b = 6.08;
				X = 4.15;
				break;
			case 2:
				compression = 0.125;
				x = 33.57;
				b = 2.75;
				X = 6.58;
				break;

		}

		double voiceLoss = x + b * (log(1 + (X * packetLoss)));

		voiceQuality += (94.2 - voiceLoss);
	}
}

void VoiceStreamStats::printRouteEnd() const {

	SerialUSB.print("Sender: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.println(" End Stream");

}

uint8_t VoiceStreamStats::getPacketLoss() const {
	return packetLoss;
}

void VoiceStreamStats::startStream() const {

	SerialUSB.print("Sender: ");
	senderAddress.printAddressASCII(&SerialUSB);
	SerialUSB.println(" Start Stream");

}

const XBeeAddress64& VoiceStreamStats::getUpStreamNeighborAddress() const {
	return upStreamNeighborAddress;
}

void VoiceStreamStats::setUpStreamNeighborAddress(const XBeeAddress64& upStreamNeighborAddress) {
	this->upStreamNeighborAddress = upStreamNeighborAddress;
}

uint8_t VoiceStreamStats::getTotalPacketsRecieved() const {
	return totalPacketsRecieved;
}

void VoiceStreamStats::setTotalPacketsRecieved(uint8_t totalPacketsRecieved) {
	this->totalPacketsRecieved = totalPacketsRecieved;
}

uint8_t VoiceStreamStats::getTotalPacketsSent() const {
	return totalPacketsSent;
}

void VoiceStreamStats::setTotalPacketsSent(uint8_t totalPacketsSent) {
	this->totalPacketsSent = totalPacketsSent;
}
