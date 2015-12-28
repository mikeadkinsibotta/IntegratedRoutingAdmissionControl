#include "Saturation.h"

Saturation::Saturation() {
	this->rate = 0;
	this->num = 0;
}

Saturation::Saturation(const int num, const float rate) {
	this->rate = rate;
	this->num = num;

}

int Saturation::getNum() const {
	return num;
}

void Saturation::setNum(const int num) {
	this->num = num;
}

float Saturation::getRate() const {
	return rate;
}

void Saturation::setRate(const float rate) {
	this->rate = rate;
}
