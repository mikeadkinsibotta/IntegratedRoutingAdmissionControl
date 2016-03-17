/*
 * RoutingTableEntry.h
 *
 *  Created on: Jul 24, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_AODV_ROUTINGTABLEENTRY_H_
#define LIBRARIES_AODV_ROUTINGTABLEENTRY_H_
#include <XBee.h>
#include <vector>
using namespace std;

class RoutingTableEntry {

	private:
		XBeeAddress64 destinationAddress;
		XBeeAddress64 nextHop;
		uint8_t hopCount;
		uint8_t seqNumDest;
		unsigned long experiationTime;
		vector<XBeeAddress64> activeNeighbors;
		bool active;

	public:
		RoutingTableEntry();
		RoutingTableEntry(const XBeeAddress64& destinationAddress, const XBeeAddress64& nextHop, uint8_t hopCount,
				uint32_t seqNum, unsigned long experiationTime);
		const XBeeAddress64& getDestinationAddress() const;
		void setDestinationAddress(const XBeeAddress64& destinationAddress);
		unsigned long getExperiationTime() const;
		void setExperiationTime(const unsigned long experiationTime);
		uint8_t getHopCount() const;
		void setHopCount(uint8_t hopCount);
		const XBeeAddress64& getNextHop() const;
		void setNextHop(const XBeeAddress64& nextHop);
		uint32_t getSeqNumDest() const;
		void setSeqNumDest(uint32_t seqNumDest);
		void printRoute();
		bool isActive() const;
		void setActive(bool valid);
		vector<XBeeAddress64>& getActiveNeighbors();
		void setActiveNeighbors(const vector<XBeeAddress64>& activeNeighbors);
};

#endif /* LIBRARIES_AODV_ROUTINGTABLEENTRY_H_ */
