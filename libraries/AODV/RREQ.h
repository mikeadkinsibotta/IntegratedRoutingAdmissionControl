/*
 * RREQ.h
 *
 *  Created on: Jul 24, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_AODV_RREQ_H_
#define LIBRARIES_AODV_RREQ_H_

#include <iostream>
#include <XBee.h>

using namespace std;

class RREQ {

	private:
		XBeeAddress64 sourceAddr;
		uint32_t sourceSeqNum;
		uint32_t broadcastId;
		uint32_t destSeqNum;
		XBeeAddress64 destAddr;
		uint8_t hopCount;

	public:
		RREQ(const XBeeAddress64& sourceAddr, const uint32_t sourceSeqNum, const uint32_t broadcastId,
				const XBeeAddress64& destAddr, const uint32_t destSeqNum, const uint8_t hopCount);
		RREQ(uint8_t* dataPtr);
		uint32_t getBroadcastId() const;
		void setBroadcastId(uint32_t broadcastId);
		const XBeeAddress64& getDestAddr() const;
		void setDestAddr(const XBeeAddress64& destAddr);
		uint8_t getHopCount() const;
		void setHopCount(uint8_t hopCount);
		uint32_t getSourceSeqNum() const;
		void setSourceSeqNum(uint32_t sourceSeqNum);
		const XBeeAddress64& getSourceAddr() const;
		void setSourceAddr(const XBeeAddress64& sourceAddr);
		uint32_t getDestSeqNum() const;
		void setDestSeqNum(uint32_t destSeqNum);
		void incrementHopCount();
		bool compare(const RREQ &j) const;
		void print() const;

};

#endif /* LIBRARIES_AODV_RREQ_H_ */
