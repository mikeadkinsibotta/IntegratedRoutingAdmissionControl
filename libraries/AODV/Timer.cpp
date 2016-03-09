/*
 * Timer.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include "Timer.h"

Timer::Timer() {
	timeStamp = millis();
	timeoutLength = 0;
	active = false;
}
Timer::Timer(const unsigned long timeoutLength) {
	timeStamp = 0;
	this->timeoutLength = timeoutLength;
	active = false;

}

Timer& Timer::operator =(const Timer &obj) {
	active = obj.active;
	timeoutLength = obj.timeoutLength;
	timeStamp = obj.timeStamp;
	return *this;
}

Timer::Timer(const Timer &obj) {
	active = obj.active;
	timeoutLength = obj.timeoutLength;
	timeStamp = obj.timeStamp;
}

void Timer::startTimer() {
	timeStamp = millis();
	active = true;
}

bool Timer::timeoutTimer() {
	if ((millis() - timeStamp) > timeoutLength && active) {
		active = false;
		return true;
	}
	return false;
}

bool Timer::isActive() const {
	return active;
}
