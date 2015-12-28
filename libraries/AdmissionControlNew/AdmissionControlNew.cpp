#include "AdmissionControlNew.h"

const XBeeAddress64 AdmissionController::broadCastaddr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
const float AdmissionController::fiveBit = 16.2686;
const float AdmissionController::fourBit = 15.956;
const float AdmissionController::threeBit = 13.5807;
const float AdmissionController::twoBit = 4.88994;

AdmissionController::AdmissionController() {
	contentionDomainRate = 0;
	contentionDomainSize = 0;
	sinkAddress = XBeeAddress64();
	myAddress = XBeeAddress64();
	buildSaturationTable();

}

AdmissionController::AdmissionController(const XBeeAddress64 &_sinkAddress, const XBeeAddress64 &myAddress) {
	sinkAddress = _sinkAddress;
	this->myAddress = myAddress;
	contentionDomainRate = 0;
	contentionDomainSize = 0;
	buildSaturationTable();
}

/**
 * Sends a packet out to to contention domain informing them of its current data rate.
 * The data rate includes both its own injection rate and the data rate of nodes that are flowing through this node.
 *
 */
void AdmissionController::sendDatasendInjectionRateAndContentionDomainRate(XBee &xbee) {

	uint8_t * injectionRateP = (uint8_t *) &totalInjectionRate;
	uint8_t * contentionDomainRateP = (uint8_t *) &contentionDomainRate;

	uint8_t payloadBroadCast[] = { 'A', 'S', 'M', '_', '\0', injectionRateP[0], injectionRateP[1], injectionRateP[2],
			injectionRateP[3], contentionDomainRateP[0], contentionDomainRateP[1], contentionDomainRateP[2],
			contentionDomainRateP[3], contentionDomainSize };

	Tx64Request tx = Tx64Request(broadCastaddr64, ACK_OPTION, payloadBroadCast, sizeof(payloadBroadCast), 0);
	// send the command
	xbee.send(tx);
}

/**neighborTotalDataRate
 * A node has informed this node (node i) that it is sending data through it.  This method updates a list of nodes
 * that are sending data through node i.  Total data rate is then calculated that combines data rate of node i and data
 * rates of nodes sending data through i.
 */
void AdmissionController::updateFlowList(const XBeeAddress64 &dataSource) {
	float neighborRate = 0;

	/*for (int i = 0; i < neighborhood.size(); i++) {
	 if (neighborhood.at(i).getAddress().equals(dataSource)) {

	 if (flowList.count(dataSource) == 0) {
	 flowList.insert(std::pair<XBeeAddress64, Neighbor>(dataSource, neighborhood.at(i)));

	 } else {
	 Neighbor flowNeighbor = flowList[dataSource];
	 flowNeighbor.setDataRate(neighborhood.at(i).getDataRate());
	 flowNeighbor.setContentionDomainRate(neighborhood.at(i).getContentionDomainRate());
	 flowNeighbor.setContentionDomainSize(neighborhood.at(i).getContentionDomainSize());
	 flowList[dataSource] = flowNeighbor;
	 }

	 break;
	 }
	 }

	 float itotalDataRate = 0;
	 for (std::map<XBeeAddress64, Neighbor>::iterator it = flowList.begin(); it != flowList.end(); ++it) {
	 itotalDataRate += it->second.getDataRate();
	 }*/

	//totalInjectionRate = itotalDataRate + injectionRate;
}

void AdmissionController::updateMyRatesInMyNeighborhood(const XBeeAddress64 &address, const float _injectionRate,
		const float _contentionDomainRate, const uint8_t _contentionDomainSize) {

//If I am sending data need to make sure I'm included in my contention domain

	/*(bool found = false;

	 for (int i = 0; i < neighborhood.size(); i++) {
	 if (neighborhood.at(i).getAddress().equals(address)) {
	 neighborhood.at(i).setDataRate(_injectionRate);
	 neighborhood.at(i).setContentionDomainRate(_contentionDomainRate);
	 neighborhood.at(i).setContentionDomainSize(_contentionDomainSize);
	 found = true;
	 break;
	 }
	 }

	 if (!found) {
	 //Serial.print("Add Myself");
	 Neighbor newNeighbor(address, _injectionRate, _contentionDomainRate, _contentionDomainSize);
	 neighborhood.push_back(newNeighbor);
	 }

	 //Need to update my total injection rate if my own injection rate was modified.

	 /*Recalculate contention domain.
	 * Get the injectionRates from neighbors and sum.

	 contentionDomainRate = 0;
	 for (int i = 0; i < neighborhood.size(); i++) {
	 contentionDomainRate += neighborhood.at(i).getDataRate();
	 }
	 contentionDomainRate += totalInjectionRate;
	 contentionDomainSize = neighborhood.size() + 1;*/

}

void AdmissionController::LocalCapacityInfo() {
	Serial.print("ContentionDomainRate");
	Serial.print(contentionDomainRate);

	Serial.print("ContentionDomainSize");
	Serial.print(contentionDomainSize);

	Serial.print("InjectionRate");
	Serial.print(injectionRate);

	Serial.print("TotalInjectionRate");
	Serial.print(totalInjectionRate);
}

/**
 * A node can receive two types of non data packets:
 * 1) ASM packet from a neighbor sends its own injectionRate and its neighborhood rate
 *
 */
void AdmissionController::updateNeighborRatesCalculateMyNeighborhoodRate(Rx64Response &response) {

	bool found = false;

//Get node's injection rate
	uint8_t* bytePointer = response.getData() + 5;
	float * dataRateP = (float*) bytePointer;
	float neighborDataRate = *dataRateP;

	bytePointer = response.getData() + 9;
	dataRateP = (float*) bytePointer;
	float contentionDomainRate = *dataRateP;

	uint8_t neighborContentionDomainSize = response.getData(13);

	XBeeAddress64 address = response.getRemoteAddress64();

	updateMyRatesInMyNeighborhood(address, neighborDataRate, contentionDomainRate, neighborContentionDomainSize);

}

float AdmissionController::requestToStream(XBee &xbee, const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop,
		const float injectionRate) {
	contentionDomainIncrease += injectionRate;

	uint8_t * injectionRateP = (uint8_t *) &injectionRate;

	uint8_t payloadBroadCast[] = { 'I', 'N', 'I', 'T', '\0', (senderAddress.getMsb() >> 24) & 0xff,
			(senderAddress.getMsb() >> 16) & 0xff, (senderAddress.getMsb() >> 8) & 0xff, senderAddress.getMsb() & 0xff,
			(senderAddress.getLsb() >> 24) & 0xff, (senderAddress.getLsb() >> 16) & 0xff, (senderAddress.getLsb() >> 8)
					& 0xff, senderAddress.getLsb() & 0xff, nextHop.getMsb() >> 24, nextHop.getMsb() >> 16,
			nextHop.getMsb() >> 8, nextHop.getMsb(), nextHop.getLsb() >> 24, nextHop.getLsb() >> 16, nextHop.getLsb()
					>> 8, nextHop.getLsb(), injectionRateP[0], injectionRateP[1], injectionRateP[2], injectionRateP[3] };
	Tx64Request tx = Tx64Request(broadCastaddr64, ACK_OPTION, payloadBroadCast, sizeof(payloadBroadCast), 0);
	// send the command
	xbee.send(tx);
	return contentionDomainIncrease;
}

void AdmissionController::printAddress(uint32_t lowerAddress) {

	Serial.print((char) ((lowerAddress >> 24) & 0xff));
	Serial.print((char) ((lowerAddress >> 16) & 0xff));
	Serial.print((char) ((lowerAddress >> 8) & 0xff));
	Serial.print((char) (lowerAddress & 0xff));

}

void AdmissionController::AskForBandwidthInContentionDomain(XBee &xbee) {

	uint8_t payloadBroadCast[3] = "LC";

	Tx64Request tx = Tx64Request(broadCastaddr64, DISABE_ACK_OPTION_AND_ROUTE_DISCOVERY, payloadBroadCast,
			sizeof(payloadBroadCast), 0);
// send the command
	xbee.send(tx);

}

void AdmissionController::buildSaturationTable() {
	satT[0] = Saturation(2, 120.90);
	satT[1] = Saturation(3, 153.39);
	satT[2] = Saturation(4, 151.2);
	satT[3] = Saturation(5, 154.45);
	satT[4] = Saturation(6, 111.42);

}

float AdmissionController::calculateLocalCapacity() {
	float capacity = numeric_limits<float>::max();
	/*for (int i = 0; i < neighborhood.size(); i++) {
	 Neighbor j = neighborhood.at(i);
	 uint8_t cSize = j.getContentionDomainSize();

	 float capacityI = satT[cSize - 2].getRate() - j.getContentionDomainRate();
	 if (capacityI < capacity)
	 capacity = capacityI;
	 }*/

	return capacity;
}

float AdmissionController::updateContentionDomainIncrease(const Rx64Response &response, float &neighborDataRate) {
	uint8_t* bytePointer = response.getData() + 21;
	float * dataRateP = (float*) bytePointer;
	neighborDataRate = *dataRateP;

	contentionDomainIncrease += neighborDataRate;
	return contentionDomainIncrease;

}

bool AdmissionController::checkLocalCapacity() {
	if (contentionDomainIncrease > calculateLocalCapacity()) {
		contentionDomainIncrease = 0;
		return false;
	}
	contentionDomainIncrease = 0;
	return true;

}

bool AdmissionController::checkStreamQualityConstraintNoData(float dupSetting, uint8_t codecSetting,
		uint8_t packetLossRatio) {
	double dataLossRatio = ((1 - dupSetting) * (packetLossRatio)) + ((dupSetting * packetLossRatio * packetLossRatio));

	double x = 0;
	double b = 0;
	double X = 0;
	double compression = 0;
	switch (codecSetting) {

		case 5:
			x = 0.16;
			b = 8.78;
			X = 8.21;
			compression = 0.3125;
			break;

		case 4:
			compression = .25;
			x = 6.50;
			b = 8.28;
			X = 5.21;
			break;

		case 3:
			compression = 0.1875;
			x = 18.58;
			b = 6.08;
			X = 4.15;
			break;
		case 2:
			compression = 0.125;
			x = 33.57;
			b = 2.75;
			X = 6.58;
			break;

	}

	double voiceLoss = x + b * (log(1 + (X * dataLossRatio)));
	//double injectionRate = 64 * compression * (1 + dupSetting);
	double injectionRate = 64 * compression * (1 + dupSetting);

	//loat ep = (1 - dupRatio) * packetLossRatio + dupRatio * (packetLossRatio * packetLossRatio);

	//Check to make sure 93.2 - ep > 50
	return voiceLoss < 43.2;
}

bool AdmissionController::checkStreamQualityConstraint(float dupSetting, uint8_t codecSetting,
		uint8_t packetLossRatio) {

	double dataLossRatio = ((1 - dupSetting) * (packetLossRatio)) + ((dupSetting * packetLossRatio * packetLossRatio));

	double x = 0;
	double b = 0;
	double X = 0;
	double compression = 0;
	switch (codecSetting) {

		case 5:
			x = 0.16;
			b = 8.78;
			X = 8.21;
			compression = 0.3125;
			break;

		case 4:
			compression = .25;
			x = 6.50;
			b = 8.28;
			X = 5.21;
			break;

		case 3:
			compression = 0.1875;
			x = 18.58;
			b = 6.08;
			X = 4.15;
			break;
		case 2:
			compression = 0.125;
			x = 33.57;
			b = 2.75;
			X = 6.58;
			break;

	}

	double voiceLoss = x + b * (log(1 + (X * dataLossRatio)));
	//double injectionRate = 64 * compression * (1 + dupSetting);
	double injectionRate = 64 * compression * (1 + dupSetting);
	double rValue = 93.2 - voiceLoss;

	//loat ep = (1 - dupRatio) * packetLossRatio + dupRatio * (packetLossRatio * packetLossRatio);
	SerialUSB.print(" Time ");
	SerialUSB.print(millis() / 1000);
	SerialUSB.print(" Voice Loss: ");
	SerialUSB.print(voiceLoss);
	SerialUSB.print(" R-Value: ");
	SerialUSB.print(93.2 - voiceLoss);
	SerialUSB.print(" Injection Rate: ");
	SerialUSB.println(injectionRate);
//Check to make sure 93.2 - ep > 50
	return voiceLoss < 43.2;

}

void AdmissionController::sendREJCMessageToSender(XBee &xbee, XBeeAddress64 requesterAddress) {

}

float AdmissionController::getInjectionRate() const {
	return injectionRate;
}

void AdmissionController::setInjectionRate(const float injectionRate) {
	this->injectionRate = injectionRate;
}
//TODO need to implement when neighbor leaves

//Serial.print("Received ");
//Serial.println(*data, BIN);

