#ifndef GLOBALS_H
#define GLOBALS_H

#include "Includes.h"

#ifdef GLOBALS_HERE
	#define EXTERN
#else
	#define EXTERN extern
#endif

// Global pointers to raw IF data
//------------------------------------------------------------
EXTERN DATA_CPX *master_data;	//direct data
EXTERN DATA_CPX *master_data_b;	//direct data
//------------------------------------------------------------

// Global pointer to a "housekeeping" structure that may be filled for each millisecond
//------------------------------------------------------------
EXTERN Msec_Header_S MsecHeader;
//------------------------------------------------------------

EXTERN Options gopt;
//------------------------------------------------------------
EXTERN class Channel			*pChannels[MAX_CHANNELS];
EXTERN class Acquisition		*pAcquisition;
EXTERN class FFT				*pFFT;
EXTERN class OLChannel			*pOLChannels[MAX_OL_CHANNELS];
EXTERN class OLChanBck			*pOLChansBck[MAX_OL_CHANNELS];
EXTERN class Open_Loop			*pOpen_Loop;
EXTERN class psrbitgen          *pbg;
//------------------------------------------------------------

// Acquisition stuff, used over and over again so made global
//------------------------------------------------------------
EXTERN DATA_TYPE *Channel_Sine[2*CARRIER_BINS+1];	//align this to 16 bytes boundaries for better mmx/sse2 performance?

EXTERN DATA_TYPE PRN_Codes[NUM_CODES][CODE_CHIPS];			//raw PRN codes
EXTERN DATA_TYPE *PRN_Sampled[NUM_CODES][2*CODE_BINS+1];	//sampled PRN codes (aligned to 16 byte boundaries)

EXTERN Acq_Result Acq_results[NUM_CODES];					//Acquisition results
EXTERN Acq_Result Acq_results_b[NUM_CODES];					//Acquisition results for backward
//------------------------------------------------------------

#ifdef GLOBALS_HERE
	EXTERN char UN[15] = {0x31,0x36,0x20,0x69,0x73,0x20,0x75,0x6E,0x68,0x61,0x70,0x70,0x79,0x2E,0x00};
#else
	EXTERN char UN[15];
#endif

/********************************************************************************/
#endif // GLOBALS_H
