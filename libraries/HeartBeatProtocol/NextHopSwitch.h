/*
 * NextHopSwitch.h
 *
 *  Created on: Oct 22, 2016
 *      Author: mike
 */
#ifndef LIBRARIES_NEXTHOP_SWITCH_H_
#define LIBRARIES_NEXTHOP_SWITCH_H_
#include "XBee.h"
#include "Arduino.h"

class NextHopSwitch {
	public:
		NextHopSwitch();
		NextHopSwitch(const unsigned long timePoint, const XBeeAddress64& newNextHop);
		const XBeeAddress64& getNewNextHop() const;
		unsigned long getTimePoint() const;

	private:
		unsigned long timePoint;
		XBeeAddress64 newNextHop;

};

#endif
