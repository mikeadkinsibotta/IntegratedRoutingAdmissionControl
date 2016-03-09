#include "VoiceSetting.h"

VoiceSetting::VoiceSetting() {
	compressionSetting = 0;
	dupRatio = 1;
}

VoiceSetting::VoiceSetting(double _compressionSetting, double _dupRatio) {
	compressionSetting = _compressionSetting;
	dupRatio = _dupRatio;
}

double VoiceSetting::getDupRatio() const {
	return dupRatio;
}

void VoiceSetting::setDupRatio(double dupRatio) {
	this->dupRatio = dupRatio;
}

double VoiceSetting::getCompressionSetting() const {
	return compressionSetting;
}

void VoiceSetting::setCompressionSetting(double compressionSetting) {
	this->compressionSetting = compressionSetting;
}

void VoiceSetting::toString() const {

	cout << "Compression Setting " << compressionSetting << " Duplication Ratio " << dupRatio << endl;
}

