/*
 * TraceMessage.h
 *
 *  Created on: Feb 27, 2016
 *      Author: mike
 */

#ifndef LIBRARIES_ADPCM_TRACEMESSAGE_H_
#define LIBRARIES_ADPCM_TRACEMESSAGE_H_

#include <vector>
#include "XBee.h"

using namespace std;

class TraceMessage {

	private:
		uint8_t addressListLength;
		vector<XBeeAddress64> addresses;

	public:
		TraceMessage();
		void transcribeMessage(const Rx64Response& response);
		const vector<XBeeAddress64>& getAddresses() const;
		void setAddresses(const vector<XBeeAddress64>& addresses);
		uint8_t getAddressListLength() const;
		void setAddressListLength(uint8_t addressListLength);
		void printTraceMessage();

};

#endif /* LIBRARIES_ADPCM_TRACEMESSAGE_H_ */
