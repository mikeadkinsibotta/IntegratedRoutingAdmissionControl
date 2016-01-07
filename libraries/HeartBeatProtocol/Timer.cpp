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

