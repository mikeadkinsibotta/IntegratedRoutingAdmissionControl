/*
 * XBeeResponseThread.h
 *
 *  Created on: Aug 25, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_ARDUINOTHREAD_XBEERESPONSETHREAD_H_
#define LIBRARIES_ARDUINOTHREAD_XBEERESPONSETHREAD_H_

#include <Thread.h>
#include <ThreadController.h>
#include <XBee.h>

class XBeeResponseThread: public Thread {
	private:

		// Callback for run() if not implemented
		Rx64Response response;
		XBeeAddress64 senderAddress;
		XBeeAddress64 destination;
		XBeeAddress64 nextHop;
	public:
		void (*_onRun)(void);
		void (*_callback)(const Rx64Response&);
		void (*_callbackAddress)(const XBeeAddress64&);

		void onRun(void (*onRun)(void));
		const Rx64Response& getResponse() const;
		void setResponse(const Rx64Response& response);
		void run();
		const XBeeAddress64& getDestination() const;
		void setDestination(const XBeeAddress64& destination);
		const XBeeAddress64& getNextHop() const;
		void setNextHop(XBeeAddress64& nextHop);
		const XBeeAddress64& getSenderAddress() const;
		void setSenderAddress(const XBeeAddress64& senderAddress);
};

#endif /* LIBRARIES_ARDUINOTHREAD_XBEERESPONSETHREAD_H_ */
