#ifndef OPEN_LOOP_H_
#define OPEN_LOOP_H_

#include "Includes.h"

class Open_Loop
{
private:

	uint32 *myLocalBuffer;
	byte *myMasterDataPtr;
	byte *FileHeader;
	Msec_Header_S *myMsecHeaderPtr;
	uint16 *msecheader;
	DATA_CPX *buffer;
	Options opt;			//!< Options
	bool OLactive;
	bool findms;
	unsigned long MSOW_S; // Start with this Millisecond(end of last available sub frame)
	int count;

public:
	Open_Loop(Options *_opt);
	~Open_Loop();
	void Start();
	void Stop();
	void bit_unpack(DATA_CPX *unpacked, uint32 *packed, int32 cnt);
};

#endif /* OPEN_LOOP_H_ */
