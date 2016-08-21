#include "pcadpcm.h"
#define DELAY 260
#define DELAY3 400

/*****************************************************************************
 *	ADPCMEncoder - ADPCM encoder routine                                     *
 *****************************************************************************
 *	Input variables:                                                         *
 *		short sample - 16-bit signed speech sample                           *
 *		struct ADPCMstate *state - ADPCM structure                           *
 *	Return variables:	                                                     *
 *		char - 8-bit number containing the 4-bit ADPCM code	                 *
 *****************************************************************************/

ADPCM::ADPCM() {
	state.previndex = 0;
	state.prevsample = 0;
}

void ADPCM::fiveBitADPCMEncoderGetter(uint8_t code, int i, uint8_t buffer[]) {
	switch (i) {

		case 0:
			buffer[0] = code << 3;
			break;
		case 1:
			buffer[0] |= code >> 2;
			buffer[1] = code << 6;
			break;
		case 2:
			buffer[1] |= code << 1;
			break;
		case 3:
			buffer[1] |= code >> 4;
			buffer[2] = code << 4;
			break;
		case 4:
			buffer[2] |= code >> 1;
			buffer[3] = code << 7;
			break;
		case 5:
			buffer[3] |= code << 2;
			break;
		case 6:
			buffer[3] |= code >> 3;
			buffer[4] = code << 5;
			break;
		case 7:
			buffer[4] |= code;
			break;
	}

}

void ADPCM::threeBitADPCMEncoderGetter(uint8_t code, int i, uint8_t buffer[]) {

	switch (i) {

		case 0:
			buffer[0] = code << 5;
			break;
		case 1:
			buffer[0] |= code << 2;
			break;
		case 2:
			hold = code;
			buffer[0] |= (code >> 1);
			break;
		case 3:
			buffer[1] = hold << 7;
			buffer[1] |= code << 4;
			break;
		case 4:
			buffer[1] |= code << 1;
			break;
		case 5:
			hold = code;
			buffer[1] |= (code >> 2);
			break;
		case 6:
			buffer[2] = hold << 6;
			buffer[2] |= code << 3;
			break;
		case 7:
			buffer[2] |= code;
	}

}

char ADPCM::ADPCMEncoder(short sample, int bits) {
	int code; /* ADPCM output value */
	int diff; /* Difference between sample and the predicted sample */
	int step; /* Quantizer step size */
	int predsample; /* Output of ADPCM predictor */
	//int diffq;			/* Dequantized predicted difference */
	int index; /* Index into step size table */

	/* Restore previous values of predicted sample and quantizer step size index*/
	predsample = (int) (state.prevsample);
	index = state.previndex;
	step = StepSizeTable[index];

	/* Compute the difference between the acutal sample (sample) and the he predicted sample (predsample)*/

	diff = sample - predsample;

	if (diff < 0) {
		switch (bits) {
			case 5:
				code = 16;
				break;
			case 4:
				code = 8;
				break;
			case 3:
				code = 4;
				break;
			case 2:
				code = 2;
				break;
		}

		diff = -diff;
	} else
		code = 0;

	/* Quantize the difference into the 4-bit ADPCM code using the the quantizer step size */

	if (diff >= step) {
		switch (bits) {
			case 5:
				code |= 8;
				break;
			case 4:
				code |= 4;
				break;
			case 3:
				code |= 2;
				break;
			case 2:
				code |= 1;
				break;
		}

		diff -= step;
	}
	if (diff >= (step >> 1)) {
		switch (bits) {
			case 5:
				code |= 4;
				diff -= (step >> 1);
				break;
			case 4:
				code |= 2;
				diff -= (step >> 1);
				break;
			case 3:
				code |= 1;
				break;
		}
	}

	if (diff >= (step >> 2)) {
		switch (bits) {
			case 5:
				code |= 2;
				diff -= (step >> 2);
				break;
			case 4:
				code |= 1;
		}
	}

	if (bits == 5 && diff >= (step >> 3))
		code |= 1;

	/* Inverse quantize the ADPCM code into a predicted difference using the quantizer step size */
	diff = 0;

	if (((bits == 4) && (code & 4)) || ((bits == 3) && (code & 2)) || ((bits == 5) && (code & 8))
			|| ((bits == 2) && (code & 1)))
		diff += step;

	if (((bits == 4) && (code & 2)) || ((bits == 3) && (code & 1)) || ((bits == 5) && (code & 4)))
		diff += (step >> 1);

	if (((bits == 4) && (code & 1)) || ((bits == 5) && (code & 2)))
		diff += (step >> 2);

	if ((bits == 5) && (code & 1))
		diff += (step >> 3);

	if (bits == 4)
		diff += (step >> 3);
	else if (bits == 2)
		diff += (step >> 1);
	else if (bits == 3)
		diff += (step >> 2);
	else
		diff += (step >> 4);

	if ((bits == 4 && (code & 8)) || (bits == 3 && (code & 4)) || ((bits == 5) && (code & 16))
			|| (bits == 2 && (code & 2)))
		predsample -= diff;
	else
		predsample += diff;

	/* Fixed predictor computes new predicted sample by adding the old predicted sample to predicted difference */

	/* Check for overflow of the new predicted sample */
	if (predsample > 32767)
		predsample = 32767;
	else if (predsample < -32767)
		predsample = -32767;

	/* Find new quantizer stepsize index by adding the old index to a table lookup using the ADPCM code */
	if (bits == 4)
		index += IndexTable[code];
	else if (bits == 3)
		index += IndexTable3bit[code];
	else if (bits == 2)
		index += IndexTable2Bit[code];
	else
		index += IndexTable5Bit[code];

	/* Check for overflow of the new quantizer step size index */
	if (index < 0)
		index = 0;
	if (index > 88)
		index = 88;

	/* Save the predicted sample and quantizer step size index for next iteration */
	state.prevsample = (short) predsample;
	state.previndex = index;

	if (bits == 4)
		code &= 0x0f;
	else if (bits == 3)
		code &= 0x07;
	else if (bits == 2)
		code &= 0x03;
	else
		code &= 0x1f;

	/* Return the new ADPCM code */
	return code;
}

/*****************************************************************************
 *	ADPCMDecoder - ADPCM decoder routine                                     *
 *****************************************************************************
 *	Input variables:                                                         *
 *		char code - 8-bit number containing the 4-bit ADPCM code             *
 *		struct ADPCMstate *state - ADPCM structure                           *
 *	Return variables:	                                                     *
 *		int - 16-bit signed speech sample	                                 *
 *****************************************************************************/
int ADPCM::ADPCMDecoder(char code, int bits) {
	int step; /* Quantizer step size */
	int predsample; /* Output of ADPCM predictor */
	int diff; /* Dequantized predicted difference */
	int index;/* Index into step size table */

	/* Restore previous values of predicted sample and quantizer step size index */
	predsample = (int) (state.prevsample);
	index = state.previndex;

	/* Find quantizer step size from lookup table using index */
	step = StepSizeTable[index];

	/* Inverse quantize the ADPCM code into a difference using the quantizer step size */

	diff = 0;
	if (((bits == 4) && (code & 4)) || ((bits == 3) && (code & 2)) || ((bits == 5) && (code & 8))
			|| ((bits == 2) && (code & 1)))
		diff += step;

	if (((bits == 4) && (code & 2)) || ((bits == 3) && (code & 1)) || ((bits == 5) && (code & 4)))
		diff += (step >> 1);

	if (((bits == 4) && (code & 1)) || ((bits == 5) && (code & 2)))
		diff += (step >> 2);

	if ((bits == 5) && (code & 1))
		diff += (step >> 3);

	if (bits == 4)
		diff += (step >> 3);
	else if (bits == 2)
		diff += (step >> 1);
	else if (bits == 3)
		diff += (step >> 2);
	else
		diff += (step >> 4);

	if ((bits == 4 && (code & 8)) || ((bits == 3) && (code & 4)) || ((bits == 5) && (code & 16))
			|| ((bits == 2) && (code & 2)))
		predsample -= diff;
	else
		predsample += diff;

	/* Check for overflow of the new predicted sample */
	if (predsample > 32767)
		predsample = 32767;
	else if (predsample < -32768)
		predsample = -32768;

	/* Find new quantizer step size by adding the old index and a table lookup using the ADPCM code */
	if (bits == 4)
		index += IndexTable[code];
	else if (bits == 3)
		index += IndexTable3bit[code];
	else if (bits == 2)
		index += IndexTable2Bit[code];
	else
		index += IndexTable5Bit[code];

	/* Check for overflow of the new quantizer step size index */
	if (index < 0)
		index = 0;
	if (index > 88)
		index = 88;

	/* Save predicted sample and quantizer step size index for next iteration*/
	state.prevsample = (short) predsample;
	state.previndex = index;

	/* Return the new speech sample */
	return (predsample);
}

void ADPCM::fiveBitEncode(uint8_t buffer[]) {
	uint16_t soundData = 0;
	for (int n = 0; n < 8; n++) {

		delayMicroseconds(DELAY);
		//make sample
		uint16_t sample = (soundData << 8) & 0xff00;
		sample |= soundData;

		uint8_t code = ADPCMEncoder(sample, 5);
		fiveBitADPCMEncoderGetter(code, n, buffer);
	}

}

uint8_t ADPCM::fourBitEncode() {
	uint16_t soundData = 0;

	delayMicroseconds(DELAY);
	//make sample
	uint16_t sample = (soundData << 8) & 0xff00;
	sample |= soundData;

	uint8_t code = ADPCMEncoder(sample, 4);
	code = (code << 4) & 0xf0;

	delayMicroseconds(DELAY);
	//make sample
	sample = (soundData << 8) & 0xff00;
	sample |= soundData;

	code |= ADPCMEncoder(sample, 4);

	return code;

}

void ADPCM::threeBitEncode(uint8_t buffer[]) {
	uint16_t soundData = 0;

	for (int f = 0; f < 8; f++) {

		delayMicroseconds(DELAY3);
		//make sample
		uint16_t sample = (soundData << 8) & 0xff00;
		sample |= soundData;

		uint8_t code = ADPCMEncoder(sample, 3);
		threeBitADPCMEncoderGetter(code, f, buffer);
	}
}

uint8_t ADPCM::twoBitEncode() {
	uint8_t code = 0;

	uint16_t soundData = 0;

	delayMicroseconds(DELAY);
	//make sample
	uint16_t sample = (soundData << 8) & 0xff00;
	sample |= soundData;

	code = ADPCMEncoder(sample, 2);
	code = (code << 6) & 0xff;

	delayMicroseconds(DELAY);
	//make sample
	sample = (soundData << 8) & 0xff00;
	sample |= soundData;

	uint8_t codeT = ADPCMEncoder(sample, 2);
	code |= (codeT << 4) & 0x3f;

	delayMicroseconds(DELAY);
	//make sample
	sample = (soundData << 8) & 0xff00;
	sample |= soundData;

	codeT = ADPCMEncoder(sample, 2);
	code |= (codeT << 2) & 0x0f;

	delayMicroseconds(DELAY);
	//make sample
	sample = (soundData << 8) & 0xff00;
	sample |= soundData;

	code |= ADPCMEncoder(sample, 2);

	return code;
}
