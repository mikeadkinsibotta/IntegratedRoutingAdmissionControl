// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _AdminRoute1_H_
#define _AdminRoute1_H_
#include "Arduino.h"
//add your includes for the project AdminRoute1 here
#include <HeartbeatProtocol.h>
#include <VoicePacketSender.h>
#include <ThreadController.h>
#include <AdmissionControl.h>

//end of add your includes here

//add your function definitions for the project AdminRoute1 here
void broadcastHeartbeat();
void sendVoicePacket();
void sendInitPacket();
void sendPathPacket();
void runCalculateThroughput();
void runInitialRestart();
void arduinoSetup();
void clearBuffer();
void setupThreads();

//Do not add code below this line
#endif /* _AdminRoute1_H_ */
