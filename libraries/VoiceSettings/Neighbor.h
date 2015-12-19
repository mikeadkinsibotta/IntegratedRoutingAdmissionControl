/*
 * Neighbor.h
 *
 *  Created on: Jun 15, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_NEIGHBOR_H_
#define LIBRARIES_NEIGHBOR_H_
#include "Arduino.h"
#include "XBee.h"
class Neighbor {

	private:
		XBeeAddress64 address;
		float dataRate;
		float contentionDomainRate;
		uint8_t contentionDomainSize;

	public:
		Neighbor();
		Neighbor(const XBeeAddress64 &address, const float dateRate);
		Neighbor(const XBeeAddress64 &address, const float dataRate, const float contentionDomainRate,
				const uint8_t contentionDomainSize);

		XBeeAddress64 getAddress() const;
		void setAddress(const XBeeAddress64 &address);
		float getDataRate() const;
		void setDataRate(const float dataRate);
		float getContentionDomainRate() const;
		void setContentionDomainRate(const float contentionDomainRate);
		uint8_t getContentionDomainSize() const;
		void setContentionDomainSize(const uint8_t contentionDomainSize);
		bool compare(const Neighbor &b);
		void printNeighbor() const;
};

#endif
