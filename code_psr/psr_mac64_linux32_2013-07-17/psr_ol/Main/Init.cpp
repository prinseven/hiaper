#include "Includes.h"
#include "Complex_Sine.h"

void usage(char* _str)
{
	fprintf(stderr,"\n");
	fprintf(stderr,"usage: [-b] [f] \n");
	fprintf(stderr,"[-f] forward tracking (default)\n");
	fprintf(stderr,"[-b] backward tracking\n");
	fflush(stderr);
	exit(1);
}

void Parse_Arguments(int argc, char* argv[])
{
	int lcv;

	//default settings
	gopt.forward = true;
	gopt.backward = false;
	//gopt.forward = false;
	//gopt.backward = true;

	for(lcv = 1; lcv < argc; lcv++)
		{
			switch (argv[lcv][1])
			{

				case 'f':
					gopt.backward = false;
					gopt.forward = true;
					break;
				case 'b':
					gopt.backward = true;
					gopt.forward = false;
					break;

			}
		}
}
void Object_Init(void)
{
	double phase, phase_step, offset, f_doppler;
	int indice, lcv, lcv2, SV;
	Acq_Options d_acq_opt;
	//--------------------------------------------
	L1_PRNGen();
	//---------------------------------------
	short *psine;

	for(lcv = 0; lcv < (2*CARRIER_BINS+1); lcv++)
		Channel_Sine[lcv] = (DATA_TYPE *)malloc(D_SAMPLES_PER_MS*4*2*sizeof(DATA_TYPE));

	//Sample the sine at different doppler bins (100 Hz resolution)
	for(lcv = 0; lcv < (2*CARRIER_BINS+1); lcv++)
	{
		f_doppler = (double)ZERO_DOPPLER_RATE + ((double)(lcv - CARRIER_BINS) * (double)CARRIER_SPACING);
		phase_step = TWO_PI*f_doppler/D_SAMPLE_RATE;
		phase = 0;

		psine = (short *)&Channel_Sine[lcv][0];

		/* Generate the negative frequency sine/cosine */
		for(lcv2 = 0; lcv2 < 2*D_SAMPLES_PER_MS; lcv2++)
		{
			psine[4*lcv2]   = (short) 32*cos(phase);
			psine[4*lcv2+1] = (short) 32*sin(phase);
			psine[4*lcv2+2] = (short) -32*sin(phase);
			psine[4*lcv2+3] = (short) 32*cos(phase);
			phase += phase_step;
		}
	}
	//------------------------------------------------------
	for(SV = 0; SV < L1_NUM_CODES; SV++)
		for(lcv2 = 0; lcv2 < (2*CODE_BINS+1); lcv2++)
			PRN_Sampled[SV][lcv2] = (DATA_TYPE *)malloc(2*2*D_SAMPLES_PER_MS*sizeof(DATA_TYPE));

	for(SV = 0; SV < L1_NUM_CODES; SV++)
		for(lcv = 0; lcv < (2*CODE_BINS+1); lcv++)
		{
			offset = -0.5 + (double)lcv/(double)CODE_BINS;

			for(lcv2 = 0; lcv2 < 2*D_SAMPLES_PER_MS; lcv2++)
			{
				indice  = (int)floor((lcv2*(static_cast<double>(L1_CODE_RATE)/D_SAMPLE_RATE)) + 0.5 + offset) % CODE_CHIPS;
				PRN_Sampled[SV][lcv][2*lcv2] = PRN_Sampled[SV][lcv][2*lcv2+1] = PRN_Codes[SV][indice];
			}
		}
	//--------------------------------------------------
	//Acquisition Options
	d_acq_opt.cticks = ACQ_CTICKS;
	d_acq_opt.iticks = ACQ_ITICKS;
	d_acq_opt.doppler_bins = ACQ_DOPPLER_BINS;
	d_acq_opt.doppler_bin_width = ACQ_DOPPLER_BIN_WIDTH;
	d_acq_opt.sample_rate = D_SAMPLE_RATE;
	d_acq_opt.resample_rate = ACQ_RESAMPLE_RATE;
	d_acq_opt.samples_per_ms = D_SAMPLES_PER_MS;
	d_acq_opt.resamples_per_ms = ACQ_SAMPLES_PER_MS;
	d_acq_opt.log = true;
	sprintf(d_acq_opt.filename,"acq.dat");

			
	//-----------------------------------------------------------

	/*Create and set all channels to inactive */
//	printf("Creating Channels\n");
	for(lcv = 0; lcv < MAX_CHANNELS; lcv++)
		pChannels[lcv] = new Channel(lcv);

	/*Create OL channels */
	#if (PROCESS_MODE == OPEN_LOOP)
//		printf("Creating Open-Loop Channels\n");
		if(gopt.forward){
			for(lcv = 0; lcv < MAX_OL_CHANNELS; lcv++)
						pOLChannels[lcv] = new OLChannel(lcv);
		}
		if(gopt.backward){
			for(lcv = 0; lcv < MAX_OL_CHANNELS; lcv++)
						pOLChansBck[lcv] = new OLChanBck(lcv);
		}
	#endif

	pAcquisition = new Acquisition(d_acq_opt);
	pOpen_Loop = new Open_Loop(&gopt);

}

//------------------------------------------------------------------------------------
void L1_PRNGen()
{

	// Generate the L1 C/A code.
	int G1[L1_CODE_CHIPS] = {0};
	int G2[L1_CODE_CHIPS][10] =	{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
	int G1_register[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int G2_register[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int registertemp[9];
	int feedback1, feedback2;
	int lcv, lcv2, lcv3;

/* Note:  Initial registers for WAAS G2 have nothing in common
		with initial C/A code G2 register, nor other WAAS G2 initial
		registers (each WAAS G2 register has a unique initial state).
		Please see USDT FAA Specification for the Wide Area Augmentation
		System (WAAS) [WAAS Specification FFE-E-2892b] Appendix 2, Page 5.
		4 Oct 2006
		TDL*/

	#if (WAAS > 0)
		int delays[L1_NUM_CODES] = {5, 6, 7, 8, 17, 18, 139, 140, 141, 251, 252, 254 ,255, 256, 257, 258, 469, 470, 471, 472, 473, 474, 509, 512, 513, 514, 515, 516, 859, 860, 861, 862, 145, 175, 52, 21, 237, 235, 886, 657, 634, 762, 355, 1012, 176, 603, 130, 359, 595, 68, 386};
	#else
		int delays[L1_NUM_CODES] = {5, 6, 7, 8, 17, 18, 139, 140, 141, 251, 252, 254 ,255, 256, 257, 258, 469, 470, 471, 472, 473, 474, 509, 512, 513, 514, 515, 516, 859, 860, 861, 862};
	#endif

	// Generate G1 Register
	for(lcv = 0; lcv < L1_CODE_CHIPS; lcv++)
	{
		G1[lcv] = G1_register[9];
		feedback1 = G1_register[2]^G1_register[9];

		for(lcv2 = 0; lcv2 < 9; lcv2++)
			registertemp[lcv2] = G1_register[lcv2];

		for(lcv2 = 0; lcv2 < 9; lcv2++)
			G1_register[lcv2+1] = registertemp[lcv2];

		G1_register[0] = feedback1;
	}


	/* Generate G2 Register */
	for(lcv2=0; lcv2 < L1_CODE_CHIPS; lcv2++)
	{
		for(lcv = 0; lcv < 10; lcv++)
			G2[lcv2][lcv] = G2_register[lcv];

			feedback2 = (G2_register[1]+G2_register[2]+G2_register[5]+G2_register[7]+G2_register[8]+G2_register[9]) % 2;

		for(lcv3 = 0; lcv3 < 9; lcv3++)
			registertemp[lcv3] = G2_register[lcv3];

		for(lcv3 = 0; lcv3 < 9; lcv3++)
			G2_register[lcv3+1] = registertemp[lcv3];

		G2_register[0] = feedback2;
	}

	/* Generate PRNs from G1 and G2 Registers */
	for(lcv2 = 0; lcv2 < L1_NUM_CODES; lcv2++)
		for(lcv = 0; lcv < L1_CODE_CHIPS; lcv++)
			PRN_Codes[lcv2][lcv] =  ( 2*(G1[lcv]^G2[((1023-delays[lcv2])+lcv)%1023][9]) )-1;
}
