#include "OLChannel.h"
using namespace std;
extern short Sine_Table[128*4];

OLChannel::~OLChannel()
{
	// Free our data storage.
	free(CycleBuff);
	free(Correlator);
	free(Carrier);

	// Close the debug file if necessary.
#if	(OL_CHANNEL_DEBUG > 0)
	if (fileout != NULL) {
		fclose(fileout);
		fileout = NULL;
	}
#endif

	// Close the input/output files if necessary.
	if (PredictOut != NULL) {
		fclose(PredictOut);
		PredictOut = NULL;
	}
	if (PredictIn != NULL) {
		fclose(PredictIn);
		PredictIn = NULL;
	}
	if (ClimateIn != NULL) {
		fclose(ClimateIn);
		ClimateIn = NULL;
	}
	if (DataBitIn != NULL) {
		fclose(DataBitIn);
		DataBitIn = NULL;
	}
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/

OLChannel::OLChannel(int _Chan_number) :
ChanNumber(_Chan_number)
{
	const int CYCLE_LENGTH_FACTOR = 2;

	// Malloc space for one "cycle" worth of data.
	// Note: longer than a single code cycle because of Doppler (factor of 2 to be _really_ safe).
	CycleBuff = (DATA_CPX *)malloc(D_SAMPLES_PER_MS*CYCLE_LENGTH_FACTOR*sizeof(DATA_CPX));

	// Also need a buffer to hold our correlated multiplies.
	Correlator = (DATA_CORR *)malloc(D_SAMPLES_PER_MS*CYCLE_LENGTH_FACTOR*sizeof(DATA_CORR));

	// Need a buffer to hold the carrier that we generate.
	Carrier = (cpx_sine *)malloc(D_SAMPLES_PER_MS*CYCLE_LENGTH_FACTOR*sizeof(cpx_sine));

	// Set our file pointers to NULL.
	PredictIn = NULL;
	ClimateIn = NULL;
	PredictOut = NULL;
	DataBitIn = NULL;

#if (OL_CHANNEL_DEBUG > 0)
	fileout = NULL;
#endif

	// Let doClearChannel do the rest of the initialization.
	doClearChannel();
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doClearChannel()
{
	// Only very simple checks take place here.
	// Most initialization work happens in doStartChannel().
	bActive = false;
	iSV = -1;

	// Close the input and output files if necessary.
	if (PredictOut != NULL) {
		fclose(PredictOut);
		PredictOut = NULL;
	}
	if (PredictIn != NULL) {
		fclose(PredictIn);
		PredictIn = NULL;
	}
	if (ClimateIn != NULL) {
		fclose(ClimateIn);
		ClimateIn = NULL;
	}
	if (DataBitIn != NULL) {
		fclose(DataBitIn);
		DataBitIn = NULL;
	}

	// Clear the log file (close it if it already exists).
#if	(OL_CHANNEL_DEBUG > 0)
	if (fileout != NULL) {
		fclose(fileout);
		fileout = NULL;
	}
	char fname[50];
	sprintf(fname,"OLchan%02d.dat",ChanNumber);
	fileout = fopen(fname,"wc");
#endif
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doStartChannel(const OL_Frame_Start_S &FrameInfo)
{
	iSV = FrameInfo.SV;

	// Get the PRN code for this satellite.
	doPRNGet();

	// Store the cycle edge index and set our stored amount to zero.
	CycleEdgeIndex = FrameInfo.frame_edge_index;
	BufferFlush();

	Cprev = 0.0;

	PhiResidual = 0.0;
	PhiNCO = 0.0;

	AtanPrev = 0.0;

	CarrierPhaseCycles = 0.0;
	FracCarrierPhaseCycles = 0.0;

	FracCodePhaseChips = FrameInfo.frac_code_phase;

	// Reset epoch counters.
	_1ms_epoch = 0;
	_20ms_epoch = 0;
	z_count = FrameInfo.z_count;

	// Open the appropriate input/output files.
	char fname[50];

	sprintf(fname,"OLPredictSV%02ds.in",iSV+1);
	//printf("test:%s\n",fname);
	PredictIn = fopen(fname,"rb");
	if (PredictIn == NULL) {
		printf("OL channel [%2d,%2d] couldn't find %s!\n",ChanNumber,iSV+1,fname);
		return;
	}

	sprintf(fname,"ClimateSV%02ds.in",iSV+1);
	ClimateIn = fopen(fname,"rb");
	if (ClimateIn == NULL) {
		printf("OL channel [%2d,%2d] couldn't find %s!\n",ChanNumber,iSV+1,fname);
		printf("No Climatological Model Included!\n");
	}
	else
	{
	    printf("The Climatological Model is Used\n");
	}

	sprintf(fname,"DataBitSV%02d.in",iSV+1);
	DataBitIn = fopen(fname,"rb");
	if (DataBitIn == NULL) {
		printf("OL channel [%2d,%2d] couldn't find %s!\n",ChanNumber,iSV+1,fname);
		return;
	}

	sprintf(fname,"OLPredictSV%02ds.out",iSV+1);
	PredictOut = fopen(fname,"wc");
	if (PredictOut == NULL) {
		printf("OL channel [%2d,%2d] couldn't make %s!\n",ChanNumber,iSV+1,fname);
		return;
	}
	else
	{
	    printf("OLpredict.out generated!\n");
	}

	// Grab the current GPS_Week and GPS_MSOW out of the global header
	// that goes along with the data stream.
	GPS_Week = MsecHeader.GPS_Week;
	GPS_MSOW = MsecHeader.GPS_MSOW;

	// Reset the prediction storage.
	FreqPrediction.GPS_Week = 0;
	FreqPrediction.GPS_SOW = 0;
	FreqPredictHz = 0;
	DataBits.Z_count = 0;

	// Grab the current data bit and predicted Doppler (for this millisecond).
	doPredictIn(GPS_Week, GPS_MSOW);

	// Calculate the current code rate and the number of samples in this bit.
	doCodeRate(FreqPredictHz);

	// Print out that we started.
	printf("OL channel %2d, SV%02d, Edgei = %d, Frac. code phase [chips] = %f, F_model = %f\n",
		ChanNumber, iSV+1, CycleEdgeIndex, FracCodePhaseChips, FreqPredictHz);

	// Compare the predicted Doppler with the Doppler from CL tracking
	// that we were initialized with.
	// Print a message letting us know that something went wrong, too.
	double FreqError = FreqPredictHz - FrameInfo.f_doppler;
	if (abs(FreqError) > 10) {
		printf("FreqError=%f***\n",FreqError);
	} else {
		printf("FreqError=%f\n",FreqError);
	}

	// Go active.
	bActive = true;
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doPredictIn(unsigned short _GPS_Wk, unsigned long _GPS_MSOW)
{

	// While we aren't matching the input GPS_SOW and we haven't hit the end of file
	if (ClimateIn == NULL) {
        while (!feof(PredictIn) && _GPS_MSOW/1000 > FreqPrediction.GPS_SOW) {
            // Read a block of the input file.
            fread(&FreqPrediction,sizeof(OL_Predict_S),1,PredictIn);
        }
	}
	else
	{
	    while (!feof(PredictIn) && _GPS_MSOW/1000 > FreqPrediction.GPS_SOW) {
            // Read a block of the input file.
            fread(&FreqPrediction,sizeof(OL_Predict_S),1,PredictIn);
            //fread(&FreqClimate,sizeof(OL_Climate_S),1,ClimateIn);
        }
        while (!feof(ClimateIn) && _GPS_MSOW/1000 > FreqClimate.GPS_SOW) {
            // Read a block of the input file.
            //fread(&FreqPrediction,sizeof(OL_Predict_S),1,PredictIn);
            fread(&FreqClimate,sizeof(OL_Climate_S),1,ClimateIn);
        }
	}

	// If we've hit the end of a file, let's be sure we know about it!
	if (feof(PredictIn)) {
		printf("OLChannel [%2d,%2d]: Predictor file ended, killing OL channel!\n",ChanNumber,iSV+1);
		bActive = false;
	}

	// Check to see if our logic is working correctly.
	if (_GPS_Wk == FreqPrediction.GPS_Week && _GPS_MSOW/1000 == FreqPrediction.GPS_SOW) {
        // Set FreqPredictHz from the input.
		// Otherwise, we just let FreqPredictHz coast with its last value.
		if(ClimateIn == NULL)
		{
		    FreqPredictHz = FreqPrediction.FreqPredictHz[_GPS_MSOW % 1000];
		}
		else
		{
		    FreqPredictHz = FreqPrediction.FreqPredictHz[_GPS_MSOW % 1000] + FreqClimate.FreqClimateHz[_GPS_MSOW % 1000];
		}

	}

	//---------------------------------------------------------
	// Data bit
	// HACK Read the data bits - if we haven't read in the current value
	while (!feof(DataBitIn) && z_count > DataBits.Z_count) {
        // Read a line of the input file.
		fread(&DataBits,sizeof(Bit_Grabber_S),1,DataBitIn);

		// HACK - Check to see if we have the right set of data bits.
		// Check the z_count contained in the 2nd 10-bit word of the data
		// bits.
		short tempbit;
		unsigned long frame_z = 0;
		for (int ii = 0; ii < 17; ii++) {
			// Invert according to bit 30 of the previous word.
			tempbit = -DataBits.data_bit[29]*DataBits.data_bit[30 + ii];

			// Convert to 0 or 1.
			tempbit = (tempbit + 1)/2;

			// Shift and add our new data bit.
			frame_z <<= 1;
			frame_z += tempbit;
		}
		frame_z *= 4;

		if (frame_z - 4 != DataBits.Z_count) {
			printf("OLChannel [%2d,%2d] : Data bit prediction has wrong data bits in it!\n",ChanNumber,iSV+1);
		}
	}

	// If we've hit the end of a file, let's be sure we know about it!
	if (feof(DataBitIn)) {
		printf("OLChannel [%2d,%2d]: Data bit predictor file ended, killing OL channel!\n",ChanNumber,iSV+1);
		bActive = false;
	}

	// Extract the right data bit.
	DataBit = DataBits.data_bit[_20ms_epoch];
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doTrack()
{
	// Entry point for the OL tracking.
	double IntegTime;
	double TotalPhase;
	big_cpx Phasor;

	// Fill our buffer (using BufferFill()) with data.  If we now have a full
	// buffer, then do our operations on it.  Otherwise, return immediately.
	// This is a loop because we will have some leftover data that's part of
	// the next code cycle.
	while (BufferFill()) {
		// Operate on a full code cycle worth of data.

// CORRELATION SUM DETERMINATION
		// Wipe code off first.
		doCodeWipe();

		// Generate a sine wave with the appropriate frequency and phase offset.
		doGenSine(FreqPredictHz+ZERO_DOPPLER_RATE,FracCarrierPhaseCycles,Carrier,NumSamplesStored);

		// Dump and accumulate the buffer.
		Phasor = doCarrierWipe();

// OUTPUT VALUES TO A FILE
		// Calculate the total phase (using values computed for the last step!).
		TotalPhase = PhiNCO + PhiResidual;

		//printf("ready to write OLPredict!");

		// Output file comes first because of the way all of the equations are defined
		// (they all rely on the information from the PREVIOUS code cycle).
		fprintf(PredictOut,"%d,%lu,%lu,%f,%d,%d,%f,%f,%d,%f",
			GPS_Week,
			GPS_MSOW,
			MsecHeader.GPS_MSOW - 1, // HACK:  because the pointer was moved to the next MSEC and we want the current MSEC
			FreqPredictHz,
			Phasor.r,
			Phasor.i,
			PhiResidual,
			PhiNCO,
			DataBit,
			NumSamplesInCycle+FracSamplesInCycle);

// OL DATA PRODUCT PRODUCTION
		// Calculate the total integration time for this data bit.
		IntegTime = L1_CODE_CHIPS/CodeFreqHz;

		// Calculate the 4 quadrant arctangent of the phasor (with the data bit!).
		AtanCurr = PhaseFromPhasor(Phasor, DataBit);

		// Determine the new value of Cn.
		// NOTE This differs from the values shown in Beyerle's paper!
		if (AtanCurr - AtanPrev < -PI) {
			Cprev += TWO_PI;
		} else if (AtanCurr - AtanPrev > PI) {
			Cprev -= TWO_PI;
		} // otherwise, just keep Cprev the same.

		// Calculate the residual phase to be used in the NEXT code cycle, but drawn from this one.
		PhiResidual = AtanCurr + Cprev;

		// Calculate the phase from the NCO (to be used in the NEXT code cycle).
		PhiNCO += TWO_PI*IntegTime*FreqPredictHz;

// UPDATE MODEL FREQUENCY AND NUMBER OF SAMPLES IN CODE CYCLE (AS WELL AS FRACTIONAL CODE PHASE).

		// Update carrier phase for next sine wave generation.
		doCarrierPhase(NumSamplesStored);

		// Update the predicted frequency and data bit for the next code cycle.
		// TODO: Handle if there are multiple cycles that start in the same millisecond.
		GPS_MSOW++; // HACK: will give bad answers for repeated or skipped MSOWs
		GPS_MSOW %= MSEC_IN_WEEK;
		if (GPS_MSOW == 0)
			GPS_Week++;

		// Update our epoch counter (where we are in the data bit, frame, and z_count).
		doEpochUpdate();

		// Extract the proper predicted frequency from our input deck for the NEXT code cycle.
		doPredictIn(GPS_Week,GPS_MSOW);

		// Calculate how many samples are in the next code cycle.
		doCodePhase();
		doCodeRate(FreqPredictHz);

		// Store the values we'll need for the next round.
		AtanPrev = AtanCurr;

		// Print again - certain variables were not technically correct when we printed above.
		fprintf(PredictOut,",%d\n",CycleEdgeIndex);

		// All done with this data, flush it!
		BufferFlush();

	}
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doEpochUpdate()
{
	// Update the 1 millisecond counter.
	_1ms_epoch++;

	// Rollover on bit
	if(_1ms_epoch == 20) {
		_1ms_epoch = 0;

		_20ms_epoch++;

		// Rollover on frame
		if(_20ms_epoch == 300) {
			_20ms_epoch = 0;

			// z_count update - update every 4 z_counts (frame edge synchronicity)
			// and account for end of week.
			z_count = (z_count + 4) % L1_D_MAX_Z_COUNT;
		}
	}
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doCodePhase(void)
{
	// At current rates, how many chips are in a single sample?
	double ChipsPerSample = CodeFreqHz / D_SAMPLE_RATE; // chips/sample

	// How many chips are contained in the "fractional" sample at the end?
	double FracChipsLeft = FracSamplesInCycle * ChipsPerSample; // chips

	// Given that our next code cycle is to start at CycleEdgeIndex, which was advanced by NumSamplesInCycle already,
	// we have to move FracCodePhaseChips backwards by the amount of fractional chips left over.
	FracCodePhaseChips -= FracChipsLeft;

	// Beware moving backwards too far!  If we do, we need to fix it.
	while (FracCodePhaseChips < 0.0) {
		// We're moving too far backwards and have to change what we're doing!
		CycleEdgeIndex++; // Move edge index up one
		FracCodePhaseChips += ChipsPerSample;

		// Update carrier phase here, too!
		doCarrierPhase(1);
	}
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doCodeRate(double FreqBase)
{
	// Calculate the code rate given a Doppler shift.
	CodeFreqHz = L1_CODE_RATE*(1.0 + FreqBase/L1_FREQ_HZ); // chips/sec

	// Calculate the number of samples for this code cycle.
	FracSamplesInCycle = L1_CODE_CHIPS*D_SAMPLE_RATE/CodeFreqHz;
	NumSamplesInCycle = static_cast<int>(FracSamplesInCycle); // whole samples / cycle
	FracSamplesInCycle = fmod(FracSamplesInCycle,1.0); // fractional samples / cycle (beyond whole samples)
	// accumulate # of samples
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
void OLChannel::doCarrierPhase(int NumSamples)
{
	// Update the number of full carrier phase cycles and fractional cycles.
	CarrierPhaseCycles += fmod(static_cast<double>(NumSamples),
		D_SAMPLE_RATE/(FreqPredictHz+ZERO_DOPPLER_RATE))
		*(FreqPredictHz+ZERO_DOPPLER_RATE)/D_SAMPLE_RATE;
	FracCarrierPhaseCycles = fmod(CarrierPhaseCycles,1.0);
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
big_cpx OLChannel::doCarrierWipe() const
{
	big_cpx result;
//	CPX_ACCUM corr_temp;
	int64 corr_temp;

//	// Carrier wipe-off and accumulation.
//	corr_temp = x86_cacc2(&Correlator[0], &Carrier[0], min(NumSamplesStored,D_SAMPLES_PER_MS));
//	result.r = (corr_temp.i);
//	result.i = (corr_temp.q);


	// Carrier wipe-off and accumulation.
	corr_temp = sse_cacc(Correlator, Carrier, min(NumSamplesStored,D_SAMPLES_PER_MS));
	result.r = int ( corr_temp & 0xffffffff);
	result.i = int ((corr_temp >> 32) & 0xffffffff);

	return result;
}

void OLChannel::doCodeWipe()
{

	// Check Init.cpp's offset to determine the exact meaning of code bins; for an offset of -0.5:
	// bin 0 is for -0.5 chip lag, bin CODE_BINS*2 is for +1.5 chip lead (helps with EPL correlators in closed-loop)
	int code_bin = static_cast<int>(floor(FracCodePhaseChips*CODE_BINS + 0.5) + CODE_BINS/2);

	// Wipe the code off.  Start at the front of the code with some partial chip offset.
	// HACK: Use the minimum of the total number of cycles in this sample, OR the number of samples in a millisecond
	// which is the max length of the sampled code.
	x86_cmuln(&PRN_Prompt[code_bin][0], &CycleBuff[0], &Correlator[0],2*min(D_SAMPLES_PER_MS,NumSamplesInCycle));
//	sse_mulc(&PRN_Prompt[code_bin][0],&CycleBuff[0], &Correlator[0],2*min(D_SAMPLES_PER_MS,NumSamplesInCycle));
}
/*---------------------------------------------------------------------------------------------
 *
 ---------------------------------------------------------------------------------------------*/
bool OLChannel::BufferFill()
{
	int NumSamplesAvail;

	// How many samples are left to make a code cycle?
	int NumSamplesRemaining = NumSamplesInCycle - NumSamplesStored;

	// Pointer to memory for the location in the cycle buffer where we're going to store incoming data.
	DATA_CPX *CycleCurr = &CycleBuff[NumSamplesStored];

	// How many samples do we have stored already?
	if (NumSamplesStored == 0) {
		// None stored yet - fresh buffer!
		// How many samples could we get out of this data set?
		NumSamplesAvail = D_SAMPLES_PER_MS - CycleEdgeIndex;

		// If there are no samples available at all, then return.
		if (NumSamplesAvail <= 0) {
			// Reset the cycle edge index to start in the next data set.
			CycleEdgeIndex = CycleEdgeIndex % D_SAMPLES_PER_MS;
			return(false); // Buffer still needs information and there's nothing in this data set we can use!
		}

		// Extract the GPS time from the header.
		GPS_Week = MsecHeader.GPS_Week;
		GPS_MSOW = MsecHeader.GPS_MSOW;

		// Check to be sure that we don't grab too many.
		if (NumSamplesAvail >= NumSamplesRemaining) {
			// Have to restrict how many we grab, otherwise we will go over a code cycle edge.
			memcpy(CycleCurr,&master_data[CycleEdgeIndex],NumSamplesRemaining*sizeof(DATA_CPX));

			// Another code cycle starts in this data set!
			NumSamplesStored = NumSamplesRemaining;
			CycleEdgeIndex += NumSamplesRemaining;
			return(true);
		} else {
			// Grab the rest of the data set.
			memcpy(CycleCurr,&master_data[CycleEdgeIndex],NumSamplesAvail*sizeof(DATA_CPX));

			// We used as much as we could of this data set and we aren't full.
			NumSamplesStored += NumSamplesAvail;
			return(false);
		}
	} else {
		// Ok, it's not a fresh buffer, so what do we have to do?
		// How many samples are available to grab? (should be a full data set)
		NumSamplesAvail = D_SAMPLES_PER_MS;

		if (NumSamplesAvail >= NumSamplesRemaining) {
			// Have to restrict how many we grab, otherwise will go over a code cycle edge.
			memcpy(CycleCurr,&master_data[0],NumSamplesRemaining*sizeof(DATA_CPX));

			// This finishes out this code cycle.  Set for the next one.
			NumSamplesStored += NumSamplesRemaining;
			CycleEdgeIndex = NumSamplesRemaining;
			return(true);
		} else {
			// Grab the whole data set (we don't have enough data to finish off the code cycle).
			memcpy(CycleCurr,&master_data[0],NumSamplesAvail*sizeof(DATA_CPX));

			NumSamplesStored += NumSamplesAvail;
			return(false);
		}
	}
}

/*---------------------------------------------------------------------------------------------
 *
/---------------------------------------------------------------------------------------------*/
void OLChannel::doPRNGet()
{
	for(int lcv = 0; lcv < (2*CODE_BINS+1); lcv++)
		PRN_Prompt[lcv] = PRN_Sampled[iSV][lcv];
}
/*---------------------------------------------------------------------------------------------
 *
/---------------------------------------------------------------------------------------------*/
void OLChannel::doGenSine(double _FrequencyHz, double _PhaseOffsetCycles, cpx_sine *_SineWave, int _SampleLength) const
{
	// Entirely in cycles, no radians!
	// Make sure _PhaseOffsetCycles is between 0 and 1 cycle by using the repeatability of the sine wave.
	_PhaseOffsetCycles = fmod(_PhaseOffsetCycles,1.0);
	if (_PhaseOffsetCycles < 0.0) {
		_PhaseOffsetCycles += 1.0;
	}

	// PhaseOffset must be between 0 and 1 cycle after the correction!!!!
#if OL_CHANNEL_DEBUG > 0
	if (_PhaseOffsetCycles < 0.0 || _PhaseOffsetCycles >= 1.0) {
        printf("OLChannel::doGenSine() PhaseOffsetCycles not between 0 and 1 cycle!!\n");
	}
#endif

	int index, lcv;
	// Take advantage of overflowing unsigned ints to handle our wraparounds.
	unsigned int acc_phase =  (unsigned int) (_PhaseOffsetCycles*PHASE_SCALE);
	unsigned int phase_step = (unsigned int) (_FrequencyHz*PHASE_SCALE/D_SAMPLE_RATE);

	int64 *pSine_Table  = (int64 *)&Sine_Table[0]; //for the sine/cosine lookup
	int64 *SineWave = (int64 *)&_SineWave[0];

	// Generate the sine/cosine.  Because of the int64 and how the complex accumulator works,
	// we're working at the sample level.
	for(lcv = 0; lcv < _SampleLength; lcv++) {
		index = (acc_phase >> PHASE_SHIFT);
		SineWave[lcv] = pSine_Table[index];
		acc_phase += phase_step;
	}
}
