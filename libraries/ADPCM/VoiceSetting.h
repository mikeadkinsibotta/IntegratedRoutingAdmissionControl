/*
 * VoiceSetting.h
 *
 *  Created on: Jun 11, 2015
 *      Author: mike
 */

#ifndef VOICESETTING_H_
#define VOICESETTING_H_
#include <iostream>

using namespace std;

class VoiceSetting {

	private:
		double compressionSetting;
		double dupRatio;

	public:
		VoiceSetting();
		VoiceSetting(double _compressionSetting, double _dupRatio);

		double getDupRatio() const;
		void setDupRatio(double dupRatio);

		double getCompressionSetting() const;
		void setCompressionSetting(double compressionSetting);

		void toString() const;
};

#endif /* VOICESETTING_H_ */
