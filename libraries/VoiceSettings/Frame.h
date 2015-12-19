/*
 * Frame.h
 *
 *  Created on: Aug 23, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_VOICESETTINGS_FRAME_H_
#define LIBRARIES_VOICESETTINGS_FRAME_H_
#include "Arduino.h"
#include "XBee.h"

class Frame {
	private:
		XBeeAddress64 address;
		unsigned long timestamp;

	public:
		Frame();
		Frame(const XBeeAddress64& address, const unsigned long timestamp);
		virtual ~Frame();
		const XBeeAddress64& getAddress() const;
		void setAddress(const XBeeAddress64& address);
		unsigned long getTimestamp() const;
		void setTimestamp(unsigned long timestamp);
};

#endif /* LIBRARIES_VOICESETTINGS_FRAME_H_ */
