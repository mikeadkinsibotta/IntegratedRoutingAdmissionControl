/*
 * Saturation.h
 *
 *  Created on: Jun 29, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_VOICESETTINGS_SATURATION_H_
#define LIBRARIES_VOICESETTINGS_SATURATION_H_

class Saturation {

	private:
		int num;
		float rate;

	public:
		Saturation();
		Saturation(const int num, const float rate);
		int getNum() const;
		void setNum(const int num);
		float getRate() const;
		void setRate(const float rate);
};

#endif /* LIBRARIES_VOICESETTINGS_SATURATION_H_ */
