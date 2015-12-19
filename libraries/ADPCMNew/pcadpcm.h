#ifndef pcadpcm_h
#define pcadpcm_h

#include "Arduino.h"
// the #include statment and code go here...
#include "stdio.h"

struct ADPCMstate {
		signed short prevsample;/* Predicted sample */
		int previndex;/* Index into step size table */
};

/* Table of index changes */
const signed char IndexTable[16] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };

const signed int IndexTable3bit[8] = { -1, -1, 1, 2, -1, -1, 1, 2 };

const signed int IndexTable2Bit[4] = { -1, 2, -1, 2 };

const signed int IndexTable5Bit[32] = { -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16, -1, -1, -1, -1, -1,
		-1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16 };

/* Quantizer step size lookup table */
const int StepSizeTable[89] = { 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60,
		66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544,
		598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
		3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818,
		18500, 20350, 22385, 24623, 27086, 29794, 32767 };

class ADPCM {

	private:
		char hold = 0;
		ADPCMstate state;

		/* Function prototype for the ADPCM Encoder routine */
		char ADPCMEncoder(short, int);

		/* Function prototype for the ADPCM Decoder routine */
		int ADPCMDecoder(char, int);

		void fiveBitADPCMEncoderGetter(uint8_t code, int i, uint8_t buffer[]);

		void threeBitADPCMEncoderGetter(uint8_t code, int i, uint8_t buffer[]);

	public:

		ADPCM();

		void fiveBitEncode(uint8_t buffer[]);

		uint8_t fourBitEncode();

		void threeBitEncode(uint8_t buffer[]);

		uint8_t twoBitEncode();

};

#endif
