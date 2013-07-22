#ifndef COMMONDEFINES_H
#define COMMONDEFINES_H

// Number of channels in the receiver
#define MAX_CHANNELS			(12)
#define MAX_OL_CHANNELS			(8)

#define RECORDER_TYPE  (GISMOS_GRS)
#define PUERTO_RICO			(0)			// Data recorded with Plessy GP2010 front end
#define APL_RECORDER			(0)			// Data recorded with APL Wideband Recorder
#define PURDUE_RECORDER			(0)			// Data recorded with Purdue Recording System
#define JILL_RECORDER			(0)			// Data recorded on Jill's Flight (with APL Recorder)
#define GISMOS_GRS			(1)			// Data recorded with the GISMOS GRS system from APL
//********************************************************************************
#include "RecorderDefines.h" // Set the correct values depending on our recorder type.  Has to occur after RECORDER_TYPE

//********************************************************************************
// Mode of operation?
#define PROCESS_MODE  (OPEN_LOOP)
//********************************************************************************

//********************************************************************************
// WAAS Mode?
#define WAAS	(0)
//********************************************************************************

//********************************************************************************

//Acquisition_PP defines
#define ACQ_DOPPLER_BINS		(12)					//(This many bins, plus minus)
#define ACQ_DOPPLER_BIN_WIDTH	(500)					//(Bin spacing (HZ))
#define ACQ_CTICKS				(1)						//(Integrate COHERENTLY for this many 1ms code epochs)
#define ACQ_ITICKS				(20)					//(Integrate INCOHERENTLY using this many ACQ_CTICKS Coherent integrations)
#define ACQ_TICKS				(ACQ_CTICKS*ACQ_ITICKS)	//Total number of 1ms epochs used in the integration

// Channel Defines (common)
#define	MEASUREMENT_INT		(1000)	// packets of ~1ms data

// Length, in seconds, of the measurement tic
#define TICKS_TO_SECONDS (MEASUREMENT_INT*D_SAMPLES_PER_MS/D_SAMPLE_RATE) 

// All IF data is in shorts, one way or another
typedef short DATA_TYPE;

// Include some standard defines that won't change no matter what your project is.
#include "MathDefines.h"
#include "GPSDefines.h"

#endif // COMMONDEFINES_H
