/*
 * AdmissionControl.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_
#define LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_
#include <XBee.h>
#include <HeartbeatProtocol.h>
#include <AdmissionControl.h>
#include <PotentialStream.h>
#include <Thread.h>
#include <vector>

using namespace std;

class AdmissionControl {
	private:
		vector<PotentialStream> potentialStreams;
		XBee xbee;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		HeartbeatProtocol * heartbeatProtocol;

	public:
		AdmissionControl();
		AdmissionControl(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const XBee& xbee,
				HeartbeatProtocol * heartbeatProtocol);

		void sendInitPacket(const XBeeAddress64& senderAddress, const XBeeAddress64& myNextHop,
				const float injectionRate);

		void intializationSenderTimeout();
		void sendGRANTPacket(const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop);
		void sendREDJPacket(const XBeeAddress64 &senderAddress);
		void handleInitPacket(const Rx64Response &response);
		void handleREDJPacket(Rx64Response &response);
		void handleGRANTPacket(const Rx64Response &response);
		void checkTimers();
		bool removePotentialStream(const XBeeAddress64& packetSource);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_ */
