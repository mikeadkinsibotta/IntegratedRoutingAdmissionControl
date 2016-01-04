#ifndef AdmissionControl_h
#define AdmissionControl_h

#include "Arduino.h"
#include <XBee.h>
#include <Saturation.h>
#include <vector>
#include <Neighbor.h>
#include <limits>
#include <map>

/*
 * Determine the neighbors in the contention domain.
 */

using namespace std;

union injectRate {
		uint8_t b[4];
		float rate;
};

class AdmissionController {
	private:
		static const XBeeAddress64 broadCastaddr64;
		static const float fiveBit;
		static const float fourBit;
		static const float threeBit;
		static const float twoBit;

		vector<Neighbor> neighborhood;
		std::map<XBeeAddress64, Neighbor> flowList;

		float contentionDomainRate = 0;
		float injectionRate = 0;
		float totalInjectionRate = 0;
		uint8_t contentionDomainSize;
		XBeeAddress64 sinkAddress;
		float contentionDomainIncrease = 0;
		XBeeAddress64 myAddress;

		void sendREJCMessageToSender(XBee &xbee, XBeeAddress64 requesterAddress);

	public:
		AdmissionController();

		AdmissionController(const XBeeAddress64 &_sinkAddress, const XBeeAddress64 &myAddress);

		void sendDatasendInjectionRateAndContentionDomainRate(XBee &xbee);

		void updateMyRatesInMyNeighborhood(const XBeeAddress64 &myAddress);

		void updateMyRatesInMyNeighborhood(const XBeeAddress64 &myAddress, const float injectionRate,
				const float contentionDomainRate, const uint8_t contentionDomainSize);

		void updateNeighborRatesCalculateMyNeighborhoodRate(Rx64Response &response);

		void printAddress(uint32_t lowerAddress);

		void AskForBandwidthInContentionDomain(XBee &xbee);

		void buildSaturationTable();

		void updateFlowList(const XBeeAddress64 &dataSource);

		float requestToStream(XBee &xbee, const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop,
				const float injectionRate);

		void forwardStream(XBee &xbee, const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop,
				const float injectionRate);

		float calculateLocalCapacity();

		bool checkLocalCapacity();

		float updateContentionDomainIncrease(const Rx64Response &response, float &neighborDataRate);

		bool checkStreamQualityConstraintNoData(float dupSetting, uint8_t codecSetting, uint8_t packetLossRatio);

		bool checkStreamQualityConstraint(float dupSetting, uint8_t codecSetting, uint8_t packetLossRatio);

		float getInjectionRate() const;

		void setInjectionRate(const float injectionRate);

		void LocalCapacityInfo();
};

#endif
