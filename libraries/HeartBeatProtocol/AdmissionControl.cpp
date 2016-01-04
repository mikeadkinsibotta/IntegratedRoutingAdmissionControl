/*
 * AdmissionControl.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include "AdmissionControl.h"

AdmissionControl::AdmissionControl() {
	heartbeatProtocol = 0;
	timeoutLength = 0;

}

AdmissionControl::AdmissionControl(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const XBee& xbee,
		HeartbeatProtocol * heartbeatProtocol, const unsigned long timeoutLength) {
	this->myAddress = myAddress;
	this->sinkAddress = sinkAddress;
	this->heartbeatProtocol = heartbeatProtocol;
	this->xbee = xbee;
	this->timeoutLength = timeoutLength;

}

void AdmissionControl::checkTimers() {

	for (int i = 0; i < potentialStreams.size(); i++) {

		if (potentialStreams.at(i).getGrantTimer().timeoutTimer()) {
			Serial.println("CheckTimer PotentialStream");
			XBeeAddress64 sourceAddress = potentialStreams.at(i).getSourceAddress();
			XBeeAddress64 nextHop = potentialStreams.at(i).getUpStreamNeighbor();

			sendGRANTPacket(sourceAddress, nextHop);
			removePotentialStream(sourceAddress);
		}
	}

}

void AdmissionControl::sendInitPacket(const uint8_t codecSetting, const float dupSetting) {

	bool hasNextHop = heartbeatProtocol->isRouteFlag();
	if (hasNextHop) {

		XBeeAddress64 heartbeatAddress = heartbeatProtocol->getBroadcastAddress();
		XBeeAddress64 myNextHop = heartbeatProtocol->getNextHopAddress();
		uint8_t injectionRate = 64.00 * (codecSetting / 16.00) * (1.00 + dupSetting);
		uint8_t * injectionRateP = (uint8_t *) &injectionRate;

		uint8_t payloadBroadCast[] = { 'I', 'N', 'I', 'T', '\0', (myAddress.getMsb() >> 24) & 0xff, (myAddress.getMsb()
				>> 16) & 0xff, (myAddress.getMsb() >> 8) & 0xff, myAddress.getMsb() & 0xff, (myAddress.getLsb() >> 24)
				& 0xff, (myAddress.getLsb() >> 16) & 0xff, (myAddress.getLsb() >> 8) & 0xff, myAddress.getLsb() & 0xff,
				myNextHop.getMsb() >> 24, myNextHop.getMsb() >> 16, myNextHop.getMsb() >> 8, myNextHop.getMsb(),
				myNextHop.getLsb() >> 24, myNextHop.getLsb() >> 16, myNextHop.getLsb() >> 8, myNextHop.getLsb(),
				injectionRateP[0], injectionRateP[1], injectionRateP[2], injectionRateP[3] };
		Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));
		// send the command
		xbee.send(tx);
	}
}

void AdmissionControl::sendREDJPacket(const XBeeAddress64 &senderAddress) {

	XBeeAddress64 nextHop = heartbeatProtocol->getNextHopAddress();

	uint8_t payload[] = { 'R', 'E', 'D', 'J', '\0', (senderAddress.getMsb() >> 24) & 0xff,
			(senderAddress.getMsb() >> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
			(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb() >> 8)
					& 0xff, senderAddress.getLsb() & 0xff };
	Tx64Request tx = Tx64Request(nextHop, payload, sizeof(payload));
	xbee.send(tx);

}

void AdmissionControl::sendGRANTPacket(const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop) {

	uint8_t payload[] = { 'G', 'R', 'N', 'T', '\0', (senderAddress.getMsb() >> 24) & 0xff,
			(senderAddress.getMsb() >> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
			(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb() >> 8)
					& 0xff, senderAddress.getLsb() & 0xff };
	Tx64Request tx = Tx64Request(nextHop, ACK_OPTION, payload, sizeof(payload), 0);
	xbee.send(tx);

}

void AdmissionControl::handleInitPacket(const Rx64Response &response) {
	Serial.print("ReceivedInitPacket");

	XBeeAddress64 senderAddress;
	XBeeAddress64 nextHop;
	uint8_t* dataPtr = response.getData();
	senderAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	senderAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	nextHop.setMsb(
			(uint32_t(dataPtr[13]) << 24) + (uint32_t(dataPtr[14]) << 16) + (uint16_t(dataPtr[15]) << 8) + dataPtr[16]);

	nextHop.setLsb(
			(uint32_t(dataPtr[17]) << 24) + (uint32_t(dataPtr[18]) << 16) + (uint16_t(dataPtr[19]) << 8) + dataPtr[20]);

	float * dataRateP = (float*) (dataPtr + 21);
	float dataRate = *dataRateP;

	if (nextHop.equals(myAddress)) {
		XBeeAddress64 heartbeatAddress = heartbeatProtocol->getBroadcastAddress();
		XBeeAddress64 myNextHop = heartbeatProtocol->getNextHopAddress();

		uint8_t * injectionRateP = (uint8_t *) &dataRate;

		uint8_t payloadBroadCast[] = { 'I', 'N', 'I', 'T', '\0', (senderAddress.getMsb() >> 24) & 0xff,
				(senderAddress.getMsb() >> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb()
						& 0xff, (senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff,
				(senderAddress.getLsb() >> 8) & 0xff, senderAddress.getLsb() & 0xff, myNextHop.getMsb() >> 24,
				myNextHop.getMsb() >> 16, myNextHop.getMsb() >> 8, myNextHop.getMsb(), myNextHop.getLsb() >> 24,
				myNextHop.getLsb() >> 16, myNextHop.getLsb() >> 8, myNextHop.getLsb(), injectionRateP[0],
				injectionRateP[1], injectionRateP[2], injectionRateP[3] };

		Tx64Request tx = Tx64Request(heartbeatAddress, payloadBroadCast, sizeof(payloadBroadCast));

		PotentialStream potentialStream = PotentialStream(senderAddress, response.getRemoteAddress64(), timeoutLength);
		potentialStream.getGrantTimer().startTimer();
		potentialStreams.push_back(potentialStream);

		xbee.send(tx);

	} else if (nextHop.equals(sinkAddress)) {
		//Start Grant Timer
		PotentialStream potentialStream = PotentialStream(senderAddress, response.getRemoteAddress64(), timeoutLength);
		potentialStreams.push_back(potentialStream);
	}

//TODO Check Local Capacity;

}

void AdmissionControl::handleREDJPacket(Rx64Response &response) {
	Serial.print("ReceiveREDJ");
	XBeeAddress64 senderAddress;
	uint8_t * dataPtr = response.getData();

	senderAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	senderAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (myAddress.equals(sinkAddress)) {
		removePotentialStream(senderAddress);
	} else {
		sendREDJPacket(senderAddress);
	}

}

void AdmissionControl::handleGRANTPacket(const Rx64Response &response) {
	XBeeAddress64 sourceAddress;
	uint8_t * dataPtr = response.getData();

	sourceAddress.setMsb(
			(uint32_t(dataPtr[5]) << 24) + (uint32_t(dataPtr[6]) << 16) + (uint16_t(dataPtr[7]) << 8) + dataPtr[8]);

	sourceAddress.setLsb(
			(uint32_t(dataPtr[9]) << 24) + (uint32_t(dataPtr[10]) << 16) + (uint16_t(dataPtr[11]) << 8) + dataPtr[12]);

	if (!myAddress.equals(sourceAddress)) {
		Serial.print("ForwardGRANT");

		for (int i = 0; i < potentialStreams.size(); i++) {
			//SerialUSB.println("Old Stream");
			if (potentialStreams.at(i).getSourceAddress().equals(sourceAddress)) {
				sendGRANTPacket(sourceAddress, potentialStreams.at(i).getUpStreamNeighbor());
				removePotentialStream(sourceAddress);
				break;
			}
		}

	} else {
		//TODO Send Data
	}
}

void AdmissionControl::intializationSenderTimeout() {
	SerialUSB.print("InitializationTimeout");

	//TODO need to resend INIT message;
}

bool AdmissionControl::removePotentialStream(const XBeeAddress64& packetSource) {
	int i = 0;
	for (vector<PotentialStream>::iterator it = potentialStreams.begin(); it != potentialStreams.end(); ++it) {
		if (potentialStreams.at(i).getSourceAddress().equals(packetSource)) {
			potentialStreams.erase(it);
			return true;
		}
		++i;
	}
	return false;
}

