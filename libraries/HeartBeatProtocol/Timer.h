/*
 * Timer.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */

#ifndef LIBRARIES_ADPCM_TIMER_H_
#define LIBRARIES_ADPCM_TIMER_H_

#include <XBee.h>

class Timer {

	private:
		unsigned long timeStamp;
		unsigned long timeoutLength;

	public:
		Timer();
		Timer(unsigned long timeoutLength);

		void startTimer();
		bool timeoutTimer();

};

#endif /* LIBRARIES_ADPCM_TIMER_H_ */
