#ifndef MATHDEFINES_H
#define MATHDEFINES_H

// Constants for scaling the ephemeris found in the data message
// the format is the following: TWO_N5 -> 2^-5, TWO_P4 -> 2^4, PI_TWO_N43 -> Pi*2^-43, etc etc
//********************************************************************************
#define PI	 				(3.14159265358979)
#define TWO_PI				(6.28318530717959)
#define THREE_PI_OVER_2		(4.71238898038469)
#define PI_OVER_2			(1.57079632679490)
#define TWO_P4				(16)
#define TWO_N5				(0.03125)
#define TWO_N19				(1.907348632812500e-006)
#define TWO_N29				(1.862645149230957e-009)
#define TWO_N31				(4.656612873077393e-010)
#define TWO_N33				(1.164153218269348e-010)
#define TWO_N43				(1.136868377216160e-013)
#define TWO_N55				(2.775557561562891e-017)
#define PI_TWO_N43			(3.571577341960839e-013)
#define PI_TWO_N31			(1.462918079267160e-009)
#define PI_TWO_P31			(6.746518852261009e+009)
#define RAD_2_DEG			(57.29577951308232)
#define DEG_2_RAD			(0.017453292519943)
#define AIS2BASBIS2C		(1.618033988749894)
//********************************************************************************

#endif // MATHDEFINES_H
