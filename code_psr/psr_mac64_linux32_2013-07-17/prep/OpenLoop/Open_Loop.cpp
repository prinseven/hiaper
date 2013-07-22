

#include "Open_Loop.h"

Open_Loop::Open_Loop(Options *_opt)
{

	memcpy(&opt, _opt, sizeof(Options));

	clock_t time_0, time_1;
	int lcv, lcv2, lcv3, lcv4, lcv5, lcv6, lcv7, tick;
	int tosat[40];  // Maximum 40 GRS files and 30 satellites
	double min,minn;
	float mean_val;

	char filename[1024];
	FILE *fp;
	//OPTION-Forward or backward

    std::cout << "\n---------Preprocessing-----------";

	std::string fileName;
	std::cout << "\nEnter file name:  \n";
   	std::cout << "Example = /media/Files/grs-data/2008.053_RF05_F01_A0_L1  \n";
	// input file name
	getline (std::cin, fileName);


	// Malloc a section of memory for holding the recorded data.
	master_data = (DATA_CPX *)malloc(D_SAMPLES_PER_MS*ACQ_TICKS*sizeof(DATA_CPX));
	master_data_b = (DATA_CPX *)malloc(D_SAMPLES_PER_MS*ACQ_TICKS*sizeof(DATA_CPX));
	myLocalBuffer= (uint32 *)malloc(D_SAMPLES_PER_MS*ACQ_TICKS*sizeof(uint32));
	myLocalBuffer_b= (uint32 *)malloc(D_SAMPLES_PER_MS*ACQ_TICKS*sizeof(uint32));
	FileHeader = (byte *)malloc(512);
	myMsecHeaderPtr = (Msec_Header_S *)malloc(sizeof(Msec_Header_S));
	msecheader = (uint16 *)malloc(sizeof(uint16)*64);
	msecheader_b = (uint16 *)malloc(sizeof(uint16)*64);
	buffer = (DATA_CPX *)malloc(D_SAMPLES_PER_MS*sizeof(DATA_CPX));

	// Open our input file and error check the opening.

	int NumMsecPerFile;
	int FileNumber;
	int FileQtt;
	int msec_num;
	int StartSOW;
	int GPSWeek = 0;
	lcv2=0;
	lcv3=0;
	lcv6=0;
	lcv7=0;
	for(int i=0; i<=29; i++)
	{
	    tosat[i]=35;
	}

	// input file length
	std::cout << "\nEnter file length in milliseconds:  ";
	std::cin >> NumMsecPerFile;
	// input first file number
	std::cout << "\nEnter first file number:  ";
	std::cin >> FileNumber;
	// input last file number
	std::cout <<"\nEnter last file number:  ";
	std::cin >> FileQtt;


	// Create an array of bools which tell us what SVs have had OL channels kicked for them.
	bool OL_spawned[L1_NUM_SVS] = {false};

	// Define a set of variables needed to spawn a new OL channel.
	OL_Frame_Start_S tempFrameInfo;

    FILE* sv_chan_file = fopen("sv_chans.dat","w");

    fprintf(sv_chan_file,"%d\n", NumMsecPerFile);

    FILE* sv_chan_file_b = fopen("sv_chans_b.dat","w");

    fprintf(sv_chan_file_b,"%d\n", NumMsecPerFile);

    for(lcv4=FileNumber; lcv4 <= FileQtt; lcv4++)
    {

        int msec_num = lcv4*NumMsecPerFile;

        sprintf(filename,"%s_%d_%d.grs",fileName.c_str(), msec_num, NumMsecPerFile);

        // open a file start with the first one

        printf("\nFile %d: %s\n",lcv4,filename);
        fp=fopen(filename,"rb");
        if (!fp)
        {
            printf("\nUnable to open file! \n");
            return;
        }

        //read file header 512 bytes
        fread(&FileHeader[0],sizeof(byte),512,fp);

        // get GPS week and SOW from header of first ms.
        fread(&msecheader[0],sizeof(uint16),2,fp);
        fread(&StartSOW,sizeof(uint16),2,fp);
        fread(&GPSWeek,sizeof(uint16),1,fp);
        fread(&msecheader[0],sizeof(uint16),59,fp);
        fread(&myLocalBuffer[0],sizeof(int),D_SAMPLES_PER_MS/16,fp);
        StartSOW = StartSOW/1000;

        //read ACQ_TICKS ms data

        for(lcv5=0; lcv5 < (ACQ_TICKS-1); lcv5++)//skip ACQ_TICS ms of data (as PSR does)
        {
            fread(&msecheader[0],sizeof(uint16),64,fp);
            fread(&myLocalBuffer[lcv5*D_SAMPLES_PER_MS/16],sizeof(int),D_SAMPLES_PER_MS/16,fp);
        }

        for(lcv5=0; lcv5 < ACQ_TICKS; lcv5++)
        {
            fread(&msecheader[0],sizeof(uint16),64,fp);
            fread(&myLocalBuffer[lcv5*D_SAMPLES_PER_MS/16],sizeof(int),D_SAMPLES_PER_MS/16,fp);
        }

        //unpack the buffered data
        bit_unpack(&master_data[0],&myLocalBuffer[0],ACQ_TICKS*D_SAMPLES_PER_MS);

        fseek(fp,(30000)*(-(2*D_SAMPLES_PER_MS/8+128)),SEEK_END);

        for(lcv5=0; lcv5 < ACQ_TICKS; lcv5++)
        {
            fread(&msecheader_b[0],sizeof(uint16),64,fp);
            fread(&myLocalBuffer_b[lcv5*D_SAMPLES_PER_MS/16],sizeof(int),D_SAMPLES_PER_MS/16,fp);
        }

        bit_unpack(&master_data_b[0],&myLocalBuffer_b[0],ACQ_TICKS*D_SAMPLES_PER_MS);

        // Acquisition - cold acquire satellites using the current acquisition routine.
        printf("Acquiring Satellites...\n");
        time_0 = clock();
        pAcquisition->Cold_Acquire(master_data, &Acq_results[0]);
        pAcquisition->Cold_Acquire(master_data_b, &Acq_results_b[0]);
        time_1 = clock();



        //Declare Detection
        //----------------------------------------------------------------------------
        // Find the mean of the SNRs found during acquisition.
        min = 0.0;
        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            Acq_results[lcv].detected = false;
            min += Acq_results[lcv].SNR / L1_NUM_CODES;
        }

        //-----------------------------------------------------------------------------
        //New detection alg.
        minn = (double)Acq_results[0].CorrMax;
        mean_val = Acq_results[0].Mean;
        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            if(Acq_results[lcv].CorrMax < minn)
            {
                minn = (double)Acq_results[lcv].CorrMax;
                mean_val = Acq_results[lcv].Mean;
            }

        }

        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            Acq_results[lcv].NewSNR = (Acq_results[lcv].CorrMax-mean_val)/mean_val;
            //std::cout << "SV " <<Acq_results[lcv].SV+1<<", SNR = "<< Acq_results[lcv].NewSNR <<"\n";
        }

        //-----------------------------------------------------------------------------
        // For all satellites...
        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            // Check to see if the satellite is there, i.e. the SNR is at least 0.5 dB above the mean.
		    //if(Acq_results[lcv].NewSNR > 2.0) {
            if((Acq_results[lcv].SNR-min)>0.3)
            {
                // Let us know that we have detected this satellite.
                Acq_results[lcv].detected = true;

            }
        }



//Declare Detection
        //----------------------------------------------------------------------------
        // Find the mean of the SNRs found during acquisition.
        min = 0.0;
        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            Acq_results_b[lcv].detected = false;
            min += Acq_results_b[lcv].SNR / L1_NUM_CODES;
        }

        //-----------------------------------------------------------------------------
        //New detection alg.
        minn = (double)Acq_results_b[0].CorrMax;
        mean_val = Acq_results_b[0].Mean;
        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            if(Acq_results_b[lcv].CorrMax < minn)
            {
                minn = (double)Acq_results_b[lcv].CorrMax;
                mean_val = Acq_results_b[lcv].Mean;
            }

        }

        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            Acq_results_b[lcv].NewSNR = (Acq_results_b[lcv].CorrMax-mean_val)/mean_val;
            //std::cout << "SV " <<Acq_results[lcv].SV+1<<", SNR = "<< Acq_results[lcv].NewSNR <<"\n";
        }

        //-----------------------------------------------------------------------------
        // For all satellites...
        for(lcv = 0; lcv < NUM_CODES; lcv++)
        {
            // Check to see if the satellite is there, i.e. the SNR is at least 0.5 dB above the mean.
		    //if(Acq_results[lcv].NewSNR > 2.0) {
            if((Acq_results_b[lcv].SNR-min)>0.3)
            {
                // Let us know that we have detected this satellite.
                Acq_results_b[lcv].detected = true;

            }
        }




        //-----------------------------------------------------------------------------
        // Declare detection and count them in total founded satellite
        fprintf(sv_chan_file,"GRS%2d GPSWeek%4d SecondOfWeek%6d\n",lcv4,GPSWeek,StartSOW);
        fprintf(sv_chan_file_b,"GRS%2d GPSWeek%4d SecondOfWeek%6d\n",lcv4,GPSWeek,StartSOW);
        for (lcv = 0; lcv < NUM_CODES; lcv++)
        {
            if (Acq_results[lcv].detected == true)
            {
                fprintf(sv_chan_file,"%2d  %11.6f  %12.6f\n",Acq_results[lcv].SV+1, Acq_results[lcv].code_phase, Acq_results[lcv].doppler);
                while( Acq_results[lcv].SV+1 > tosat[lcv3] )
                {
                    lcv3 ++ ;
                }
                if (Acq_results[lcv].SV+1 < tosat[lcv3])
                {
                    for(int i=lcv2; i>lcv3; i--)
                    {
                        tosat[i]=tosat[i-1];
                    }
                    tosat[lcv3] = Acq_results[lcv].SV+1;
                    lcv2 ++ ;
                }
                lcv3 = 0;
                lcv7++;
            }

        }
        lcv6 ++;
        lcv7 = 0;

        for (lcv = 0; lcv < NUM_CODES; lcv++)
        {
            if (Acq_results_b[lcv].detected == true)
            {
                fprintf(sv_chan_file_b,"%2d  %11.6f  %12.6f\n",Acq_results_b[lcv].SV+1, Acq_results_b[lcv].code_phase, Acq_results_b[lcv].doppler);
            }
        }

        //fprintf(sv_chan_file,'\n');



        printf("Direct acquisition of GRS%d took %.2f seconds!\n",  lcv4, (double)(time_1 - time_0) /CLOCKS_PER_SEC);

        // Start the timer (to see how long it takes to chew through the data)
        time_0 = clock();

        tick = 0;

        fclose(fp);

    }
    fprintf(sv_chan_file,"EOF\n");
    fclose(sv_chan_file);
    fprintf(sv_chan_file_b,"EOF\n");
    fclose(sv_chan_file_b);


    return;

}

/*-----------------------------------------------------------------------------------
 *
 * ---------------------------------------------------------------------------------*/
// Unpack GRS data
void Open_Loop::bit_unpack(DATA_CPX *unpacked, uint32 *packed, int32 cnt)
{

	int32 lcv, k;
	uint32 *pB= (uint32*)packed;
	uint32 val;
	DATA_CPX lookup[4];
	k = 0;


	// Assumes packed such that bit 0 holds Q0, bit 1 holds I0, bit 2 holds Q1, ... bit 30 holds Q15, bit 31 holds I15.
	lookup[0].r = 1;
	lookup[0].i = 1;

	lookup[1].r = 1;
	lookup[1].i = -1;

	lookup[2].r = -1;
	lookup[2].i = 1;

	lookup[3].r = -1;
	lookup[3].i = -1;

	val = *pB;
	for(lcv = 0; lcv < cnt; lcv++) {
		unpacked[lcv] = lookup[(val & 0x0000003)];
		val >>= 2;

		if(k++ >= 15) {
			pB++;
			val = *pB;
			k = 0;
		}
	}
}

