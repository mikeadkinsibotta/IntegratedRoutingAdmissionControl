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
}
Timer::Timer(const unsigned long timeoutLength) {
	timeStamp = millis();
	this->timeoutLength = timeoutLength;

}

void Timer::startTimer() {

	timeStamp = millis();
}

bool Timer::timeoutTimer() {
	if (millis() - timeStamp > timeoutLength) {
		return true;
	}
	return false;
}

