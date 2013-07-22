#include "Includes.h"

extern short Sine_Table[128*4];
using namespace std;
Channel::~Channel()
{
//	delete pPLL;
//	delete pDLL;

	free(Corr_e);
	free(Corr_p);
	free(Corr_l);

	#if	CHANNEL_DEBUG || CHANNEL_INVESTIGATE
	if (fileout != NULL) {
		fflush(fileout);
		fclose(fileout);
		fileout = NULL;
	}
	#endif

#if BIT_GRABBER
	if (BitGrabberFile != NULL) {
		fflush(BitGrabberFile);
		fclose(BitGrabberFile);
		BitGrabberFile = NULL;
	}
#endif
}



Channel::Channel(int _Chan_number)
{

	int lcv;

	Chan_number = _Chan_number;

	/*Get the pipe handles */
	//Chan_pipe = Write_P[Chan_number];

	/* PLL Init */
//	pPLL = new PLL;
//	pDLL = new DLL;

	//Get the pointer to this global array
	for(lcv = 0; lcv < (2*CARRIER_BINS+1); lcv++)
		sine[lcv] = &Channel_Sine[lcv][0];

	Corr_e = (DATA_CORR *)malloc(D_SAMPLES_PER_MS*sizeof(DATA_CORR));
	Corr_p = (DATA_CORR *)malloc(D_SAMPLES_PER_MS*sizeof(DATA_CORR));
	Corr_l = (DATA_CORR *)malloc(D_SAMPLES_PER_MS*sizeof(DATA_CORR));

	bActive = false;

	long_count = 0;

	bBitEdgePresent = false;
	bFrameEdgePresent = false;

#if CHANNEL_DEBUG || CHANNEL_INVESTIGATE
	fileout = NULL;
#endif

#if BIT_GRABBER
	BitGrabberFile = NULL;
#endif
}

void Channel::doClearChannel()
{
	int lcv;

	/* Clear the log file */
	//***********************************************************
	#if	CHANNEL_DEBUG || CHANNEL_INVESTIGATE
		if (fileout != NULL) { // Close the file before we open it again (prevent orphaned handles).
				fflush(fileout);
				fclose(fileout);
				fileout = NULL;
		}

		char fname[50];
		sprintf(fname,"chan%02d.dat",Chan_number);
		fileout = fopen(fname,"wc");
	#endif
	//***********************************************************

	// Close the bit grabber file, if it's there.
	#if BIT_GRABBER
		if (BitGrabberFile != NULL) {
			fflush(BitGrabberFile);
			fclose(BitGrabberFile);
			BitGrabberFile = NULL;
		}
	#endif


	/* PLL & Carrier PhaseItems */
	//***********************************************************
	memset(&pPLL, 0x0, sizeof(PLL));

	pPLL.PLLBW = PLL_BN;
	pPLL.FLLBW = FLL_BN;
	pPLL.b3 = 2.40;
	pPLL.a3 = 1.10;
	pPLL.a2 = 1.414;
	pPLL.w0p = 1.2747*(pPLL.PLLBW);
	pPLL.w0p2 = pPLL.w0p*pPLL.w0p;
	pPLL.w0p3 = pPLL.w0p2*pPLL.w0p;
	pPLL.w0f = 4*pPLL.a2*(pPLL.FLLBW)/(pPLL.a2*pPLL.a2+1.0);
	pPLL.w0f2 = pPLL.w0f*pPLL.w0f;
	pPLL.gain = .1;
	T = .001;

	bb3 = 2.40;
	aa3 = 1.10;
	aa2 = 1.414;
	ww0p = 1.2747*PLL_BN;
	ww0p2 = ww0p*ww0p;
	ww0p3 = ww0p2*ww0p;
	ww0f = 4*aa2*FLL_BN/(aa2*aa2+1.0);
	ww0f2 = ww0f*ww0f;
	TT = .001;

	f_carrier = 0;
	carrier_phase = 0;		/* accumulated carrier phase */
	carrier_phase_output = 0;  /*has ZERO_DOPPLER_RATE removed for post processing*/
	frac_carrier_phase = 0;	/* phase (0 -> 1 cycle) */
	freq_error = 0;
	phase_error = 0;
	w = 0;
	x = 0;
	z = 0;
	doppler_bin = 0;
	//***********************************************************


	/* DLL Items */
	//***********************************************************
	code_phase = 0;			/* Code delay in chips */
	frac_code_phase = 0;	/* Accumulated code delay */
	code_rate = 0;			/* code rate */
	DLL_Avg = 0;
	code_bin[0] = code_bin[1] = code_bin[2] = 0;
	memset(&pDLL,0x0,sizeof(DLL));
	code_err = 0;
	//***********************************************************


	/* Correlations */
	//***********************************************************
	for(lcv = 0; lcv < 3; lcv++)
		Re[lcv] = Im[lcv] =  Re2_Im2[lcv] = 0;

	Re_prev = 0;
	Im_prev = 0;
	Re_prompt_avg = 10000;
	correlations = 0;
	samples_processed = 0;
	I_avg = 0;
	Q_var = 0;
	//***********************************************************

	/* Bit lock stuff */
	//***********************************************************
	Bit_lock = false;						/* Bitlock ?*/
	Bit_lock_ticks = 0;						/* Number of ticks in attempted bit lock */
	I_sum20 = 0;							/* I prompt sum for past 20 ms */
	Q_sum20 = 0;							/* Q prompt sum for past 20 ms */
	memset(I_buffer,0x0,sizeof(I_buffer));
	memset(Q_buffer,0x0,sizeof(Q_buffer));
	memset(Power_buff,0x0,sizeof(Power_buff));
	_20ms_epoch = 0;						/* _20ms epoch */
	_1ms_epoch = 0;							/* _1ms  epoch */
	Best_epoch = 0;
	frame_z = 0;
	Zc_lock = false;
	z_count = 0;

	bBitEdgePresent = false;
	BitEdgeSampleIndex = 0;
	BitEdgeFracCodePhase = 0.0;
	BitEdgeFracCarrierPhase = 0.0;
	BitEdgeDoppler = 0.0;

	bFrameEdgePresent = false;
	memset(&FrameEdgeInfo,0x0,sizeof(FrameEdgeInfo));
	//***********************************************************

	/*Data message */
	//***********************************************************
	for(lcv = 0; lcv < FRAME_SIZE_PLUS_2; lcv++)
	{
		Word_buff[lcv] = 0;
		aPipeWall_Struct.Pipe_2_Ephem.Subframe[lcv] = 0;
	}

	for(lcv = 0; lcv < 5; lcv++)
	{
		Valid_Frame[lcv] = false;
	}

	Navigate = false;
	//***********************************************************

	/*Frame synch, process data bit sheit */
	//***********************************************************
	Bit_number = 0;
	Subframe = 0;
	Frame_lock = false;
	//***********************************************************
}


void Channel::doStartChannel(int _SV, double _frac_code_phase, double _f_doppler)
{
	iSV = _SV;

	/* get the PRN codes */
	doPRNGet();

	/* Carrier stuff */
	//***********************************************************
	f_carrier = _f_doppler + ZERO_DOPPLER_RATE;
	carrier_phase = 0;
	carrier_phase_output = 0;
	frac_carrier_phase = 0;
	w = z = 0;
	x = _f_doppler;
	phase_rotate  = _frac_code_phase;

	//pre-sample carrier
	doppler_bin = floor((f_carrier - ZERO_DOPPLER_RATE)/CARRIER_SPACING + 0.5) + CARRIER_BINS;
	dopp_sine = (int64 *)&sine[doppler_bin][0];
	//***********************************************************

	/* Code Stuff */
	//***********************************************************
	code_rate = CODE_RATE + CODE_RATE_OFFSET;
	frac_code_phase = _frac_code_phase;
	code_phase = _frac_code_phase;

	/* Code bin */
	code_bin[1] = floor(fmod(frac_code_phase,1.0) * CODE_BINS + 0.5) + (CODE_BINS/2);
	code_bin[0] = code_bin[1] + (CODE_BINS/2);
	code_bin[2] = code_bin[1] - (CODE_BINS/2);

	/*First calculate rollover point*/
	rollover = ceil((CODE_CHIPS - frac_code_phase)*D_SAMPLE_RATE/code_rate);

	/*Calculate index into PRN code*/
	code_index_rollover = floor(frac_code_phase*D_SAMPLE_RATE/code_rate +.5);
	//***********************************************************

	// No bit edge yet!
	bBitEdgePresent = false;
	bFrameEdgePresent = false;
	FrameEdgeInfo.SV = iSV;

	samples_processed = 0;
	bActive = true;

	// Open the bit grabber file.
	#if BIT_GRABBER
		char fname[50];
		sprintf(fname,"DataBitSV%02d.out",iSV+1);
		BitGrabberFile = fopen(fname,"wb");
	#endif

	printf("Started channel %2d, SV%02d, code phase [chips] = %f, f_D = %f\n",
		Chan_number,
		iSV+1,
		code_phase,
		_f_doppler);
}


void Channel::doTrack()
{
	int accum_length;

	/* Get the data from the FIFO as a _sample_ indexed value */
	data = master_data;

	// Quick error check...
	if(doError())
		return;
	// Clear the bit edge flag (in case the last time we tracked, we had a bit edge).
	bBitEdgePresent = false;
	bFrameEdgePresent = false;


	// Check to see where the roll over occurs (this packet, or in the next packet).
	if(rollover <= D_SAMPLES_PER_MS)
	{
		// Rollover occurs in this packet of data.

		// Accumulate and update the code and carrier phases.
		doAccumAndUpdate(rollover);

		// Dump the accumulation
		doDumpAccum();

		// If the data bit edge is now present, we need to grab the sample
		// index in the current data set.
		if (bBitEdgePresent) {
			// TODO Check all of these values!
			BitEdgeSampleIndex = rollover;
			BitEdgeFracCodePhase = frac_code_phase;
			BitEdgeFracCarrierPhase = frac_carrier_phase;
			BitEdgeDoppler = f_carrier - ZERO_DOPPLER_RATE;
		}

		if (bFrameEdgePresent) {
			FrameEdgeInfo.frame_edge_index = rollover;
			FrameEdgeInfo.frac_code_phase = frac_code_phase;
			FrameEdgeInfo.frac_carrier_phase = frac_carrier_phase;
			FrameEdgeInfo.f_doppler = f_carrier - ZERO_DOPPLER_RATE;
			FrameEdgeInfo.msow = MsecHeader.GPS_MSOW;
			FrameEdgeInfo.carrier_phase_output=carrier_phase_output;
		}

		// Where are we in the ~1ms packet of data now - note data is already equal
		// to master_data and we now use sample indexing, too.
		data += rollover;

		// First calculate the _next_ rollover point
		accum_length = D_SAMPLES_PER_MS - rollover;
		rollover = ceil((CODE_CHIPS - frac_code_phase)*D_SAMPLE_RATE/code_rate);

		// Calculate bins for code and carrier
		code_bin[0] = floor((frac_code_phase + 0.5) * CODE_BINS + 0.5) + (CODE_BINS/2);
		code_bin[1] = floor((frac_code_phase + 0.0) * CODE_BINS + 0.5) + (CODE_BINS/2);
		code_bin[2] = floor((frac_code_phase - 0.5) * CODE_BINS + 0.5) + (CODE_BINS/2);

		// Reset code index into PRN code
		code_index_rollover = 0;

		// Calculate fine carrier phase
		phase_rotate = frac_carrier_phase;

		// Find bin in carrier wipeoff
		#if CARRIER_PRESAMPLE
			doppler_bin = floor((f_carrier - ZERO_DOPPLER_RATE)/CARRIER_SPACING + 0.5) + CARRIER_BINS;
			dopp_sine = (int64 *)sine[doppler_bin];
		#else
			doGenSine();
			dopp_sine = (int64 *)&csine[0];
		#endif

		/* Zero out the correlations */
		Re[0] = Im[0] = 0;
		Re[1] = Im[1] = 0;
		Re[2] = Im[2] = 0;

		if(doError())
			return;

		// Accumulate the remaining part of this data set (either the rollover occurs in the next data set,
		// or the rollover occurs for a second time in this data set).
		doAccumAndUpdate(min(accum_length,rollover));

		// If another rollover occurs in THIS ~1ms packet of data, then handle it.
		// Otherwise, we'll just update our indices and be done.
		if(rollover < accum_length)
		{
			//Dump the accumulation
			doDumpAccum();

			// BitEdgePresent values
			if (bBitEdgePresent) {
				// TODO Check all of these values!
				BitEdgeSampleIndex = rollover + (data - master_data);
				BitEdgeFracCodePhase = frac_code_phase;
				BitEdgeFracCarrierPhase = frac_carrier_phase;
				BitEdgeDoppler = f_carrier - ZERO_DOPPLER_RATE;	// TODO check if this is really true
			}

			if (bFrameEdgePresent) {
				FrameEdgeInfo.frame_edge_index = rollover + (data - master_data);
				FrameEdgeInfo.frac_code_phase = frac_code_phase;
				FrameEdgeInfo.frac_carrier_phase = frac_carrier_phase;
				FrameEdgeInfo.f_doppler = f_carrier - ZERO_DOPPLER_RATE;
				FrameEdgeInfo.msow = MsecHeader.GPS_MSOW;
				FrameEdgeInfo.carrier_phase_output=carrier_phase_output;
			}


			/* Where are we in the ~1ms packet of data now - Note, sample indexed! */
			data += rollover;

			/* First calculate rollover point */
			accum_length -= rollover;
			rollover = ceil((CODE_CHIPS - frac_code_phase)*D_SAMPLE_RATE/code_rate);

			/* Calculate bins for code and carrier */
			code_bin[0] = floor((frac_code_phase + 0.5) * CODE_BINS + 0.5) + (CODE_BINS/2);
			code_bin[1] = floor((frac_code_phase + 0.0) * CODE_BINS + 0.5) + (CODE_BINS/2);
			code_bin[2] = floor((frac_code_phase - 0.5) * CODE_BINS + 0.5) + (CODE_BINS/2);

			/*Reset code index into PRN code*/
			code_index_rollover = 0;

			/*Calculate fine carrier phase*/
			phase_rotate = frac_carrier_phase;

			/* Find bin in carrier wipeoff */
			#if CARRIER_PRESAMPLE
				doppler_bin = floor((f_carrier - ZERO_DOPPLER_RATE)/CARRIER_SPACING + 0.5) + CARRIER_BINS;
				dopp_sine = (int64 *)sine[doppler_bin];
			#else
				doGenSine();
				dopp_sine = (int64 *)&csine[0];
			#endif

			/* Zero out the correlations */
			Re[0] = Im[0] = 0;
			Re[1] = Im[1] = 0;
			Re[2] = Im[2] = 0;

			if(doError())
				return;

			// Accumulate and update code and phase.
			doAccumAndUpdate(accum_length);
		}

		// Update the rollover point, the code index and the doppler sine wave.
		rollover -= accum_length;
		code_index_rollover += accum_length;
		dopp_sine += accum_length;
	} else {
		// Rollover occurs in next packet, so accumulate over the entire packet.
		// Accumulate and update code and phase.
		doAccumAndUpdate(D_SAMPLES_PER_MS);

		rollover -= D_SAMPLES_PER_MS;
		code_index_rollover += D_SAMPLES_PER_MS;
	}

	samples_processed += D_SAMPLES_PER_MS;
	long_count++;

    //printf("FrameEdgeInfo.msow=%d\n",FrameEdgeInfo.msow);
    //printf("FrameEdgeInfo.z_count=%d\n",FrameEdgeInfo.z_count);
	//fclose(BitGrabberFile); //NOT SO SURE IF WE ARE GOING TO DO THIS
}


void Channel::doDumpAccum()
{

	/* Rotate the correlations */
	#if CARRIER_PRESAMPLE
		double fix;
		double temp_re, temp_im;
		double sang;
		double cang;
		double ang;

		/* Calculate the additional phase offset */
		fix = PI*static_cast<double>(D_SAMPLES_PER_MS)*((((double)doppler_bin - CARRIER_BINS) * CARRIER_SPACING) + ZERO_DOPPLER_RATE - f_carrier)/D_SAMPLE_RATE;

		ang = -(fix + phase_rotate*2*PI); // negative angle
		sang = sin(ang); cang = cos(ang);

		temp_re = Re[0];
		temp_im = Im[0];

		Re[0] = (int)(cang*temp_re - sang*temp_im);
		Im[0] = (int)(sang*temp_re + cang*temp_im);


		temp_re = Re[1];
		temp_im = Im[1];

		Re[1] = (int)(cang*temp_re - sang*temp_im);
		Im[1] = (int)(sang*temp_re + cang*temp_im);


		temp_re = Re[2];
		temp_im = Im[2];

		Re[2] = (int)(cang*temp_re - sang*temp_im);
		Im[2] = (int)(sang*temp_re + cang*temp_im);

	#endif

	/* Adjust for the sine scale */
	Re[0] >>= 5;		Im[0] >>= 5;
	Re[1] >>= 5;		Im[1] >>= 5;
	Re[2] >>= 5;		Im[2] >>= 5;


	/* Power */
	Re2_Im2[0] = Re[0]*Re[0] + Im[0]*Im[0];
	Re2_Im2[1] = Re[1]*Re[1] + Im[1]*Im[1];
	Re2_Im2[2] = Re[2]*Re[2] + Im[2]*Im[2];


	/* Perform Receiver Functions */
	doDLL();
	doPLL();
	doBitLock();
	doBitStuff();
	Navigate = doIODECheck(); // returns true iff valid subframes
	doEpoch();
	correlations++;

	/* Lowpass filtered values here */
	Re_prompt_avg += (Re2_Im2[1] - Re_prompt_avg) * .01;
	I_avg += (abs((float)Re[1]) - I_avg) * .001;
	Q_var += ((float)(Im[1]*Im[1]) - Q_var) * .001;

	#if CHANNEL_DEBUG
        // phase included
        fprintf(fileout,"%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%ld\n",
z,code_rate,carrier_phase_output,Re[0],Im[0],Re[1],Im[1],Re[2],Im[2],Re_prompt_avg,long_count,rollover,z_count,MsecHeader.GPS_MSOW);
        // phase not included
        /*fprintf(fileout,"%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            z,  // CL Doppler
            code_rate,
            Re[0],
            Im[0],
            Re[1],
            Im[1],
            Re[2],
            Im[2],
            Re_prompt_avg,
            long_count,
            rollover,
            z_count,
            MsecHeader.GPS_MSOW);*/
    #endif
}

bool Channel::doError()
{
	// BDV changed - added a new define to trace some reasons for channel shutdowns.  Note
	// that if CHANNEL_SHUT_DOWN_MSG is 0, then this compiles to its original form (no preemptive
	// returns)

	// Preemptive return - if we're already dead, then just acknowledge the fact.
	if (bActive == false)
		return !bActive;

	//Check the code bins
	for(int lcv = 0; lcv < 3; lcv++)
	{
		if(code_bin[lcv] > 2*CODE_BINS+1) {
			bActive = false;
#if (CHANNEL_SHUT_DOWN_MSG > 0)
			printf("Channel [%2d,%2d]: Code bins out of range\n",Chan_number,iSV+1);
			return (!bActive);
#endif
		}

		if(code_bin[lcv] < 0) {
			bActive = false;
#if (CHANNEL_SHUT_DOWN_MSG > 0)
			printf("Channel [%2d,%2d]: Code bins out of range\n",Chan_number,iSV+1);
			return (!bActive);
#endif
		}
	}

	//Check the carrier bins
	if(doppler_bin >= 2*CARRIER_BINS+1) {
		printf("\ndoppler bin:%02d, %d\n",iSV+1,doppler_bin);
		bActive = false;
#if (CHANNEL_SHUT_DOWN_MSG > 0)
		printf("Channel [%2d,%2d]: Carrier bins out of range\n",Chan_number,iSV+1);
		return (!bActive);
#endif
	}

	if(doppler_bin < 0) {
		printf("\ndoppler bin:%02d, %d\n",iSV+1,doppler_bin);
		bActive = false;
#if (CHANNEL_SHUT_DOWN_MSG > 0)
		printf("Channel [%2d,%2d]: Carrier bins out of range\n",Chan_number,iSV+1);
		return (!bActive);
#endif
	}

	// Kill channel if lost lock
	if ( (correlations > 2000) && (Re_prompt_avg < 5000) ) {
		bActive = false;
#if (CHANNEL_SHUT_DOWN_MSG > 0)
		printf("Channel [%2d,%2d]: Lost or never achieved lock\n",Chan_number,iSV+1);
		return (!bActive);
#endif
	}

	// Kick the channel off if frame synch hasnt occured in 20 seconds
	// HACK changed to 30 by tdl 27jan09y
	if ( (iSV < L1_NUM_SVS) && (correlations > 30000) && (Frame_lock == false) ) {
		bActive = false;
#if (CHANNEL_SHUT_DOWN_MSG > 0)
		printf("Channel [%2d,%2d]: Frame sync didn't occur in 30 seconds\n",Chan_number,iSV+1);
		return (!bActive);
#endif
	}

	return(!bActive);
}



void Channel::doGenSine()
{
	// Entirely in cycles, no radians!
	unsigned acc_phase = (unsigned int) (frac_carrier_phase*PHASE_SCALE);
	unsigned phase_step = (unsigned int) (f_carrier*PHASE_SCALE/D_SAMPLE_RATE);
	int index;
	int64 *pSine_Table  = (int64 *)&Sine_Table[0]; //for the sine/cosine lookup
	int64 *psine        = (int64 *)&csine[0];

	/* Generate the sine/cosine */
	for(int lcv = 0; lcv < 2*D_SAMPLES_PER_MS; lcv++)
	{
		index = (acc_phase >> PHASE_SHIFT);
		psine[lcv] = pSine_Table[index];
		acc_phase += phase_step;
	}

}


void Channel::doAccumAndUpdate(int _num)
{
	// Accumulate the data.
	int64 corr_temp;

	//	Multiply the PRN code by the PRN code (early, prompt, and late).

	//sse_mulc(&PRN_Prompt[code_bin[0]][2*code_index_rollover], data, &Corr_e[0], 2*_num);
	//sse_mulc(&PRN_Prompt[code_bin[1]][2*code_index_rollover], data, &Corr_p[0], 2*_num);
	//sse_mulc(&PRN_Prompt[code_bin[2]][2*code_index_rollover], data, &Corr_l[0], 2*_num);

	x86_cmuln(&PRN_Prompt[code_bin[0]][2*code_index_rollover], data, &Corr_e[0], 2*_num);
	x86_cmuln(&PRN_Prompt[code_bin[1]][2*code_index_rollover], data, &Corr_p[0], 2*_num);
	x86_cmuln(&PRN_Prompt[code_bin[2]][2*code_index_rollover], data, &Corr_l[0], 2*_num);

	// Multiply the code-wiped versions by the carrier and accumulate.
	corr_temp = sse_cacc(&Corr_e[0], dopp_sine, _num);
	Re[0] += (int) (corr_temp & 0xffffffff);
	Im[0] += (int) (corr_temp >> 32);
	corr_temp = sse_cacc(&Corr_p[0], dopp_sine, _num);
	Re[1] += (int) (corr_temp & 0xffffffff);
	Im[1] += (int) (corr_temp >> 32);
	corr_temp = sse_cacc(&Corr_l[0], dopp_sine, _num);
	Re[2] += (int) (corr_temp & 0xffffffff);
	Im[2] += (int) (corr_temp >> 32);



	// Update the carrier phase based on the number of samples we just accumulated on.
//	carrier_phase += fmod(_num, D_SAMPLE_RATE/f_carrier)*f_carrier/D_SAMPLE_RATE;  // GH via TDL 09.Nov.09
	carrier_phase += (double)_num*f_carrier/D_SAMPLE_RATE; // GH via TDL 09.Nov.09
	carrier_phase_output += (double)_num*(f_carrier-ZERO_DOPPLER_RATE)/D_SAMPLE_RATE;
	frac_carrier_phase = fmod(carrier_phase, 1.0);

	// Update the code phase based on the number of samples we just accumulated on.
	code_phase		+= (double)_num * code_rate / (D_SAMPLE_RATE);
	frac_code_phase += (double)_num * code_rate / (D_SAMPLE_RATE);
	frac_code_phase = fmod(frac_code_phase, CODE_CHIPS);
}

void Channel::doDLL()
{
	#if CARRIER_AIDING
		code_err = double(Re2_Im2[0] - Re2_Im2[2]) * .0001 * (2048.0/D_SAMPLES_PER_MS);

		if(correlations < 5000)
			DLL_Avg = code_err;
		else
			DLL_Avg += .1*(code_err - DLL_Avg);

		//code and carrier aiding
		code_rate = CODE_RATE + ((f_carrier - ZERO_DOPPLER_RATE) * CODE_RATE / L1_FREQ_HZ) + 0.0 + DLL_Avg;
	#else
		code_err += static_cast<float>(Re2_Im2[0] - Re2_Im2[2]) / static_cast<float>(Re2_Im2[0] + Re2_Im2[2]);

		if((correlations % 4) == 0x0) {
			pDLL->Code_D2 = code_err * .1;
			pDLL->Code_Delta = (pDLL->Code_D2)*0.5 + (pDLL->Code_D2 - pDLL->Code_D1)*10.0;
			pDLL->Code_D1 = pDLL->Code_D2;
			code_rate += pDLL->Code_Delta;
			code_err = 0;
		}
	#endif
}

void Channel::doPLL()
{
	// Use frequency error to start with, then move to phase error.
	if(correlations < 3000) {
		// Frequency error
		freq_error = (double)(Im[1]*Re_prev - Re[1]*Im_prev);
		freq_error *= .0005; // 2*pi / integration length

		Re_prev = Re[1];
		Im_prev = Im[1];
		phase_error = 0;
	} else {
		// Phase error
		if(Re[1] == 0)
			phase_error = 0;
		else
			phase_error =  .1 * atan((double)Im[1]/(double)Re[1]);

		freq_error  = 0;
	}

	// Control loop (both frequency and phase error at the same time.
	// Items of the form q = q + T (...) are discrete integrators
	// w = acceleration accumulator
	// x = velocity accumulator

	w = w + T * (pPLL.w0p3 * phase_error + pPLL.w0f2 * freq_error);
	x = x +	0.5*T* (w  + pPLL.a2*pPLL.w0f * freq_error + pPLL.a3*pPLL.w0p2 * phase_error);  // added 0.5 to T * (...) TDL 09.Nov.09
	z = x + pPLL.b3 * pPLL.w0p * phase_error;
	f_carrier = ZERO_DOPPLER_RATE + z;

}

void Channel::doBitLock()
{
	//Change the discriminator
	I_sum20 += Re[1] - I_buffer[_1ms_epoch];	//Last 20 Correlations
	Q_sum20 += Im[1] - Q_buffer[_1ms_epoch];	//Last 20 Correlations

	//Always do this stuff
	I_buffer[_1ms_epoch] = Re[1];				//Inphase Prompt Correlation
	Q_buffer[_1ms_epoch] = Im[1];

	if(Bit_lock == false)
	{

		if(Bit_lock_ticks >  0)	//Wait for PLL and phase lock
		{
			Power_buff[_1ms_epoch] = (63*Power_buff[_1ms_epoch] +
			 (I_sum20>>6)*(I_sum20>>6) + (Q_sum20>>6)*(Q_sum20>>6)) >> 6;
		}

		if((Bit_lock_ticks >= 4019) & (_1ms_epoch == 0))		//let Power_buff fill for 4 seconds
		{
			//Find the maximum (best epoch)
			int sum = 0;
			for(int lcv = 0; lcv < 20; lcv++)
			{
				if(Power_buff[lcv] > sum)
				{
					sum = Power_buff[lcv];
					Best_epoch = lcv;
				}
			}

			/* Got bit lock */
			_1ms_epoch = (19 - Best_epoch) % 20;
			Bit_lock = true;

			/* Clear in case we fall out of bit lock */
			memset(Power_buff,0x0,sizeof(Power_buff));
			Bit_lock_ticks = 0;
		}

		Bit_lock_ticks++;
	}
}

void Channel::doBitStuff()
{
	unsigned int temp_bit;
    short int dataword;
    short int feedbit;

	// Clear the bit edge flag (we set it below if it's true.)
	bBitEdgePresent = false;
	bFrameEdgePresent = false;

	if(Bit_lock && (_1ms_epoch == 19))
	{
		if(I_sum20 > 0)
			temp_bit = 1;
		else
			temp_bit = 0;

		// A new data bit starts in this data set, so raise the flag.
		bBitEdgePresent = true;

		/*now add the bit into the buffer */
		/*Each word in wordbuff is composed of:
		Bits 0 to 29 = the GPS data word
		Bits 30 to 31 = 2 LSBs of the GPS word ahead. */
		for (dataword = 0; dataword <= (FRAME_SIZE_PLUS_2-2); dataword++)
		{
			/* Shift the data word 1 place to the left. */
			Word_buff[dataword] <<= 1;

			/* Add the MSB of the following GPS word (bit 29 of a wordbuff word). */
			feedbit = (short)(Word_buff[dataword+1]>>29)&0x00000001;
			Word_buff[dataword] += feedbit;
		}

		/* The bit added for the 12th word is the new data bit. */
		Word_buff[FRAME_SIZE_PLUS_2-1] <<= 1;
		Word_buff[FRAME_SIZE_PLUS_2-1] += temp_bit;

		doProcessDataBit();

		// HACK - frame edge occurrence
		// Because we require the next 60 data bits of a subframe to establish sequential
		// subframe IDs and accurate preamble/zerobits, _20ms_epoch is synchronous not to
		// the _real_ zcount but to when the buffer has enough data.
		// Therefore, we actually have a frame edge occuring when we hit
		// _20ms_epoch = 240 and _1ms_epoch = 19.
		if (Zc_lock && _20ms_epoch == 240) {
			// Frame edge occurs RIGHT NOW (where this data bit ends)!
			bFrameEdgePresent = true;
			FrameEdgeInfo.sid = (Subframe + 1) % L1_D_NUM_SUBFRAMES;
			FrameEdgeInfo.z_count = (z_count + 4) % L1_D_MAX_Z_COUNT;
		}
	}
}


void Channel::doProcessDataBit()
{
	short int sid;
	short int lcv, lcv2;
    unsigned long word0, word1;

	// Clear the frame edge flag (we set it below if it's true.
	bFrameEdgePresent = false;

	if(Frame_lock) {
		// Have frame lock, so update the number of bits we have collected in this subframe.
		Bit_number = (Bit_number + 1) % 300;

		// Complete subframe is now available, so operate on it.
		if (Bit_number == 0) {
			//Copy the subframe over
			for(lcv = 0; lcv < FRAME_SIZE_PLUS_2; lcv++)
				aPipeWall_Struct.Pipe_2_Ephem.Subframe[lcv] = Word_buff[lcv];

			// Check the frame, and if it looks ok, flip the appropriate bits and prepare to use it.
			// Otherwise, have a panic attack.
			if(doValidFrameFormat(aPipeWall_Struct.Pipe_2_Ephem.Subframe)) {
				// Valid subframe!
				sid = (short int)((aPipeWall_Struct.Pipe_2_Ephem.Subframe[1] >> 8) & 0x00000007);
				frame_z =  4 *  ((aPipeWall_Struct.Pipe_2_Ephem.Subframe[1] >> 13) & 0x0001FFFF);

				if(!Zc_lock) {
					z_count = frame_z;
					Zc_lock = true;
				}

				Subframe = sid;
				Valid_Frame[sid-1] = true;
				aPipeWall_Struct.Pipe_2_Ephem.Subframe_num = Subframe;

#if BIT_GRABBER
				// BIT GRABBER - use the fact that we have a valid subframe to perform "bit grabber" functions
				// Go back to Word_buff to get the entire subframe, unflipped.
				// Write it out to a file (in transmission order, 0 -> -1, 1 -> +1).
				unsigned long curr_word;
				unsigned short curr_bit;
				// frame_z (extracted from the subframe itself) corresponds to the transmission edge of the _next_ subframe
				if (frame_z < 4) {
					BitBin.Z_count = L1_D_MAX_Z_COUNT - 4; // Next subframe starts at the begining of the week, so this is the last subframe of the week
				} else {
					BitBin.Z_count =  frame_z - 4; // This subframe is for the previous z_count, not the one we decoded
				}
				for (lcv = 0; lcv < FRAME_SIZE_PLUS_2-2; lcv++) {
					// Extract the current word from the word buffer.
					curr_word = Word_buff[lcv];

					// Bits 0 to 29 = the GPS data word, bits 30 to 31 = 2 LSBs of the GPS word ahead.
					// Bit 29 holds the first bit transmitted in time.
					for (lcv2 = 0; lcv2 < NUM_BITS_PER_DATA_WORD; lcv2++) {
						curr_bit = static_cast<unsigned short>(curr_word >> 29) & 0x1;
						BitBin.data_bit[lcv*NUM_BITS_PER_DATA_WORD + lcv2] = (curr_bit == 1 ? 1 : -1);
						//BitBin.data_bit[lcv*NUM_BITS_PER_DATA_WORD + lcv2] = (curr_bit == 1 ? -1 : 1);
						curr_word <<= 1;
					}
				}
				// Flush BitBin to disk.
				fwrite(&BitBin,sizeof(BitBin),1,BitGrabberFile);
#endif // BIT_GRABBER
				// End valid subframe
			} else {
				// Start invalid subframe
				Subframe = 0;
				Valid_Frame[0] = Valid_Frame[1] = Valid_Frame[2] = Valid_Frame[3] = Valid_Frame[4] = false;
				frame_z = 0;
				z_count = 0;
				Navigate = false;
				Frame_lock = false;
				Bit_lock = false;
				Zc_lock = false;
				bBitEdgePresent = false;
				memset(Word_buff,0x0,sizeof(Word_buff));
#if (CHANNEL_BIT_LOCK_MSG > 0)
				printf("Channel [%2d,%2d]: Lost bit lock with invalid subframe!\n",Chan_number,iSV+1);
#endif
				// End invalid subframe
			}
		}
		// End have frame_lock.
	} else {
		// No frame lock, so attempt frame sync.
		word0 = Word_buff[FRAME_SIZE_PLUS_2-2];
		word1 = Word_buff[FRAME_SIZE_PLUS_2-1];

		if(doFrameSync(word0, word1)) {
			// We now have frame lock, but we've already collected 60 data bits so far.
			// For legacy reasons, _20ms_epoch and Bit_number = 0, even though we've
			// actually just marked bit 60.  Go figure.
			Frame_lock = true;
			Bit_number = 0;
			_20ms_epoch = 0;
#if (CHANNEL_BIT_LOCK_MSG > 0)
			printf("Channel [%2d,%2d]: Frame lock\n",Chan_number,iSV+1);
#endif
		}
		// End attempting frame sync
	}
}

void Channel::doEpoch()
{
	_1ms_epoch++;

	//Rollover on bit
	if(_1ms_epoch == 20) {
		_1ms_epoch = 0;

		_20ms_epoch++;

		//Rollover on frame
		if(_20ms_epoch == 300) {
			_20ms_epoch = 0;
			z_count += 4;
		}
	}
}



bool Channel::doFrameSync(unsigned long word0, unsigned long word1) const
{
    short int preamble;	/* The TLM word preamble sequence 10001011. */
    short int sid;		/* The subframe ID (1 to 5). */
    short int zerobits;	/* The zero bits (the last 2 bits of word 2). */

    /* Extract the preamble. */
    preamble = static_cast<short int>((word0>>22) & 0x000000FF);

    /* Extract the subframe ID. */
    sid = static_cast<short int>((word1>>8) & 0x00000007);

    /* Extract the zero bits. */
    zerobits = static_cast<short int>(word1 & 0x00000003);

    /* Invert the extracted data according to bit 30 of the previous word. */
    if (word0 & 0x40000000) {
        preamble ^= 0x000000FF;
        zerobits ^= 0x00000003;
    }
    if (word1 & 0x40000000)
        sid ^= 0x00000007;

    /* Check that the preamble, subframe and zero bits are ok. */
    if (preamble != PREAMBLE)
        return false;

    /* Check if the subframe ID is ok. */
    if (sid < 1 || sid > 5)
        return false;

    /* Check that the zero bits are ok. */
    if (zerobits!=0)
        return false;

    /* Check that the 2 most recently logged words pass parity. Have to first
       invert the data bits according to bit 30 of the previous word. */
    if (word0 & 0x40000000)
        word0 ^= 0x3FFFFFC0;

    if (word1 & 0x40000000)
        word1 ^= 0x3FFFFFC0;

    if ( !this->doParityCheck(word0) || !this->doParityCheck(word1))
        return false;

    return true;
}



bool Channel::doParityCheck(unsigned long gpsword) const
{
    unsigned long d1,d2,d3,d4,d5,d6,d7,t,parity;

    /* XOR as many bits in parallel as possible.  The magic constants pick
       up bits which are to be XOR'ed together to implement the GPS parity
       check algorithm described in ICD-GPS-200.  This avoids lengthy shift-
       and-xor loops. */

    d1 = gpsword & 0xFBFFBF00;
    d2 = _lrotl(gpsword,1) & 0x07FFBF01;
    d3 = _lrotl(gpsword,2) & 0xFC0F8100;
    d4 = _lrotl(gpsword,3) & 0xF81FFE02;
    d5 = _lrotl(gpsword,4) & 0xFC00000E;
    d6 = _lrotl(gpsword,5) & 0x07F00001;
    d7 = _lrotl(gpsword,6) & 0x00003000;

    t = d1 ^ d2 ^ d3 ^ d4 ^ d5 ^ d6 ^ d7;

    /* Now XOR the 5 6-bit fields together to produce the 6-bit final result. */
    parity = t ^ _lrotl(t,6) ^ _lrotl(t,12) ^ _lrotl(t,18) ^ _lrotl(t,24);
    parity = parity & 0x3F;
    if (parity == (gpsword&0x3F))
        return(true);
    else
        return(false);
}


bool Channel::doValidFrameFormat(unsigned long *subframe) const
{
	// If the array pointed to by subframe passes the checks, this function will
	// return true as well as flip the appropriate subframes according to the data bits.
	// If the subframes are poorly formatted/bad data, then this will return false without
	// touching the data.

	short int preamble,next_preamble;
    short int zerobits,next_zerobits;
    short int sid,next_sid;
    short int wordcounter;
    short int parity_errors;

    /* Extract the preamble. */
    preamble = static_cast<short int>((subframe[0]>>22)&0x000000FF);

    /* Extract the subframe ID. */
    sid = static_cast<short int>((subframe[1]>>8)&0x00000007);

    /* Extract the zero bits. */
    zerobits = static_cast<short int>(subframe[1]&0x00000003);

    /* Invert the extracted data according to bit 30 of the previous word. */
	if (subframe[0]&0x40000000) {
        preamble ^= 0x000000FF;
        zerobits ^= 0x00000003;
	}
    if (subframe[1]&0x40000000)
        sid ^= 0x00000007;

    /* Check that the preamble, subframe and zero bits are ok. */
    if (preamble!=PREAMBLE)
        return(false);

    if (sid<1 || sid>5)
        return(false);

    if (zerobits!=0)
        return(false);

    /* Extract the next preamble (10001011). */
    next_preamble = static_cast<short int>((subframe[FRAME_SIZE_PLUS_2-2]>>22)&0x000000FF);

    /* Extract the next subframe ID. */
    next_sid = static_cast<short int>((subframe[FRAME_SIZE_PLUS_2-1]>>8)&0x00000007);

    /* Extract the next zero bits. */
    next_zerobits = static_cast<short int>((subframe[FRAME_SIZE_PLUS_2-1]&0x00000003));

    /* Invert the extracted data according to bit 30 of the previous word. */
    if(subframe[FRAME_SIZE_PLUS_2-2]&0x40000000) {
        next_preamble ^= 0x000000FF;
        next_zerobits ^= 0x00000003;
    }
    if (subframe[FRAME_SIZE_PLUS_2-1]&0x40000000)
        next_sid ^= 0x00000007;

    /* Check that the preamble, subframe and zero bits are ok. */
    if (next_preamble!=PREAMBLE)
        return(false);

    if (next_sid<1 || next_sid>5)
        return(false);

    if (next_zerobits!=0)
        return(false);

    /* Check that the subframe IDs are consistent. */

    if ((next_sid-sid)!=1 && (next_sid-sid)!=-4)
        return(false);

    /* Check that all 12 words have correct parity. Have to first
       invert the data bits according to bit 30 of the previous word. */
    parity_errors = 0;

	for (wordcounter=0; wordcounter<FRAME_SIZE_PLUS_2; wordcounter++)
		if (subframe[wordcounter]&0x40000000)
			subframe[wordcounter] ^= 0x3FFFFFC0;

    for (wordcounter=0; wordcounter<FRAME_SIZE_PLUS_2; wordcounter++)
		if (!this->doParityCheck(subframe[wordcounter]))
			parity_errors++;

    if(parity_errors!=0)
        return(false);

	// BDV removed setting of Subframe - was immediately done anyway in calling function.
    return(true);
}


bool Channel::doIODECheck() const
{
	// Returns:
	//  1. False if the frames are bad.
	//  2. True (which you should set Navigate equal to) if the frames are OK.

	if((Valid_Frame[0] == true) && (Valid_Frame[1] == true) && (Valid_Frame[2] == true))
		return(true);
	else
		return(false);
}


void Channel::doPRNGet()
{
	for(int lcv = 0; lcv < (2*CODE_BINS+1); lcv++)
		PRN_Prompt[lcv] = PRN_Sampled[iSV][lcv];
}

bool Channel::BitEdgePresent(int *_SV, int *_bit_edge_index, double *_f_doppler, double *_frac_code_phase, double *_frac_carrier_phase)
{
	// Returns false if we either don't have bit lock yet or the bit edge doesn't occur in this data set.
	if (Bit_lock == false)
		return false;
	if (bBitEdgePresent == false)
		return false;

	// No error checking is performed on the input pointers!
	*_SV = iSV;
	*_bit_edge_index = BitEdgeSampleIndex;
	*_f_doppler = BitEdgeDoppler;
	*_frac_code_phase = BitEdgeFracCodePhase;
	*_frac_carrier_phase = BitEdgeFracCarrierPhase;

	return true;
}

bool Channel::FrameEdgePresent(OL_Frame_Start_S &_FrameStartInfo)
{
	if(gopt.forward){
		if (bFrameEdgePresent == false){
			_FrameStartInfo = FrameEdgeInfo;
			return false;
		}
		else{
			_FrameStartInfo = FrameEdgeInfo;

			return true;
		}

	}
	else{
		_FrameStartInfo = FrameEdgeInfo;
		return true;
	}


	// Populate the input structure (which is passed by reference!)

}

void Channel::DeActive()
{
	bActive=false;
}
