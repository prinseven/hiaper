#ifndef RECORDERDEFINES_H
#define RECORDERDEFINES_H

// Issue a warning if things are not looking good.
#ifndef RECORDER_TYPE
#error You must define the recorder type before including the RecorderDefines.h file.
#endif

// Signal stuff for Puerto Rico Data
#if PUERTO_RICO
	#define ZERO_DOPPLER_RATE		(1.405e6)	//(IF Frequency)
	#define D_SAMPLES_PER_MS		(5714)		//(Number of samples/ms, always round DOWN)
	#define D_SAMPLE_RATE			(5.71428e6)	//(Sampling rate)
	#define ACQ_RESAMPLE_RATE		(8.192e6)	//(Resample rate for the acquisition)
	#define ACQ_SAMPLES_PER_MS		(8192)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be a power of 2)
	#define CODE_RATE_OFFSET		(-2)		//(Code rate offset, property of a recording system, basically a hack)

	#define R_ZERO_DOPPLER_RATE		(1.405e6)	//(IF Frequency)
	#define R_SAMPLES_PER_MS		(5714)		//(Number of samples/ms, always round DOWN)
	#define R_SAMPLE_RATE			(5.71428e6)	//(Sampling rate)
	#define R_ACQ_RESAMPLE_RATE		(8.192e6)	//(Resample rate for the acquisition)
	#define R_ACQ_SAMPLES_PER_MS	(8192)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be 
												//a power of 2)
#endif

// Signal stuff for APL Data
#if APL_RECORDER
	#define ZERO_DOPPLER_RATE		(420000)	//(IF Frequency)
	#define D_SAMPLES_PER_MS		(2333)		//(Number of samples/ms, always round DOWN)
	#define D_SAMPLE_RATE			(2.3333e6)	//(Sampling rate)
	#define ACQ_RESAMPLE_RATE		(4.096e6)	//(Resample rate for the acquisition)
	#define ACQ_SAMPLES_PER_MS		(4096)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be a power of 2)
	#define CODE_RATE_OFFSET		(-12)		//(Code rate offset, property of a recording system, basically a hack)

	#define R_ZERO_DOPPLER_RATE		(420000)	//(IF Frequency)
	#define R_SAMPLES_PER_MS		(23333)		//(Number of samples/ms, always round DOWN)
	#define R_SAMPLE_RATE			(23.333e6)	//(Sampling rate)
	#define R_ACQ_RESAMPLE_RATE		(16.384e6)	//(Resample rate for the acquisition)
	#define R_ACQ_SAMPLES_PER_MS	(16384)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be 
												//a power of 2)
#endif

// Signal stuff for Purdue Recorder data
#if PURDUE_RECORDER
	#define ZERO_DOPPLER_RATE		(604000)	//(IF Frequency)
	#define D_SAMPLES_PER_MS		(2048)		//(Number of samples/ms, always round DOWN)
	#define D_SAMPLE_RATE			(2.048e6)	//(Sampling rate)
	#define ACQ_RESAMPLE_RATE		(2.048e6)	//(Resample rate for the acquisition)
	#define ACQ_SAMPLES_PER_MS		(2048)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be a power of 2)
	#define CODE_RATE_OFFSET		(0)			//(Code rate offset, property of a recording system, basically a hack)							

	#define R_ZERO_DOPPLER_RATE		(604000)	//(IF Frequency)
	#define R_SAMPLES_PER_MS		(2048)		//(Number of samples/ms, always round DOWN)
	#define R_SAMPLE_RATE			(2.048e6)	//(Sampling rate)
	#define R_ACQ_RESAMPLE_RATE		(2.048e6)	//(Resample rate for the acquisition)
	#define R_ACQ_SAMPLES_PER_MS	(2048)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be 
												//a power of 2)
#endif

//Signal stuff for Jill's trip
#if JILL_RECORDER
	#define ZERO_DOPPLER_RATE		(420000)	//(IF Frequency)
	#define D_SAMPLES_PER_MS		(2333)		//(Number of samples/ms, always round DOWN)
	#define D_SAMPLE_RATE			(2.3333e6)	//(Sampling rate)
	#define ACQ_RESAMPLE_RATE		(4.096e6)	//(Resample rate for the acquisition)
	#define ACQ_SAMPLES_PER_MS		(4096)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be a power of 2)
	#define CODE_RATE_OFFSET		(-14)		//(Code rate offset, property of a recording system, basically a hack)

	#define R_ZERO_DOPPLER_RATE		(420000)	//(IF Frequency)
	#define R_SAMPLES_PER_MS		(2333)		//(Number of samples/ms, always round DOWN)
	#define R_SAMPLE_RATE			(2.3333e6)	//(Sampling rate)
	#define R_ACQ_RESAMPLE_RATE		(4.096e6)	//(Resample rate for the acquisition)
	#define R_ACQ_SAMPLES_PER_MS	(4096)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be 
												//a power of 2)
#endif

//Signal stuff for the GISMOS GRS from APL
#if GISMOS_GRS
	#define ZERO_DOPPLER_RATE		(420000)	//(IF Frequency)
	#define D_SAMPLES_PER_MS		(10000)		//(Number of samples/ms, always round DOWN)
	#define D_SAMPLE_RATE			(10e6)		//(Sampling rate)
	#define ACQ_RESAMPLE_RATE		(16.384e6)	//(Resample rate for the acquisition)
	#define ACQ_SAMPLES_PER_MS		(16384)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be a power of 2)
	#define CODE_RATE_OFFSET		(0)			//(Code rate offset, property of a recording system, basically a hack)

	#define R_ZERO_DOPPLER_RATE		(420000)	//(IF Frequency)
	#define R_SAMPLES_PER_MS		(10000)		//(Number of samples/ms, always round DOWN)
	#define R_SAMPLE_RATE			(10e6)		//(Sampling rate)
	#define R_ACQ_RESAMPLE_RATE		(16.384e6)	//(Resample rate for the acquisition)
	#define R_ACQ_SAMPLES_PER_MS	(16384)		//(Resample D_SAMPLES_PER_MS into this for the acquisition, needs to be 
												//a power of 2)
#endif

#endif // GPSDEFINES_H
