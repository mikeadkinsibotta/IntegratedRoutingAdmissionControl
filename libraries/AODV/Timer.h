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
		bool active;

	public:
		Timer();
		Timer(unsigned long timeoutLength);
		Timer(const Timer &obj);
		Timer& operator =(const Timer &obj);

		void startTimer();
		bool timeoutTimer();
		bool isActive() const;
};

#endif /* LIBRARIES_ADPCM_TIMER_H_ */
