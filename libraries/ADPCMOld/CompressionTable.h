/*
 * CompressionTable.cpp
 *
 *  Created on: Jun 11, 2015
 *      Author: mike
 */

#ifndef COMPRESSIONTABLE_CPP_
#define COMPRESSIONTABLE_CPP_

#include <VoiceSetting.h>

class Compression {
	private:

		VoiceSetting compressionTable[25];

	public:
		void buildCompressionLookUpTable();
		VoiceSetting* getCompressionTable();

};

#endif /* COMPRESSIONTABLE_CPP_ */
