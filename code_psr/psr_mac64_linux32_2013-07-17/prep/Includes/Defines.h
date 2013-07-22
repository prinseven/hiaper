#ifndef DEFINES_H
#define DEFINES_H



/*----------------------------------------------------------------------------------------------*/
typedef unsigned char       byte;
typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned long		uint32;
typedef unsigned long long	uint64;
typedef signed char			int8;
typedef signed short		int16;
typedef int			int32;
typedef signed long long	int64;
/*----------------------------------------------------------------------------------------------*/

//What stuff to log
/********************************************************************************/
#define CHANNEL_DEBUG			(1)			/* Log channel data to HD */
#define CHANNEL_INVESTIGATE		(0)			/* Try to figure out Nav bug */
#define CHANNEL_SHUT_DOWN_MSG	(1)			// Print a message when a channel dies
#define CHANNEL_BIT_LOCK_MSG	(1)         // Print a message when a channel gets bit lock
#define OL_CHANNEL_DEBUG		(0)			/* Debug level of open-loop channel data */
#define LOG_PSEUDO				(1)			/* Pseudoranges */
#define LOG_SV					(1)			/* SV Navigation data */
#define LOG_NAV					(1)			/* Nav Output */
#define	LOG_RT_ACQ				(0)			/* Log Realtime Acquisition Stats */
/********************************************************************************/

// Be a data bit grabber?
/********************************************************************************/
#define BIT_GRABBER	(1)
/********************************************************************************/

/*Channel Defines */
/********************************************************************************/
#define PREAMBLE			(0x008B)		// GPS data message preamble.
#define INVERSEPREAMBLE		(0x0074)		// Inverted preamble.
#define PHASE_SCALE			(4294967296.0)	// 2^N, ie 2^32
#define PHASE_SHIFT			(25)			// Right shift by how much?
#define CODE_BINS			(20)			// Partial code offset bins code resolution -> 1 chip/X bins
#define CARRIER_AIDING		(1)				// Carrier aid the DLL
#define CARRIER_PRESAMPLE	(1)				// Presample sine wave
#define CARRIER_BINS		(100)			// Number of pre-sampled carrier wipeoff bins
#define CARRIER_SPACING		(100)			// Spacing of bins (Hz)
#define PLL_BN				(30)			//(PLL Bandwidth)
#define FLL_BN				(30)			//(FLL Bandwidth)

/********************************************************************************/

#endif // DEFINES_H
