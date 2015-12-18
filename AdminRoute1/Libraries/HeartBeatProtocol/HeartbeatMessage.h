/*
 * HeartbeatMessage.h
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_
#define LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_

#include "Arduino.h"
#include "XBee.h"

class HeartbeatMessage {

	private:
		XBeeAddress64 senderAddress;
		XBeeAddress64 sinkAddress;
		uint8_t seqNum;
		double dataRate;
		double rssi;
		double qualityOfPath;
		double neighborhoodCapacity;
		bool routeFlag;

	public:
		const XBeeAddress64& getSenderAddress() const;
		void setSenderAddress(const XBeeAddress64& senderAddress);

		const XBeeAddress64& getSinkAddress() const;
		void setSinkAddress(const XBeeAddress64& sinkAddress);

		uint8_t getSeqNum() const;
		void setSeqNum(uint8_t seqNum);

		double getDataRate() const;
		void setDataRate(double dataRate);

		double getRssi() const;
		void setRssi(double rssi);

		double getQualityOfPath() const;
		void setQualityOfPath(double qualityOfPath);

		double getNeighborhoodCapacity() const;
		void setNeighborhoodCapacity(double neighborhoodCapacity);

		bool isRouteFlag() const;
		void setRouteFlag(bool routeFlag);

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_ */
