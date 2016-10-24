/*
 * NextHopSwitch.cpp
 *
 *  Created on: Oct 22, 2016
 *      Author: mike
 */
#include "NextHopSwitch.h"

NextHopSwitch::NextHopSwitch() {
	timePoint = 0;
	newNextHop = XBeeAddress64();
}

NextHopSwitch::NextHopSwitch(const unsigned long timePoint, const XBeeAddress64& newNextHop) {
	this->timePoint = timePoint;
	this->newNextHop = newNextHop;
}

const XBeeAddress64& NextHopSwitch::getNewNextHop() const {
	return newNextHop;
}

unsigned long NextHopSwitch::getTimePoint() const {
	return timePoint;
}

