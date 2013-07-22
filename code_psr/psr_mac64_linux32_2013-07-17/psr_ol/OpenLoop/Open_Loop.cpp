

#include "Open_Loop.h"

Open_Loop::Open_Loop(Options *_opt)
{

	memcpy(&opt, _opt, sizeof(Options));

	clock_t time_0, time_1;
	int lcv, tick2[32], lcv2, lcv3, lcv4, lcv5, tick, chan;
	int Year, Month, Day;
	int grs_flag=0;
	int prn_cus[32];
	int monday1[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int monday2[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	double min,minn;
	float mean_val;

	char predictfname[50];
	char filename[1024];
	char svchanfile[100];
	char svstring [100];
	char empstring [100]="                                                                                         ";
	char svstring2 [100];
	char prepstring [150];
	char prepstring2 [150];
	char yearstr [5];
	char daystr [4];
	const char *pch;

	FILE *fp,*sv,*predictf;
	OL_Predict_S predictline1,predictline2;



    // ----------------IO--------------------
	//OPTION-Forward or backward
	if(opt.forward){
		std::cout << "\n---------Forward Tracking-----------";
		sprintf(svchanfile,"sv_chans.dat");
	}
	if(opt.backward){
		std::cout << "\n---------Backward Tracking----------";
		sprintf(svchanfile,"sv_chans_b.dat");
	}
	std::string fileName;
	std::cout << "\nEnter file name:  \n";
   	std::cout << "Example = /media/Files/grs-data/2008.053_RF05_F01_A0_L1  \n";
	// input GRS file name
	getline (std::cin, fileName);

	std::string bitarcName;
	std::cout << "\nEnter bitarc path:  \n";
   	std::cout << "Example = /project/enezeg/a/products/2008.025_heft08/2008.025_bitarc  \n";
	// input bit arc file name
	getline (std::cin, bitarcName);

	printf("bitarcName=%s\n",bitarcName.c_str());
	// ----------------IO--------------------



	// calculate the date for psrbitgen from GRS file name
	pch=strrchr(fileName.c_str(),'/');
    for (int i=pch-fileName.c_str()+1;i<pch-fileName.c_str()+5;i++)
    {
        yearstr[i-(pch-fileName.c_str()+1)]=fileName.c_str()[i];
    }
    for (int i=pch-fileName.c_str()+6;i<pch-fileName.c_str()+9;i++)
    {
        daystr[i-(pch-fileName.c_str()+6)]=fileName.c_str()[i];
    }
    Year = atoi(yearstr);
    Day = atoi(daystr);
    if (Year == 2004 || Year == 2008 || Year == 2012)
    {
        int i=0;
        while ((Day-monday1[i])>0)
        {
            Day = Day - monday1[i];
            i++;
        }
        Month = i+1;
    }
    else
    {
        int i=0;
        while ((Day-monday2[i])>0)
        {
            Day = Day - monday2[i];
            i++;
        }
        Month = i+1;
    }



    // Calculate the total number of the satellite from the user request
    int prn_cus_num=0;
    do
    {
        std::cout << "\nPRN: ";
        std::cin >> prn_cus[prn_cus_num];
        printf("PRN %d\n",prn_cus[prn_cus_num]);
        prn_cus_num++;

    }while(prn_cus[prn_cus_num-1]!=0);
    prn_cus_num = prn_cus_num-1;


	// Malloc a section of memory for holding the recorded data.
	master_data = (DATA_CPX *)malloc(D_SAMPLES_PER_MS*ACQ_TICKS*sizeof(DATA_CPX));
	myLocalBuffer= (uint32 *)malloc(D_SAMPLES_PER_MS*ACQ_TICKS*sizeof(uint32));
	FileHeader = (byte *)malloc(512);
	myMsecHeaderPtr = (Msec_Header_S *)malloc(sizeof(Msec_Header_S));
	msecheader = (uint16 *)malloc(sizeof(uint16)*64);
	buffer = (DATA_CPX *)malloc(D_SAMPLES_PER_MS*sizeof(DATA_CPX));



	int NumMsecPerFile;
	int FileNumber;
	int FileQtt;
	int msec_num;
	int StartSOW;
	int GPSWeek = 0;
	int File_Len_Sec = 3600;
	int lin = 0;
	int sv_ind = 0;
	int sv_ind2 = 0;
	int prep_ind = 0;
	int prep_ind2 = 0;
	int sv_grs[50];
	int sv_sow[50];
	int sv_num[50];
	int sv_prn[50][32];
	double sv_cp[50][32];
	double sv_dop[50][32];
	double occtab[40][10];


	// Create an array of bools which tell us what SVs have had OL channels kicked for them.
	bool OL_spawned[L1_NUM_SVS] = {false};

	// Define a set of variables needed to spawn a new OL channel.
	OL_Frame_Start_S tempFrameInfo;



 	// Open our input file and error check the opening.
    sv=fopen(svchanfile,"r");
    if (!sv)
    {
        printf("\nUnable to open sv_chans file! \n");
        return;
    }
    else
    {
        printf("Read sv_chans file...\n");
    }



    //read sv_chans.dat
    // sv_ind is GRS number, sv_ind2 is satellite number
    fgets (svstring , 100 , sv);
    NumMsecPerFile = atoi(svstring);
    printf("NumMsecPerFile=%d\n",NumMsecPerFile);

    while (fgets (svstring , 100 , sv) != NULL)
    {
        //printf("svstring1_test=%s\n",svstring);

        if (svstring[0]=='G') //GRS
        {
            memmove (svstring,svstring+3,2);
            strncpy (svstring2,svstring,2);
            sv_grs[sv_ind] = atoi(svstring2);

            /* gps week and gps sow
            memmove (svstring,svstring+13,4);
            strncpy (svstring2,svstring,4);
            */
            memmove (svstring,svstring+30,6);
            strncpy (svstring2,svstring,6);
            sv_sow[sv_ind] = atoi(svstring2);

            sv_num[sv_ind] = 0;

            sv_ind++;
            sv_ind2=0;

            strcpy(svstring,empstring);
            strcpy(svstring2,empstring);

        }
        else if (svstring[0]=='E') //EOF
        {

        }
        else  //PRN, Doppler, and code phase
        {

            //printf("svstring1=%s\n",svstring);

            strncpy (svstring2,svstring,2);
            //printf("svstring2=%s\n",svstring2);
            sv_prn[sv_ind-1][sv_ind2] = atoi(svstring2);
            //printf("sv_prn=%d\n",sv_prn[sv_ind-1][sv_ind2]);

            memmove (svstring,svstring+4,11);
            strncpy (svstring2,svstring,11);
            sv_cp[sv_ind-1][sv_ind2] = atof(svstring2);
            //printf("sv_cp[%d][%d]=%f   ",sv_ind-1, sv_ind2, sv_cp[sv_ind-1][sv_ind2]);
            memmove (svstring,svstring+17,12);
            strncpy (svstring2,svstring,12);
            sv_dop[sv_ind-1][sv_ind2] = atof(svstring2);
            //printf("sv_dop[%d][%d]=%f\n",sv_ind-1, sv_ind2, sv_dop[sv_ind-1][sv_ind2]);
            sv_num[sv_ind-1] = sv_num[sv_ind-1]+1;

            sv_ind2++;

            //printf("svstring3=%s\n",svstring);
            strcpy(svstring,empstring);
            strcpy(svstring2,empstring);
            //printf("svstring4=%s\n",svstring);
        }
    }
    fclose(sv);

    /*
    // Read prep_time
    pp=fopen(prepfile,"r");
    if (!pp)
    {
        printf("\nUnable to open prep_time file! \n");
        return;
    }
    else
    {
        printf("Read prep_time file...\n");
    }
    fgets (prepstring , 150 , pp);
    while (fgets (prepstring , 150 , pp) != NULL)
    {
        //PRN
        strncpy (prepstring2,prepstring,2);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //start time
        memmove (prepstring,prepstring+6,134);
        strncpy (prepstring2,prepstring,6);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //end time
        memmove (prepstring,prepstring+16,118);
        strncpy (prepstring2,prepstring,6);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //set or rise
        memmove (prepstring,prepstring+15,103);
        strncpy (prepstring2,prepstring,2);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //start elevation
        memmove (prepstring,prepstring+15,88);
        strncpy (prepstring2,prepstring,7);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //end elevation
        memmove (prepstring,prepstring+19,69);
        strncpy (prepstring2,prepstring,7);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //start file
        memmove (prepstring,prepstring+19,50);
        strncpy (prepstring2,prepstring,2);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //end file
        memmove (prepstring,prepstring+14,36);
        strncpy (prepstring2,prepstring,2);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //acq start
        memmove (prepstring,prepstring+14,22);
        strncpy (prepstring2,prepstring,2);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        //acq end
        memmove (prepstring,prepstring+14,8);
        strncpy (prepstring2,prepstring,2);
        occtab[prep_ind][prep_ind2] = atof(prepstring2);
        prep_ind2++;

        prep_ind++;
        prep_ind2=0;
    }
    */



    // Determine the GRS and satellite we would like to run
    // If user doesn't define the PRN and time
    int sattag = 1;
    int k=0;
    FileNumber = sv_grs[0];
    FileQtt = sv_grs[sv_ind-1];
    if(prn_cus_num==0)
    {
        for (int i=0; i<sv_ind; i++)
        {
            for (int j=0; j<sv_num[i]; j++)
            {
                for(int m=0; m<=k; m++)
                {
                    if(sv_prn[i][j] == prn_cus[m])
                    sattag = 0;
                }
                if (sattag == 1)
                {
                    prn_cus[k] = sv_prn[i][j];
                    k=k+1;
                }
                sattag=1;
            }
        }
        printf("prn_cus=");
        for (int i=0; i<k; i++)
        {
            printf("%d ",prn_cus[i]);
        }
        prn_cus_num=k;
        printf("\nprn_cus_num=%d\n", prn_cus_num);
    }
    else
    {
        // input first file number
        std::cout << "\nEnter first file number:  ";
        std::cin >> FileNumber;
        // input last file number
        std::cout <<"\nEnter last file number:  ";
        std::cin >> FileQtt;
    }



    printf("First GRS: %d\n",FileNumber);
    printf("Last GRS: %d\n",FileQtt);
	// File number error!!!!
	if(opt.forward)
	{
		if(FileNumber > FileQtt)
        {
            printf("\nFirst file number cannot be greater than the last file number\n");
            return;
        }
	}
	if(opt.backward)
	{
		if(FileNumber < FileQtt)
        {
            printf("\nFirst file number connot be less than the last file number (backward!)\n");
            return;
        }
	}







	// Start forward tracking
	if(opt.forward)
	{

			for (int i=0; i<32; i++)
			{
			    tick2[i]=0;
			}


			for(lcv4=FileNumber; lcv4 <= FileQtt; lcv4++)
			{

				// calculate the msec
				int msec_num = lcv4*NumMsecPerFile;
				sprintf(filename,"%s_%d_%d.grs",fileName.c_str(), msec_num, NumMsecPerFile);



				// open a file start with the first one
				printf("File %d: %s\n",lcv4,filename);
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
                printf("Reading GRS file...\n");
                //printf("StartSOW=%d ",StartSOW);
                //printf("GPSWEEK=%d ",GPSWeek);



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



				// Find the GRS file index in sv_chans.dat
				sv_ind=0;
				sv_ind2=0;
				while (sv_grs[sv_ind]!=lcv4)
				{
				    sv_ind++;
				}


                // Run thru every satellite
				for(int i = 0; i<prn_cus_num; i++)
				{

				    // Find the PRN index in sv_chans.dat
				    sv_ind2 = 0;
				    while (sv_prn[sv_ind][sv_ind2]!=prn_cus[i] && sv_ind2<32)
                    {
                        sv_ind2++;
                    }


                    // check if the OLPredict file of this PRN is ready
                    // If ready, grs_flag=1
				    printf("Check PRN%d...\n",prn_cus[i]);
				    //printf("sv_ind2=%d...\n",sv_ind2);
				    sprintf(predictfname,"OLPredictSV%02ds.in",prn_cus[i]);
				    predictf = fopen(predictfname,"rb");

                    if(predictf!=NULL)
                    {
                        fread(&predictline1,sizeof(OL_Predict_S),1,predictf);
                        while (!feof(predictf))
                        {
                            fread(&predictline2,sizeof(OL_Predict_S),1,predictf);
                        }
                                 //printf("predictline1.GPS_SOW=%d\n",predictline1.GPS_SOW);
                                 //printf("predictline2.GPS_SOW=%d\n",predictline2.GPS_SOW);
                        if(predictline1.GPS_SOW<=sv_sow[sv_ind]+599 && predictline2.GPS_SOW>=sv_sow[sv_ind])
                        {
                            if (sv_ind2 == 32 && tick2[prn_cus[i]-1] == 0)
                            {
                                printf("Sorry, no acq result for this GRS file\n");
                            }
                            else
                            {
                                grs_flag = 1;
                            }
                        }
                        else
                        {
                            printf("Sorry, OLPredict file not in the right time\n");
                        }
                    }
                    else
                    {
                        printf("Sorry, NO corresponding OLPredict file!!\n");
                    }


				    lcv = prn_cus[i]-1;
                    //printf("before_lcv=%d\n",lcv);

				    // If OLPredict file is ready,
				    if(grs_flag==1)
				    {
				        printf("We are running PRN%d...\n",prn_cus[i]);

				        if(tick2[prn_cus[i]-1] == 0)
                        {


                            // Clear out the current channel and start tracking this one (closed-loop).
                            pChannels[lcv]->doClearChannel();

                            // give the channel acquisition result from sv_chans.dat
                            pChannels[lcv]->doStartChannel(lcv,
														sv_cp[sv_ind][sv_ind2],
														sv_dop[sv_ind][sv_ind2]);

                            //printf("this sv_cp[%d][%d]=%f\n",sv_ind, sv_ind2, sv_cp[sv_ind][sv_ind2]);
                            //printf("this sv_dop[%d][%d]=%f\n",sv_ind, sv_ind2, sv_dop[sv_ind][sv_ind2]);


                            // Start the timer (to see how long it takes to chew through the data)
                            time_0 = clock();


                            tick = 0;


                            //printf("lcv=%d\n",lcv);
                            //----------------------------------------------------------------------------
                            // Reset our read position to the beginning of the data file (after having done the
                            // acquisition).  Sets to the start of the 0th millisecond.
                            rewind(fp);
                            //read the file header. set to first ms
                            fread(FileHeader,sizeof(byte),512,fp);
                            //---------------------------------------------------------------------------

                            //-----------------------------------------------------------------------------
                            // Process either the entire file.

                            for(lcv2=0; lcv2 < NumMsecPerFile; lcv2++)
                            {
                                //**********
                                //read 1 ms data!!!
                                //**********
                                //read msec header of the data
                                fread(&myMsecHeaderPtr[0],sizeof(byte),sizeof(Msec_Header_S),fp);
                                //memcpy(MsecHeader,&myMsecHeaderPtr[0],sizeof(myMsecHeaderPtr));
                                MsecHeader=*myMsecHeaderPtr;
                                //read 1 ms data
                                fread(&myLocalBuffer[0],sizeof(DATA_CPX),D_SAMPLES_PER_MS/16,fp);
                                //unpacked data (0->1,1->-1)
                                bit_unpack(&master_data[0],&myLocalBuffer[0],D_SAMPLES_PER_MS);

                                // Correlate on all of the active channels.
//					            for(lcv = 0; lcv < MAX_CHANNELS; lcv++)

                                if(pChannels[lcv]->Active())
                                {
                                    pChannels[lcv]->doTrack();
                                }


                            }



                            // Clear out the current channel and start tracking this one (closed-loop).
                            pChannels[lcv]->doClearChannel();

                            //char CLBitFileName[20];
                            //FILE *fid = NULL;
                            //sprintf(CLBitFileName,"DataBitSV06.out");
                            //fid = fopen(CLBitFileName, "rb");
                            //printf("sizeA = %d \n",ftell(fid));
                            //fclose(fid);

                            /*
                            printf("pChannels[lcv]->SV()+1= %d\n",pChannels[lcv]->SV()+1);
                            printf("StartSOW= %d\n",StartSOW);
                            printf("GPSWeek= %d\n",GPSWeek);
                            printf("Year= %d\n",Year);
                            printf("Month= %d\n",Month);
                            printf("Day= %d\n",Day);
                            printf("File_Len_Sec= %d\n",File_Len_Sec);
                            printf("lin= %d\n",lin);
                            printf("bitarcName.c_str()= %s\n",bitarcName.c_str());
                            */


                            pChannels[lcv]->doStartChannel(lcv,sv_cp[sv_ind][sv_ind2],sv_dop[sv_ind][sv_ind2]);
                            pbg->bitgen(pChannels[lcv]->SV()+1,StartSOW,GPSWeek,Year,Month,Day,File_Len_Sec,lin, bitarcName.c_str());
                            //pbg->bitgen(pChannels[lcv]->SV()+1,StartSOW,GPSWeek,Year,Month,Day,File_Len_Sec,lin);

                            //printf("test1");

                        }
                        tick2[prn_cus[i]-1]++;

                        //char CLBitFileName[20];
                        //FILE *fid = NULL;
                        //sprintf(CLBitFileName,"DataBitSV06.out");
                        //fid = fopen(CLBitFileName, "rb");
                        //printf("sizeA = %d \n",ftell(fid));
                        //fclose(fid);

                        //----------------------------------------------------------------------------
                        // Reset our read position to the beginning of the data file (after having done the
                        // acquisition).  Sets to the start of the 0th millisecond.
                        rewind(fp);
                        //read the file header. set to first ms
                        fread(FileHeader,sizeof(byte),512,fp);
                        //---------------------------------------------------------------------------


                        for(lcv2=0; lcv2 < NumMsecPerFile; lcv2++)
                        {
                            //**********
                            //read 1 ms data!!!
                            //**********
                            //read msec header of the data
                            fread(&myMsecHeaderPtr[0],sizeof(byte),sizeof(Msec_Header_S),fp);
                            //memcpy(MsecHeader,&myMsecHeaderPtr[0],sizeof(myMsecHeaderPtr));
                            MsecHeader=*myMsecHeaderPtr;
                            //read 1 ms data
                            fread(&myLocalBuffer[0],sizeof(DATA_CPX),D_SAMPLES_PER_MS/16,fp);
                            //unpacked data (0->1,1->-1)
                            bit_unpack(&master_data[0],&myLocalBuffer[0],D_SAMPLES_PER_MS);

                            // Correlate on all of the active channels.
                            //for(lcv = 0; lcv < MAX_CHANNELS; lcv++)

                            if(pChannels[lcv]->Active())
                            {
                                pChannels[lcv]->doTrack();
                            }


                            // Correlate on all of the active OL channels.  For the inactive ones, check to see if
                            // there are new satellites available.
                            //for(lcv = 0; lcv < MAX_OL_CHANNELS; lcv++) {
							if(pOLChannels[lcv]->Active()) {

								// OL channel is active, so track on it.
								pOLChannels[lcv]->doTrack();
							} else {
								// OL channel is inactive, so over all closed-loop channels...
								for (lcv3 = 0; lcv3 < MAX_CHANNELS; lcv3++) {
									// which are active...
									if (pChannels[lcv3]->Active()) {
										// are a satellite and not WAAS...
										if (pChannels[lcv3]->SV() < L1_NUM_SVS) {
											// and haven't yet spawned off an OL channel...
											if (OL_spawned[pChannels[lcv3]->SV()] == false) {
												// and also say that there is a bit edge in this data set...
												if (pChannels[lcv3]->FrameEdgePresent(tempFrameInfo)) {
													// then we start the OL channel with the appropriate values.

													pOLChannels[lcv]->doClearChannel();
													pOLChannels[lcv]->doStartChannel(tempFrameInfo);
													// Set to spawned regardless of if it worked to avoid repeating errors.

													OL_spawned[tempFrameInfo.SV] = true;

													// Only track if the initialization worked!
													// Otherwise, mark this channel as still inactive.
													if (pOLChannels[lcv]->Active()) {

														pOLChannels[lcv]->doTrack();
														// Optional - kill the CL channel to save time
														// pChannels[lcv3]->Kill();
														break;  // Spawned one OL channel, don't keep spawning in it!
													}
												}
											}
										}
									}
								}
							}
                            tick++;
                        }
				    }
				    grs_flag=0;
				}
                fclose(fp);
			}
		time_1 = clock();
		printf("Tracking took %.2f seconds to process %.2f seconds of data!\n",  (double)(time_1 - time_0) /CLOCKS_PER_SEC, tick/1000.0);

		return;
	}


	/*-----------------------------------------------------------------------
	 * backward tracking
	 * ---------------------------------------------------------------------*/

	if(opt.backward)
	{
		for (int i=0; i<32; i++)
		{
		    tick2[i]=0;
		}

        findms=true;
        for(lcv4=FileNumber; lcv4 >= FileQtt; lcv4--)
        {

            msec_num = lcv4*NumMsecPerFile;
            sprintf(filename,"%s_%d_%d.grs",fileName.c_str(), msec_num, NumMsecPerFile);


            // open a file start with the first one
            printf("File %d: %s\n",lcv4,filename);
            fp=fopen(filename,"rb");
            if (!fp)
            {
                printf("\nUnable to open file! \n");
                return;
            }


            //read ACQ_TICKS ms data
            fseek(fp,(30000)*(-(2*D_SAMPLES_PER_MS/8+128)),SEEK_END);

            for(lcv5=0; lcv5 < (ACQ_TICKS); lcv5++) //skip the first 20 ms
            {
                //fread(&msecheader[0],sizeof(uint16),2,fp);
                //fread(&StartSOW,sizeof(uint16),2,fp);
                //fread(&GPSWeek,sizeof(uint16),1,fp);
				fread(&msecheader[0],sizeof(uint16),64,fp);
                fread(&myLocalBuffer[lcv5*D_SAMPLES_PER_MS/16],sizeof(int),D_SAMPLES_PER_MS/16,fp);
                //StartSOW = StartSOW/1000;
            }

            bit_unpack(&master_data[0],&myLocalBuffer[0],ACQ_TICKS*D_SAMPLES_PER_MS);



            sv_ind=0;
            sv_ind2=0;
            while (sv_grs[sv_ind]!=lcv4)
            {
                sv_ind++;
            }

            for(int i = 0; i<prn_cus_num; i++) //run thru every satellite
            {
                sv_ind2 = 0;
                while (sv_prn[sv_ind][sv_ind2]!=prn_cus[i] && sv_ind2<32)
                {
                    sv_ind2++;
                }

                // check if the OLPredict file of this PRN is ready
                // if ready, grs_flag=1
                //printf("sv_ind2=%d...\n",sv_ind2);
                printf("Check PRN%d...\n",prn_cus[i]);
                sprintf(predictfname,"OLPredictSV%02dr.in",prn_cus[i]);
                predictf = fopen(predictfname,"rb");

                if(predictf!=NULL)
                {
                    fread(&predictline1,sizeof(OL_Predict_S),1,predictf);
                    while (!feof(predictf))
                    {
                        fread(&predictline2,sizeof(OL_Predict_S),1,predictf);
                    }
                                 //printf("predictline1.GPS_SOW=%d\n",predictline1.GPS_SOW);
                                 //printf("predictline2.GPS_SOW=%d\n",predictline2.GPS_SOW);
                    if(predictline1.GPS_SOW<=sv_sow[sv_ind]+599 && predictline2.GPS_SOW>=sv_sow[sv_ind])
                    {
                        if (sv_ind2 == 32 && tick2[prn_cus[i]-1] == 0)
                        {
                            printf("Sorry, no acq result for this GRS file\n");
                        }
                        else
                        {
                            grs_flag = 1;
                        }
                    }
                    else
                    {
                        printf("Sorry, OLPredict file not in the right time\n");
                    }
                }
                else
                {
                    printf("Sorry, NO corresponding OLPredict file!!\n");
                }



                lcv = prn_cus[i]-1;
                //printf("before_lcv=%d\n",lcv);

                if(grs_flag==1)
                {

                    printf("We are running PRN%d...\n",prn_cus[i]);
                    //printf("sv_ind=%d\n",sv_ind);
                    //printf("sv_ind2=%d\n",sv_ind2);
                    //printf("sv_cp[sv_ind][sv_ind2]=%d\n",sv_cp[sv_ind][sv_ind2]);
                    if(tick2[prn_cus[i]-1] == 0)
                    {


                        // Clear out the current channel and start tracking this one (closed-loop).
                        pChannels[lcv]->doClearChannel();

                        // give the channel acquisition result from sv_chans.dat
                        pChannels[lcv]->doStartChannel(lcv,
                                                sv_cp[sv_ind][sv_ind2],
                                                sv_dop[sv_ind][sv_ind2]);



                        // Start the timer (to see how long it takes to chew through the data)
                        time_0 = clock();


                        tick = 0;


                        //printf("lcv=%d\n",lcv);
                        //----------------------------------------------------------------------------
                        // Reset our read position to the beginning of the data file (after having done the
                        // acquisition).  Sets to the start of the 0th millisecond.
                        rewind(fp);
                        //read the file header. set to first ms
                        fseek(fp,(30000)*(-(2*D_SAMPLES_PER_MS/8+128)),SEEK_END);
                        //---------------------------------------------------------------------------

                        //-----------------------------------------------------------------------------
                        // Process either the entire file.
                        for(lcv2=0; lcv2 < 30000; lcv2++)
                        {
                            //**********
                            //read 1 ms data!!!
                            //**********
                            //read msec header of the data
                            fread(&myMsecHeaderPtr[0],sizeof(byte),sizeof(Msec_Header_S),fp);
                            //memcpy(MsecHeader,&myMsecHeaderPtr[0],sizeof(myMsecHeaderPtr));
                            MsecHeader=*myMsecHeaderPtr;
                            //read 1 ms data
                            fread(&myLocalBuffer[0],sizeof(DATA_CPX),D_SAMPLES_PER_MS/16,fp);
                            //unpacked data (0->1,1->-1)
                            bit_unpack(&master_data[0],&myLocalBuffer[0],D_SAMPLES_PER_MS);

                            // Correlate on all of the active channels.
                            //for(lcv = 0; lcv < MAX_CHANNELS; lcv++)

                            if(pChannels[lcv]->Active())
                            {
                                pChannels[lcv]->doTrack();
                            }


                        }

                        pChannels[lcv]->FrameEdgePresent(tempFrameInfo); //NOT SURE
                        //pChannels[lcv]->DeActive();
                        printf("tempFrameInfo.msow=%d\n",tempFrameInfo.msow);
                        printf("tempFrameInfo.z_count=%d\n",tempFrameInfo.z_count);

/*

                        // Clear out the current channel and start tracking this one (closed-loop).
                        pChannels[lcv]->doClearChannel();

                        //printf("test1");
                        pChannels[lcv]->doStartChannel(lcv,
                                                sv_cp[sv_ind][sv_ind2],
                                                sv_dop[sv_ind][sv_ind2]);

*/
                        //GPS_Week = MsecHeader.GPS_Week;
	                    //GPS_MSOW = MsecHeader.GPS_MSOW-1;
                        StartSOW = (MsecHeader.GPS_MSOW-1)/1000;
                        GPSWeek = MsecHeader.GPS_Week;

                        /*
                        printf("pChannels[lcv]->SV()+1=%d\n",pChannels[lcv]->SV()+1);
                        printf("StartSOW=%d\n",StartSOW);
                        printf("GPSWeek=%d\n",GPSWeek);
                        printf("Year=%d\n",Year);
                        printf("Month=%d\n",Month);
                        printf("Day=%d\n",Day);
                        printf("File_Len_Sec=%d\n",File_Len_Sec);
                        printf("lin=%d\n",lin);
                        */

                        pbg->bitgen(pChannels[lcv]->SV()+1,StartSOW-File_Len_Sec,GPSWeek,Year,Month,Day,File_Len_Sec,lin, bitarcName.c_str());
                        //pbg->bitgen(pChannels[lcv]->SV()+1,StartSOW-File_Len_Sec,GPSWeek,Year,Month,Day,File_Len_Sec,lin);

                    }
                    tick2[prn_cus[i]-1]++;

                    //----------------------------------------------------------------------------
                    // Reset our read position to the beginning of the data file (after having done the
                    // acquisition).  Sets to the start of the 0th millisecond.
                    //rewind(fp);
                    //read the file header. set to first ms
                    //fseek(fp,0,SEEK_END);
                    //---------------------------------------------------------------------------




                    count = 0;
					if(findms)
					{
						while(!(MsecHeader.GPS_MSOW == tempFrameInfo.msow))
						{
							//read msec header of the data
                            fseek(fp,(count+1)*(-(2*D_SAMPLES_PER_MS/8+128)),SEEK_END);
                            fread(&myMsecHeaderPtr[0],sizeof(byte),sizeof(Msec_Header_S),fp);
                            //memcpy(MsecHeader,&myMsecHeaderPtr[0],sizeof(myMsecHeaderPtr));
                            MsecHeader=*myMsecHeaderPtr;
                            //read 1 ms data
                            fread(&myLocalBuffer[0],sizeof(DATA_CPX),D_SAMPLES_PER_MS/16,fp);
                            //unpacked data (0->1,1->-1)
                            bit_unpack(&master_data[0],&myLocalBuffer[0],D_SAMPLES_PER_MS);

                            count++;

                        }
                        //printf("MsecHeader.GPS_MSOW=%d\n",MsecHeader.GPS_MSOW);
                        //printf("I'm here A\n");
					}
					if(findms)
					{
						count--;
						//printf("I'm here B\n");
					}

					findms=false;
					lcv2 = count; //for testing
					//printf("lcv2=%d\n",lcv2);
					for(lcv2 = count; lcv2 < NumMsecPerFile; lcv2++)
					{

						//read msec header of the data
						fseek(fp,(lcv2+1)*(-(2*D_SAMPLES_PER_MS/8+128)),SEEK_END);
                        //fseek(fp,(1)*(-(2*D_SAMPLES_PER_MS/8+128)),SEEK_END);
						fread(&myMsecHeaderPtr[0],sizeof(byte),sizeof(Msec_Header_S),fp);
						//memcpy(MsecHeader,&myMsecHeaderPtr[0],sizeof(myMsecHeaderPtr));
						MsecHeader=*myMsecHeaderPtr;
						//read 1 ms data
						fread(&myLocalBuffer[0],sizeof(DATA_CPX),D_SAMPLES_PER_MS/16,fp);
						//unpacked data (0->1,1->-1)
						bit_unpack(&master_data[0],&myLocalBuffer[0],D_SAMPLES_PER_MS);



						// Correlate on all of the active OL channels.  For the inactive ones, check to see if
						// there are new satellites available.
						for(lcv = 0; lcv < MAX_OL_CHANNELS; lcv++)
						{
							if(pOLChansBck[lcv]->Active())
							{
								// OL channel is active, so track on it.
								pOLChansBck[lcv]->doTrack();
							}
							else
							{
								// OL channel is inactive, so over all closed-loop channels...
								//for (lcv3 = 0; lcv3 < MAX_CHANNELS; lcv3++)
								//{
									// which are active...
									if (pChannels[lcv]->Active())
									{
										//printf("pChannels[%d] is active\n",lcv);
										// are a satellite and not WAAS...
										if (pChannels[lcv]->SV() < L1_NUM_SVS)
										{
											//printf("pChannels[%d]->SV < L1_NUM_SVS\n",lcv);
											// and haven't yet spawned off an OL channel...
											if (OL_spawned[pChannels[lcv]->SV()] == false)
											{
												//printf("pChannels[%d]->SV == false\n",lcv);
												//printf("tempFrameInfo.msow=%d\n",tempFrameInfo.msow);
                                                //printf("tempFrameInfo.z_count=%d\n",tempFrameInfo.z_count);
												// and also say that there is a bit edge in this data set...
												if (pChannels[lcv]->FrameEdgePresent(tempFrameInfo))
												{
													//printf("pChannels[%d]->FrameEdgePresent(tempFrameInfo)\n",lcv);
													//printf("tempFrameInfo.msow=%d\n",tempFrameInfo.msow);
													//printf("tempFrameInfo.z_count=%d\n",tempFrameInfo.z_count);
													// then we start the OL channel with the appropriate values.
													pOLChansBck[lcv]->doClearChannel();
													pOLChansBck[lcv]->doStartChannel(tempFrameInfo);
													// Set to spawned regardless of if it worked to avoid repeating errors.
													OL_spawned[tempFrameInfo.SV] = true;
													// Only track if the initialization worked!
													// Otherwise, mark this channel as still inactive.
													if (pOLChansBck[lcv]->Active())
													{
														pOLChansBck[lcv]->doTrack();
														// Optional - kill the CL channel to save time
														// pChannels[lcv3]->Kill();
														break;  // Spawned one OL channel, don't keep spawning in it!
													}
												}
											}
										}
									}
								}
							}
						}
						tick++;
					//}
                }
                grs_flag=0;
            }
            fclose(fp);

        }
	}
	/*-----------------------------------------------------------------------------*/

	// Get ending time for this run and print the data.
	time_1 = clock();
	printf("Tracking took %.2f seconds to process %.2f seconds of data!\n",  (double)(time_1 - time_0) /CLOCKS_PER_SEC, tick/1000.0);

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

