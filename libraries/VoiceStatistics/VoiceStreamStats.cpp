#include "VoiceStreamStats.h"

/*
 * VoiceStreamStats.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */
VoiceStreamStats::VoiceStreamStats() {
	this->upStreamNeighborAddress = XBeeAddress64();
	this->senderAddress = XBeeAddress64();
	this->payloadSize = 0;
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
	codecSetting = 2;
	numNoPacketReceived = 0;
}

VoiceStreamStats::VoiceStreamStats(const XBeeAddress64& senderAddress, const XBeeAddress64& upStreamNeighborAddress,
		const uint8_t payloadSize) {
	this->upStreamNeighborAddress = upStreamNeighborAddress;
	this->senderAddress = senderAddress;
	this->payloadSize = payloadSize;
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
	codecSetting = 2;
	numNoPacketReceived = 0;

}

VoiceStreamStats::VoiceStreamStats(const XBeeAddress64& senderAddress, const uint8_t payloadSize) {
	this->upStreamNeighborAddress = XBeeAddress64();
	this->senderAddress = senderAddress;
	this->payloadSize = payloadSize;
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
	codecSetting = 2;
	numNoPacketReceived = 0;

}

const XBeeAddress64& VoiceStreamStats::getSenderAddress() const {
	return senderAddress;
}

double VoiceStreamStats::getThroughput() const {
	return throughput;
}

void VoiceStreamStats::calculateThroughput(const double timeDifference, const double lastDistanceMeasurement) {

	if (totalPacketsRecieved == 0) {
		numNoPacketReceived++;
		voiceQuality = 0;
	} else {
		numNoPacketReceived = 0;
		voiceQuality = (voiceQuality / totalPacketsRecieved);
	}

	if (voiceQuality != 0.00) {

		if (rValueHead < 8) {
			rValueArray[rValueHead] = voiceQuality;
			rValueHead++;
//			SerialUSB.print("Not Maxed out");
//			SerialUSB.println(rValueHead);
		} else {
//			SerialUSB.print("Maxed out: ");
//			SerialUSB.println(rValueHead);
			rValueArray[7] = 0;
			for (int i = 6; i >= 0; --i) {
				rValueArray[i + 1] = rValueArray[i];
			}
			rValueArray[0] = voiceQuality;

			double timepoint = millis() / 1000.0;
			double timeDiff = timepoint - timeStamp - timeDifference;
			timeStamp = timepoint - timeDifference;

			unsigned long totaldata = totalPacketsRecieved * payloadSize * 8;
			double inKb = totaldata / 1000.0;
			throughput = inKb / timeDiff;

			double total = 0;
			for (int i = 0; i < 8; ++i) {
				total += rValueArray[i];
			}

			double voiceQualityMovingAverage = total / 8;
			SerialUSB.print("$");
			senderAddress.printAddressASCII(&SerialUSB);

			SerialUSB.print("   ");
			SerialUSB.print(timeStamp);
			SerialUSB.print("  ");
			SerialUSB.print(voiceQualityMovingAverage);
			SerialUSB.println("  ");

			SerialUSB.print("Source Address: ");
			senderAddress.printAddressASCII(&SerialUSB);

			SerialUSB.print(" Timestamp: ");
			SerialUSB.print(timeStamp);

			SerialUSB.print(" ThroughputRate: ");
			SerialUSB.print(throughput);
			SerialUSB.print(" kbps ");

			SerialUSB.print(" TotalPacketsSent: ");
			SerialUSB.print(totalPacketsSent);

			SerialUSB.print(" TotalPacketRecieved: ");
			SerialUSB.print(totalPacketsRecieved);

			SerialUSB.print("  CurrentPacketLoss: ");
			SerialUSB.print(packetLoss);

			SerialUSB.print("  Compression Last Packet: ");
			SerialUSB.print(codecSetting);

			SerialUSB.print("  Last Distance: ");
			SerialUSB.print(lastDistanceMeasurement);

			SerialUSB.print("  Average Moving R-Quality ");
			SerialUSB.println(voiceQualityMovingAverage);
		}
	}
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
	codecSetting = dataPtr[22];

	//Ignore duplicate if we received the original packet
	if (receivedFrame != duplicateFrame) {

		duplicateFrame = receivedFrame;

		uint8_t lostPackets = receivedFrame - expectedFrameId;

		totalPacketsSent = receivedFrame - intervalStartFrame;

		expectedFrameId++;
		totalPacketsRecieved++;

		double packetLRatio = ((double) lostPackets / totalPacketsSent) * 100.0;

		uint8_t rounded = packetLRatio < 0 ? packetLRatio - 0.5 : packetLRatio + 0.5;

		if (packetLossHead < 8) {
			packetLossArray[packetLossHead] = rounded;
			packetLossHead++;

		} else {
			packetLossArray[7] = 0;
			for (int i = 6; i >= 0; --i) {
				packetLossArray[i + 1] = packetLossArray[i];
			}
			packetLossArray[0] = rounded;

			double total = 0;
			for (int i = 0; i < 8; ++i) {
				total += packetLossArray[i];
			}

			packetLoss = total / 8;

			double alpha = 0;
			double beta = 0;
			double chi = 0;

			switch (codecSetting) {

				case 5:
					alpha = 0.16;
					beta = 8.78;
					chi = 8.21;
					break;

				case 4:
					alpha = 6.50;
					beta = 8.28;
					chi = 5.21;
					break;

				case 3:
					alpha = 18.58;
					beta = 6.08;
					chi = 4.15;
					break;
				case 2:
					alpha = 33.57;
					beta = 2.75;
					chi = 6.58;
					break;

			}

			double voiceLoss = alpha + (beta * (log(1 + (chi * packetLoss))));
			voiceQuality += (94.2 - voiceLoss);
		}
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

uint8_t VoiceStreamStats::getNumNoPacketReceived() const {
	return numNoPacketReceived;
}
