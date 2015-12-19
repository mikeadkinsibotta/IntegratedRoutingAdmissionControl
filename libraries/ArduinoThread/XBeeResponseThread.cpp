#include "XBeeResponseThread.h"

void XBeeResponseThread::onRun(void (*onRun)(void)) {
	_onRun = onRun;
}

const Rx64Response& XBeeResponseThread::getResponse() const {
	return response;
}

void XBeeResponseThread::setResponse(const Rx64Response& response) {
	this->response = response;
}

const XBeeAddress64& XBeeResponseThread::getDestination() const {
	return destination;
}

void XBeeResponseThread::setDestination(const XBeeAddress64& destination) {
	this->destination = destination;
}

const XBeeAddress64& XBeeResponseThread::getNextHop() const {
	return nextHop;
}

const XBeeAddress64& XBeeResponseThread::getSenderAddress() const {
	return senderAddress;
}

void XBeeResponseThread::setSenderAddress(const XBeeAddress64& senderAddress) {
	this->senderAddress = senderAddress;
}

void XBeeResponseThread::setNextHop(XBeeAddress64& nextHop) {
	this->nextHop = nextHop;
}

void XBeeResponseThread::run() {
	if(_onRun != NULL)
		_onRun();
	// Update last_run and _cached_next_run
	runned();
}

