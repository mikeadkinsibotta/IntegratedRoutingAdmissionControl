/*
 * PotentialStream.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_POTENTIALSTREAM_H_
#define LIBRARIES_HEARTBEATPROTOCOL_POTENTIALSTREAM_H_

#include <XBee.h>
#include <Timer.h>

class PotentialStream {

	private:
		XBeeAddress64 sourceAddress;
		XBeeAddress64 upStreamNeighbor;
		Timer grantTimer;

	public:
		PotentialStream(XBeeAddress64 sourceAddress, XBeeAddress64 upStreamNeighbor, unsigned long grantTimeoutLength);

		Timer& getGrantTimer();

		const XBeeAddress64& getSourceAddress() const;
		const XBeeAddress64& getUpStreamNeighbor() const;

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_POTENTIALSTREAM_H_ */
