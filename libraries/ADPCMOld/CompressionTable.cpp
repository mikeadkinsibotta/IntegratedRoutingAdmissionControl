#include "CompressionTable.h"

VoiceSetting * Compression::getCompressionTable() {
	return compressionTable;
}

void Compression::buildCompressionLookUpTable() {

	//0% Packet Loss
	compressionTable[0] = VoiceSetting(2, 0);

	//1% Packet Loss
	compressionTable[1] = VoiceSetting(2, 0);

	//2% Packet Loss
	compressionTable[2] = VoiceSetting(2, 0);

	//3% Packet Loss
	compressionTable[3] = VoiceSetting(2, 0);

	//4% Packet Loss
	compressionTable[4] = VoiceSetting(2, 0);

	//5% Packet Loss
	compressionTable[5] = VoiceSetting(2, 0.03);

	//6% Packet Loss
	compressionTable[6] = VoiceSetting(2, .2);

	//7% Packet Loss
	compressionTable[7] = VoiceSetting(2, .33);

	//8% Packet Loss
	compressionTable[8] = VoiceSetting(2, .43);

	//9% Packet Loss
	compressionTable[9] = VoiceSetting(3, 0);

	//10% Packet Loss
	compressionTable[10] = VoiceSetting(3, 0);

	//11% Packet Loss
	compressionTable[11] = VoiceSetting(3, 0);

	//12% Packet Loss
	compressionTable[12] = VoiceSetting(3, 0);

	//13% Packet Loss
	compressionTable[13] = VoiceSetting(3, 0);

	//14% Packet Loss
	compressionTable[14] = VoiceSetting(3, 0.04);

	//15% Packet Loss
	compressionTable[15] = VoiceSetting(3, 0.12);

	//16% Packet Loss
	compressionTable[16] = VoiceSetting(3, 0.19);

	//17% Packet Loss
	compressionTable[17] = VoiceSetting(2, 0.86);

	//18% Packet Loss
	compressionTable[18] = VoiceSetting(2, 0.89);

	//19% Packet Loss
	compressionTable[19] = VoiceSetting(2, 0.92);

	//20% Packet Loss
	compressionTable[20] = VoiceSetting(2, 0.95);

	//21% Packet Loss
	compressionTable[21] = VoiceSetting(2, 0.98);

	//22% Packet Loss
	compressionTable[22] = VoiceSetting(3, 0.5);

	//23% Packet Loss
	compressionTable[23] = VoiceSetting(3, 0.54);

	//24% Packet Loss
	compressionTable[24] = VoiceSetting(3, 0.58);

}
