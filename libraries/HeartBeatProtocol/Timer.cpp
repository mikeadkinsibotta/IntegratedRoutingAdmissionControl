/*
 * Timer.cpp
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */
#include <Timer.h>

Timer::Timer() {
	timer = 0;
	timeoutLength = 0;
}
Timer::Timer(const unsigned long timeoutLength) {
	timer = 0;
	this->timeoutLength = timeoutLength;
}

void Timer::startTimer() {

	timer = millis();
}

bool Timer::timeoutTimer() {
	if (millis() - timer > timeoutLength) {
		return true;
	}
	return false;
}

