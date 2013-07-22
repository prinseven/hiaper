#ifndef OLCHANBCK_H
#define OLCHANBCK_H

#include "Includes.h"

class OLChanBck
{
	private:
		OLChanBck(); // Default constructor is private - have to initiate it with a channel number

		int ChanNumber;
		bool bActive;
		int iSV;
//		HANDLE ChanPipe;

		// Initialization values and sub-phase information for accumulations.
		double FracCodePhaseChips;
		double FracCarrierPhaseCycles;
		double CarrierPhaseCycles;

		int CycleEdgeIndex; // SAMPLE index where the next code cycle begins - zero-indexed!!
		int NumSamplesStored; // How many samples of this code cycle have we stored?
		int NumSamplesInCycle; // How many samples is this cycle in total?
		double FracSamplesInCycle; // How many fractional samples in this cycle?

		unsigned short GPS_Week; // Unambiguous GPS week number for the start of the code cycle
		unsigned long GPS_MSOW; // GPS millisecond of the week where the code cycle starts
		unsigned long GPS_MSOW_PRV;

		int DataBit; // Data bit

		unsigned short _1ms_epoch; // Counter of 1 millisecond data cycles (0-19)
		short _20ms_epoch; // Counter of 20 millisecond epochs (data bit counter)
		unsigned long z_count;		// Z counter
		unsigned short rem_z;
		unsigned long n;
		unsigned long data_bins;
		unsigned short data_bin;

		double CodeFreqHz; // Calculated code rate (from the Doppler shift on the signal)

		double FreqPredictHz; // Predicted carrier frequency

		double Cprev; // Constant term to account for phase ambiguities
		double PhiResidual; // Residual phase
		double PhiNCO; // Numerically controlled oscillator phase

		double AtanCurr;
		double AtanPrev;


		// Store the carrier that we generate.  This is int64 (4 shorts) and is also indexed by sample.
		cpx_sine *Carrier;

		// Indexed by sample, not indiv. real and imaginary!!!!!!!!!!!!!!!
		DATA_CPX *CycleBuff; // Local storage of a code cycle's worth of data
		DATA_CORR *Correlator; // Local storage of a multiply

		FILE *PredictOut;
		FILE *PredictIn;
		FILE *ClimateIn;
		FILE *DataBitIn;
		FILE *Data;

		bool act1;
		bool act2;

		bool c_e_corr;
		bool ms_corr;
		int ms_add;
		Bit_Grabber_S DataBits;
		OL_Predict_S FreqPrediction;
		OL_Climate_S FreqClimate;

		#if	(OL_CHANNEL_DEBUG > 0)
			FILE *fileout;
		#endif

		DATA_TYPE *PRN_Prompt[2*CODE_BINS+1];

		void doPRNGet();					// Generate PRN codes for this->SV
		void doCodeWipe();					// Wipe off the code from the incoming data
		big_cpx doCarrierWipe() const;		// Wipe off the carrier and accumulate the data buffer
		bool BufferFill();					// Fill as much of the cycle buffer as we can
		inline void BufferFlush() {	NumSamplesStored = 0; }	// Reset the buffer for a new code cycle
		void doCodePhase(void);					// Calculate the new fractional code phase and update CycleEdgeIndex if necessary
		void doCodeRate(double FreqBase);	// Calculate the code rate and number of samples in a cycle given a baseband Doppler frequency
		void doCarrierPhase(int NumSamples);// Calculate the fractional and whole carrier phase in cycles given a number of samples
		void doGenSine(double FrequencyHz, double PhaseOffsetCycles, cpx_sine *SineWave, int SampleLength) const;  // Given a frequency and phase offset, generate a sine-wave.

		void doEpochUpdate();				// Update the 1 and 20 millisecond epoch counters

		void doPredictIn(unsigned short _GPS_Wk, unsigned long _GPS_MSOW);  // Extract information from the input file

		inline double PhaseFromPhasor(const big_cpx &_Phasor) const
			{ return(atan2(static_cast<double>(_Phasor.i),static_cast<double>(_Phasor.r)));}
		inline double PhaseFromPhasor(const big_cpx &_Phasor, int _DataBit) const
			{ return(atan2(static_cast<double>(_Phasor.i / _DataBit),static_cast<double>(_Phasor.r / _DataBit)));}
		inline double AmpFromPhasor(const big_cpx &_Phasor) const
			{ return(sqrt(static_cast<double>(_Phasor.r)*static_cast<double>(_Phasor.r) + static_cast<double>(_Phasor.i)*static_cast<double>(_Phasor.i)));}

	public:
		inline bool Active() const {return bActive;}
		inline int  SV() const {return iSV;}

		 OLChanBck(int _Chan_number);
		~OLChanBck();
		void doClearChannel();
		void doStartChannel(const OL_Frame_Start_S &FrameInfo);
		void doTrack();
};

#endif // CHANNEL_H
