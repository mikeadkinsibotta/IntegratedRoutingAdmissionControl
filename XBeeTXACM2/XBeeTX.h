// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _XBeeTX_H_
#define _XBeeTX_H_
#include "Arduino.h"

//add your includes for the project XBeeTX here
#include <math.h>
#include <CompressionTable.h>
#include <AdmissionControl.h>
#include <pcadpcm.h>
#include <XBeeResponseThread.h>
#include <ThreadController.h>
#include <VoiceStreamsManager.h>
#include <AODV.h>
#include <Frame.h>

struct CompressionSettings {
		uint8_t codecSetting;
		float dupSetting;
};
//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif
/*
 * Creates at sends data packets using the compression and duplication settings.
 */
void loopSend();

void clearBuffer();

void calculateThroughput();

/*
 * Calculates the total data rate of this node.  A node may be sending data as well as forwarding
 * data from another node to the sink.
 */
void calculateTotalDataRate();

/*
 * Retrieves the address of this node.
 */
XBeeAddress64 getMyAddress();

/*
 * The main method for handling packets.  It then forwards packet data to other methods
 * that handle that particular packet type.
 */
void listenForResponses();

/*
 * TODO not setup.  Needs to clean routes that do not exist anymore.
 */
void cleanUpRouteTable();

/*
 * Adjusts the current data rate depending on the number of data packets lost.
 */
void updateDataRate(const uint8_t dataLoss);

/*
 * Adjusts the packet loss ratio that is used to determine the optimum duplication and compression settings.
 */
void getPacketLossRatio(const uint8_t * dataPtr);

/*
 * Setups an original getRoute for sender before requesting to stream
 */
void requestDataInjectionThread();

/*
 * Cleans up data packets waiting for TX Status response.
 * Nodes that are left in the map, never received a TX status response and
 * are therefore deemed lost.
 */
void purgePacketWaiting();

/****Starts a request route when can't forward packet *******************************/

/*
 * Gets a route to a destination if node needs to forward a packet.  If there is no
 * route.  It calls aodv.getRoute() until it gets one.  Then calls the original function
 * that is trying to forward a packet.
 */
void getRouteWithResponse();

/*
 * Gets a route to a destination if node needs to forward a packet.  If there is no
 * route.  It calls aodv.getRoute() until it gets one.  Then calls the original function
 * that is trying to forward a packet.
 */
void getRouteWithAddress();

/*
 * Check the timers to determine if a stream can be admitted into the network.
 */
void checkTimers();

/******Packet Sending Functions ******************************************************/
void sendPathPacket();
void sendREDJPacket(const XBeeAddress64 &senderAddress);
void sendGRANTPacket(const XBeeAddress64 &senderAddress);
uint8_t* addDestinationToPayload(XBeeAddress64 packetSource, XBeeAddress64 packetDestination, uint8_t payload[],
		uint8_t sizePayload, uint8_t& resultSize, uint8_t& frameId, uint8_t& codec);

/******Packet Handle Functions *******************************************************/
void handleDataPacket(const Rx64Response &response);
void handleAODVPacket(Rx64Response &response, const char control[]);
void handlePathPacket(const Rx64Response &response);
void handleTXStatus(TxStatusResponse &status);
void handleInitPacket(const Rx64Response &response);
void handleGRANTPacket(const Rx64Response &response);
void handleREDJPacket(Rx64Response &response);

//Do not add code below this line
#endif /* _XBeeTX_H_ */
