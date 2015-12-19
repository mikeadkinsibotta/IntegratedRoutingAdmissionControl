/*
 * Frame.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: mike
 */

#include <Frame.h>

Frame::Frame() {
	this->address = XBeeAddress64();
	this->timestamp = 0;

}

Frame::Frame(const XBeeAddress64& address, const unsigned long timestamp) {
	this->address = address;
	this->timestamp = timestamp;
	// TODO Auto-generated constructor stub

}

const XBeeAddress64& Frame::getAddress() const {
	return address;
}

void Frame::setAddress(const XBeeAddress64& address) {
	this->address = address;
}

unsigned long Frame::getTimestamp() const {
	return timestamp;
}

void Frame::setTimestamp(const unsigned long timestamp) {
	this->timestamp = timestamp;
}

Frame::~Frame() {
	// TODO Auto-generated destructor stub
}

