#ifndef CHANNEL_H
#define CHANNEL_H

#include "Includes.h"

class Channel
{
	private:

		//***********************************************************
		PipeWall_Struct aPipeWall_Struct;
		int Chan_number;
		int iSV;
		bool bActive;
		FILE *fileout;
		//***********************************************************

		// Bit grabber
		#if BIT_GRABBER
			Bit_Grabber_S BitBin;
			FILE* BitGrabberFile;
		#endif

		/* PLL Items */
		//***********************************************************
		double T;
		PLL pPLL;
		double f_carrier;

		double carrier_phase;		/* accumulated carrier phase */
		double carrier_phase_output;/* accumulated carrier phase minus ZERO_DOPPLER_RATE*/
		double frac_carrier_phase;	/* phase (0 -> 2*pi) */
		double phase_rotate;		/* rotate the phase */
		
		double freq_error;
		double phase_error;
		double w,x,z;

		short doppler_bin;
        int64 *dopp_sine;
		//***********************************************************
        //PLL Items
        double TT;

        float bb3;
        float aa3;
        float aa2;
        float ww0p;
        float ww0p2;
        float ww0p3;
        float ww0f;
        float ww0f2;
		/* DLL Items */
		//***********************************************************
		double	frac_code_phase;	/* Fractional code phase in chips */
		double	code_phase;			/* Accumulated code phase in chips */
		double	code_rate;			/* code rate */
		double	DLL_Avg;
		int		code_bin[3];		/* Partial code phase bin */
		DLL		pDLL;
		double	code_err;
		//***********************************************************


		/* Pointers to data */
		//***********************************************************
		DATA_CPX *data;
		//***********************************************************


		/* Correlations */
		//***********************************************************
		int Re[3];
		int Im[3];
		int Re2_Im2[3];
		int Re_prev;
		int Im_prev;
		int Re_prompt_avg;
		float I_avg;
		float Q_var;
		int correlations;
		int	samples_processed;
		int long_count;
		//***********************************************************

		/*For the sine/cosine waves */
		//***********************************************************
		DATA_TYPE *sine[2*CARRIER_BINS+1];
		DATA_TYPE csine[2*4*D_SAMPLES_PER_MS];

		/*Intermediate correlation steps */
		//***********************************************************
		DATA_CORR *Corr_p;
		DATA_CORR *Corr_e;
		DATA_CORR *Corr_l;
		//***********************************************************

		// Bit edge detection (for initialization of OL tracking)
		//***********************************************************
		bool bBitEdgePresent;
		int BitEdgeSampleIndex;
		double BitEdgeFracCodePhase;
		double BitEdgeFracCarrierPhase;
		double BitEdgeDoppler;
		//***********************************************************

		// Frame edge detection (for initialization of OL tracking)
		//***********************************************************
		bool bFrameEdgePresent;
		OL_Frame_Start_S FrameEdgeInfo;
		//***********************************************************

		/* Rollover Items */
		//***********************************************************
		int code_index_rollover; //not needed?
		int rollover;			 //number of samples to correlate in one code period
		//***********************************************************

		/* Sampled PRN Code */
		//***********************************************************
		DATA_TYPE *PRN_Prompt[2*CODE_BINS+1];
		//***********************************************************

		/* Bit lock stuff */
		//***********************************************************
		bool    Bit_lock;		/* Bitlock ?*/
		int		Bit_lock_ticks;	/* Number of ticks in attempted bit lock */
		int		I_sum20;		/* I prompt sum for past 20 ms */
		int		Q_sum20;		/* Q prompt sum for past 20 ms */
		int		I_buffer[20];	/* I_demod for last 20 ms */
		int		Q_buffer[20];	/* I_demod for last 20 ms */
		int		Power_buff[20];	/* Last 20 I_Sum20 */
		int		_20ms_epoch;	/* _20ms epoch */
		int		_1ms_epoch;		/* _1ms  epoch */
		int		Best_epoch;		/* Best epoch from bit synch */
		//***********************************************************

		/*Data message */
		//***********************************************************
		unsigned long Word_buff[FRAME_SIZE_PLUS_2];		/* Buffer for 12 GPS words. */
		bool	Valid_Frame[5];							/* Are these subframes now valid? */
		bool	Navigate;								/* Should we navigate on this channel? */
		int		frame_z;								/* Current z count */
		int		z_count;								/* Zc incremented by _20ms counter */
		bool    Zc_lock;								/* Is the Z count synched? */
		//***********************************************************
		
		/*Frame synch, process data bit sheit */
		//***********************************************************
		bool Frame_lock;		/* frame_lock */
		int  Bit_number;		/* ummm, the bit number in the subframe */
		int  Subframe;			/* the current subframe */
		//***********************************************************

		/* Measurements to Dump Out */
		//***********************************************************
		//***********************************************************

		// Following functions are called only by the Channel object, and not anyone else.
		Channel(); // Private constructor - don't want anyone making us without a channel number
		void doCorrelate();
		void doDumpAccum();
		void doAccumAndUpdate(int _num); // Accumulate and update all at once - more for readability than anything
		void doDLL();
		void doPLL();
		void doPRNGet();			// Generate PRN codes for this->SV
		void doBitLock();			// declare the bit lock?
		void doEpoch();				// increase _1ms_epoch, _20ms_epoch
		void doBitStuff();			// get data bits from I_Sum20 and stuff them into data_buff
		void doProcessDataBit();	// Process the data bits, how fun!
		bool doError();				// check for array out of bound errors in correlation
		void doGenSine();
		
		// Computational functions - const, don't modify the class at all, but may return/take a value that should
		bool doFrameSync(unsigned long word0, unsigned long word1) const;	// frame synch?
		bool doParityCheck(unsigned long gpsword) const;					// parity check
		bool doValidFrameFormat(unsigned long *subframe) const;	// valid frame? - also flips the subframes
		bool doIODECheck() const;			// Are all of the IODEs the same? - Returns Navigate!!

	public:
		inline bool Active() const {return bActive;}
		inline int  SV() const {return iSV;}
		void DeActive();
		// Manual kill function to shutdown a channel (useful to save computation time)
		void Kill() {printf("Channel [%2d,%2d] : Manually killed\n",Chan_number,iSV+1); bActive = false;}
		
		 Channel(int _Chan_number);
		~Channel();
		void doClearChannel();
		void doStartChannel(int _SV, double _fine_code_delay, double _f_carrier);
		void doTrack();

		// Returns true if there is a bit edge in this data set; if true, will populate all the other fields.
		bool BitEdgePresent(int *_SV, int *_bit_edge_index, double *_f_doppler, 
			double *_frac_code_phase, double *_frac_carrier_phase);
		bool FrameEdgePresent(OL_Frame_Start_S &_FrameStartInfo);

};

#endif // CHANNEL_H
