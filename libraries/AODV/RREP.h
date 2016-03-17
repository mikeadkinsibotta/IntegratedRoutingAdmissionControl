/*
 * RREP.h
 *
 *  Created on: Jul 24, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_AODV_RREP_H_
#define LIBRARIES_AODV_RREP_H_

#include <XBee.h>

using namespace std;

class RREP {
	private:
		XBeeAddress64 sourceAddr;
		XBeeAddress64 destAddr;
		uint32_t destSeqNum;
		uint8_t hopCount;
		uint8_t lifeTime;
	public:
		RREP();
		RREP(XBeeAddress64 sourceAddr, XBeeAddress64 destAddr, uint32_t destSeqNum, uint8_t hopCount, uint8_t lifeTime);
		RREP(uint8_t* dataPtr);
		const XBeeAddress64& getDestAddr() const;
		void setDestAddr(const XBeeAddress64& destAddr);
		uint32_t getDestSeqNum() const;
		void setDestSeqNum(uint32_t destSeqNum);
		uint8_t getHopCount() const;
		void setHopCount(uint8_t hopCount);
		uint8_t getLifeTime() const;
		void setLifeTime(uint8_t lifeTime);
		const XBeeAddress64& getSourceAddr() const;
		void setSourceAddr(const XBeeAddress64& sourceAddr);
		void incrementHopCount();
		void print() const;
};

#endif /* LIBRARIES_AODV_RREP_H_ */
