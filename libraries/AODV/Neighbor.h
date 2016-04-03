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
#include <deque>

using namespace std;

class Neighbor {

	private:
		XBeeAddress64 address;
		XBeeAddress64 downStreamNeighbor;
		float dataRate;

	public:
		Neighbor();
		Neighbor(const float dataRate, const XBeeAddress64& address, const XBeeAddress64& downStreamNeighbor);
		const XBeeAddress64& getAddress() const;
		void setAddress(const XBeeAddress64& address);
		float getDataRate() const;
		void setDataRate(float dataRate);
		const XBeeAddress64& getDownStreamNeighbor() const;
		void setDownStreamNeighbor(const XBeeAddress64& downStreamNeighbor);
};

#endif
