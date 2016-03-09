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
		Timer rejcTimer;
		float increasedDataRate;
		bool onPath;

	public:
		PotentialStream(const XBeeAddress64& sourceAddress, const XBeeAddress64& upStreamNeighbor,
				const unsigned long grantTimeoutLength, const unsigned long rejcTimeoutLength,
				const float increasedDataRate);
		PotentialStream& operator =(const PotentialStream &obj);

		Timer& getGrantTimer();
		const XBeeAddress64& getSourceAddress() const;
		const XBeeAddress64& getUpStreamNeighbor() const;
		void printPotentialStream() const;
		void increaseDataRate(const float increasedDataRate);
		Timer& getRejcTimer();
		float getIncreasedDataRate() const;
		bool isOnPath() const;
		void setOnPath(bool onPath);
		void setGrantTimer(const Timer& grantTimer);
		void setRejcTimer(const Timer& rejcTimer);
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_POTENTIALSTREAM_H_ */
