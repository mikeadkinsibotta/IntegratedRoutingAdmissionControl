/*
 * AdmissionControl.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_
#define LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_
#include <XBee.h>
#include <AODV.h>
#include <PotentialStream.h>
#include <VoiceStreamsManager.h>
#include <VoicePacketSender.h>
#include <Thread.h>
#include <vector>
#include "HeartbeatMessage.h"
#include <Saturation.h>

using namespace std;

class AdmissionControl {
	private:
		vector<PotentialStream> potentialStreams;
		XBee xbee;
		XBeeAddress64 myAddress;
		XBeeAddress64 sinkAddress;
		VoiceStreamManager * voiceStreamManager;
		AODV * aodv;
		VoicePacketSender * voicePacketSender;
		unsigned long grantTimeoutLength;
		unsigned long rejcTimeoutLength;
		vector<Neighbor> neighborhoodTable;
		float localCapacity;
		Saturation satT[7];

		void addPotentialStream(PotentialStream& potentialStream, const float addDataRate);
		bool checkLocalCapacity(const PotentialStream& potentialStream) const;
		void getLocalCapacity(float myDataRate);
		void buildSaturationTable();
	public:
		AdmissionControl();
		AdmissionControl(const XBeeAddress64& myAddress, const XBeeAddress64& sinkAddress, const XBee& xbee,
				AODV * aodv, VoiceStreamManager * voiceStreamManager, VoicePacketSender * voicePacketSender,
				const unsigned long grantTimeoutLength, const unsigned long rejcTimeoutLength);

		void sendInitPacket(const uint8_t codecSetting, const float dupSetting);
		void intializationSenderTimeout();
		void sendGRANTPacket(const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop);
		void sendREDJPacket(const XBeeAddress64 &senderAddress);
		void handleInitPacket(const Rx64Response &response);
		void handleREDJPacket(Rx64Response &response);
		void handleGRANTPacket(const Rx64Response &response, bool& initThreadActive, bool& voiceThreadActive);
		void checkTimers();
		bool removePotentialStream(const XBeeAddress64& packetSource);
		void printPotentialStreams() const;
		void receiveHeartBeat(const float myDataRate, const Rx64Response& response);
		void updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage);
		void updateNeighbor(Neighbor& neighbor, const HeartbeatMessage& heartbeatMessage);
		void broadcastHeartBeat(const float myDataRate, const XBeeAddress64& broadCastAddress,
				const XBeeAddress64& downStreamNeighbor);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_ */
