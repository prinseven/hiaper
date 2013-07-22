#ifndef GPSDEFINES_H
#define GPSDEFINES_H

// 10 words/frame, 12 = 10 + 2
#define FRAME_SIZE_PLUS_2	    (12)

// 1 C/A D(t) word = 30 bits
#define NUM_BITS_PER_DATA_WORD  (30)

#define L1_D_NUM_SUBFRAMES		(6)
#define L1_D_MAX_Z_COUNT		(403200)

// Earth's WGS-84 gravitational constant (m^3/s^2) specified for GPS receivers in ICD-GPS-200.
#define GravConstant		    (3.986005E14)

// Earth's WGS-84 rotation rate (rads/s) specified for GPS receivers in ICD-GPS-200.
#define WGS84oe                 (7.2921151467E-5)

// Some periods of time
#define HALF_OF_SECONDS_IN_WEEK (302400.0)
#define SECONDS_IN_WEEK			(604800.0)
#define MSEC_IN_WEEK			(604800000)
#define END_OF_TIME				(12212012)

// Speed of light as specified in ICD-GPS-200 
#define SPEED_OF_LIGHT			(2.99792458e8) //exact

// L1 C/A definitions
#define L1_CODE_RATE			(1023e3)	//(Nominal Code chipping rate)
#define L1_CODE_CHIPS			(1023)		//(Length of CDMA code, in chips)
#define L1_CODES_PER_SEC		(L1_CODE_RATE / L1_CODE_CHIPS) // Number of codes in a second = 1000
#define L1_CODES_PER_MS			(L1_CODE_RATE / L1_CODE_CHIPS * 1000) // Number of codes in a msec = 1
#define L1_CODES_PER_DATA_BIT	(20)		// Number of code cycles per data bit
#if WAAS
	#define L1_NUM_CODES		(51)
#else
	#define L1_NUM_CODES		(32)		//(Number of CDMA codes)
#endif
#define L1_NUM_SVS				(32)		// Number of space vehicles (exclude WAAS)

// Legacy purposes (L1 C/A codes)
#define CODE_RATE (L1_CODE_RATE)
#define CODE_CHIPS (L1_CODE_CHIPS)
#define NUM_CODES (L1_NUM_CODES)

// Frequency definitions for GPS
#define L1_FREQ_HZ (1.57542e9)
#define L2_FREQ_HZ (1.22760e9)
#define L5_FREQ_HZ (1.17645e9)

#endif // GPSDEFINES_H
