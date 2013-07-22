#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <cstdlib>
#include <stddef.h>
#include <float.h>
#include <dirent.h>
#include "psrbitgen.h"

using namespace std;
//char test_c[20];
//int test_i;


psrbitgen::psrbitgen()
{

}

void psrbitgen::bitgen(int PRN, int startSOW, int GPS_Week, int Year, int Month, int Day, int File_Len_Sec, int lin, const char *bitarcName)
//void psrbitgen::bitgen(int PRN, int startSOW, int GPS_Week, int Year, int Month, int Day, int File_Len_Sec, int lin)
{
    printf("************* PSRBitGen start a job *************\n");
    //char bitarcName[100]="/project/enezeg/b/data/hiaper/2010.215_predict/2010.215_bitarc/2010data";
    //printf("bitarcName(psrbitgen)=%s\n",bitarcName);

    int i                  = 0;
	int j                  = 0;
	int zMatchi[5]={0,0,0,0,0};                    //check if need to increase!
	int prn[5]={0,0,0,0,0};                        //check if need to increase!
	int CN[5]={0,0,0,0,0};                         //check if need to increase!
	int sfMask[5]={0,0,0,0,0};                     //check if need to increase!
    int sfParityMask[5]={0,0,0,0,0};               //check if need to increase!
	int frameStart[5]={0,0,0,0,0};                 //check if need to increase!
	int CLBit_Length;
	int OLBit_Length;
    int lcv;
    int lcv2;
	int bitarc_number      = 0;
	int bitarc_length      = 0;
	int match_length       = 0;
    int maxCN0i            = 1;
    int maxCN0             = 0;
    int bad                = 0;
    int good               = 0;
    int offset             = 0;
    int Z;
    unsigned int frame[5][subframe_per_frame][10]; //check if need to increase!
    short int COSMICframeBits[subframe_per_frame][301];
    char COSMICframeBits_t[subframe_per_frame][301];
    char frameb[subframe_per_frame][5][301];

    double Zcount          = floor(startSOW / 1.5);
    double numZcounts      = File_Len_Sec / 1.5;
	double endZcount;
    char CLBitFileName[20];
    char YMD[10];
    char bitArcDir[100];
    char outfilestr[20];
    char OLBitFileName[20];
	char bitArcFileSec[10];
	char matchName[5][35];     //check if need to increase!
	char pathname[5][35];      //check if need to increase!
	char hostName[5][8];       //check if need to increase!
	char sfMaskb[5][8];        //check if need to increase!
	char sfParityMaskb[5][8];  //check if need to increase!
	char sfMaskb_t[5][8];      //check if need to increase!
	char sfParityMaskb_t[5][8];//check if need to increase!
	struct CLB
	{
		//unsigned int zcount;
		int zcount;
		int databit[300];
	}CLBits[610];
	struct dirent *bitarcname;
    DIR *arcdir;
    FILE *output_file=NULL;

	sprintf(CLBitFileName,"DataBitSV%02d.out",PRN);
    //printf("CLBitFileName=%s \n",CLBitFileName);

	endZcount = Zcount + numZcounts;
	Zcount = Zcount - int(Zcount)%20;
    //printf("Zcount_a=%f\n",Zcount);

	// PSRBitReader.m*****************************************************
	FILE *fid = NULL;		// File pointer


    fid = fopen(CLBitFileName, "rb");
    //printf("sizeB = %d \n",ftell(fid));


	if (!fid)
	{
		cout << "Data Bit file " << CLBitFileName << " not found." << endl;
	}
	else
	{

		while (!feof(fid))
		{

			//printf("hello!\n");

			CLBits[i].zcount=0;

			fread(&CLBits[i].zcount, 4, 1, fid);

			for (j = 0; j <= 299; j++)
			{

			    fread(&CLBits[i].databit[j], 2, 1, fid);
			    if (CLBits[i].databit[j]==65535) CLBits[i].databit[j]=-1;
			}

			i=i+1;
			//printf("to the end?%d\n",feof(fid));

		}
		//printf("CLBits[0].zcount=%d \n",CLBits[0].zcount);

	}

	CLBit_Length = i-1;
    printf("CLBit_Length=%d \n",CLBit_Length);
	fclose(fid);
	//*********************************************************************

	//for (j = 0; j <= 299; j++) printf("%d ",CLBits[0].databit[j]);
    //for (i = 0; i <= 97; i++) printf("%d ",CLBits[i].zcount);
    //printf("CLBits[0].zcount=%d \n",CLBits[0].zcount);


	//We need to start building the file on a subframe edge, but we might
    //as well start at the beginning of the frame.
	int epochs = File_Len_Sec / Sec_Per_Frame;
	double sow[epochs], dow[epochs], hod[epochs], GPS_sec[epochs];

	for (lcv=1 ; lcv <= epochs; lcv++)
	{
		//sow[lcv-1] = Zcount*1.5 + lcv*Sec_Per_Frame;
		sow[lcv-1] = Zcount*1.5 + (lcv-1)*Sec_Per_Frame;
		dow[lcv-1] = (sow[lcv-1]/3600)/24;
		hod[lcv-1] = floor((dow[lcv-1]-floor(dow[lcv-1]))*24);
		GPS_sec[lcv-1] = GPS_Week*Sec_Per_Week + sow[lcv-1];
	}
	//for (j = 0; j <= epochs-1; j++) printf("%f ",GPS_sec[j]);

    //printf("epochs=%d\n", epochs);
	//We do not have to worry about the occ. event time crossing more than
    //one 'hour boundary', but we must check to see if we do in fact cross
    //one.
	sprintf (YMD, "%4d%02d%02d" ,Year,Month,Day);
	int MinMax_length = int( hod[epochs-1] - ( hod[0] - 1 ) );
	int MinMax_hod[MinMax_length];
	printf("MinMax_length=%d\n", MinMax_length);
	for (i=0 ; i<(MinMax_length) ; i++)
	{
		MinMax_hod[i] = int(hod[0]) + i;
	}
	//for (i=0 ; i<(MinMax_length) ; i++) printf("%d ",MinMax_hod[i]);

	for (lcv = 1; lcv <= MinMax_length ; lcv++)
	{
		if(lin)
        {
			sprintf(bitArcDir,".\\%s\\%02d\\%02d\\", YMD, MinMax_hod[lcv-1], PRN);
	    }
		else
		{
			//sprintf(bitArcDir,"%s/%s/%02d/%02d/", bitarcName, YMD, MinMax_hod[lcv-1], PRN);
			sprintf(bitArcDir,"%s/%02d/%02d/", bitarcName, MinMax_hod[lcv-1], PRN);
		}
		arcdir = opendir(bitArcDir);
		if (arcdir == NULL)
		{
			printf("Bit Archive Directory %s not found.",bitArcDir);
		}
		else
		{
		    while ((bitarcname = readdir(arcdir)) != NULL)
            {
                if(strlen(bitarcname->d_name)==unsigned(nameLength))
                {
                    bitarc_number = bitarc_number+1;
                }
            }
            bitarc_length = bitarc_length + bitarc_number;
            rewinddir(arcdir);
		}

		bitarc_number = 0;

	}

	char bitArcFileName[bitarc_length][35];
	int arc_gps_sec[bitarc_length];
	int arcSOW[bitarc_length];
	int arcHOD[bitarc_length];
	double arcZcount[bitarc_length];

	for (lcv = 1; lcv <= MinMax_length ; lcv++)
	{
		//printf("lcv=%d\n",lcv);
		if(lin)
        {
			sprintf(bitArcDir,".\\%s\\%02d\\%02d\\", YMD, MinMax_hod[lcv-1], PRN);
			printf("%s\n",bitArcDir);
	    }
		else
		{
			//sprintf(bitArcDir,"%s/%s/%02d/%02d/", bitarcName, YMD, MinMax_hod[lcv-1], PRN);
			sprintf(bitArcDir,"%s/%02d/%02d/", bitarcName, MinMax_hod[lcv-1], PRN);
		}
        printf("bitArcDir=%s\n",bitArcDir);
		arcdir = opendir(bitArcDir);
        if (arcdir == NULL)
		{
			printf("Bit Archive Directory %s not found.",bitArcDir);
		}
		else
		{
		    while ((bitarcname = readdir(arcdir)) != NULL)
		    {
			    if(strlen(bitarcname->d_name)==unsigned(nameLength))
			    {
                    strcpy(bitArcFileName[bitarc_number],bitarcname->d_name);
                    for( i=0 ; i<=9 ; i++ ) bitArcFileSec[i]=bitArcFileName[bitarc_number][i+8];
                    arc_gps_sec[bitarc_number]=atoi(bitArcFileSec);
                    arcSOW[bitarc_number]=arc_gps_sec[bitarc_number] - GPS_Week * Sec_Per_Week;
                    arcZcount[bitarc_number]=arcSOW[bitarc_number]/1.5;
                    arcHOD[bitarc_number]=floor((arcSOW[bitarc_number]/3600)%24);
                    bitarc_number = bitarc_number+1;
			    }
		    }
		}


	}
	//for (i=0;i<bitarc_length;i++) printf("%s\n",bitArcFileName[i]);
	//for (i=0;i<bitarc_length;i++) printf("%d ",arcHOD[i]);

	//Open the output file for writing.
	sprintf( outfilestr, "DataBitSV%02d.in", PRN );
	output_file = fopen(outfilestr, "wb");
	if (!output_file)
	{
		cout << "Unable to open " << outfilestr << " !" << endl;
	}

	//Loop through the appropriate number of Z-counts.

    //printf("bitArcDir=%s\n",bitArcDir);
    //printf("bitarc_number=%d\n",bitarc_number);
	//printf("Zcount=%f\n",Zcount);
    //for (i=0;i<bitarc_number;i++) printf("%d ",arc_gps_sec[i]);
    //printf("arcZcount[7]=%f\n",arcZcount[7]);

    printf("bitarc_length=%d\n", bitarc_length);
    //printf("bitarc_number=%d\n", bitarc_number);

	i=0, j=0;
	while (Zcount < endZcount)
	{
        i=0;
		match_length=0;
		while ( i < bitarc_length )
		{
			//printf("arc_gps_sec=%d\n",arc_gps_sec[i]);
			//printf("arcSOW=%d\n",arcSOW[i]);
			//printf("arcZcount=%f\n",arcZcount[i]);
			if(int(arcZcount[i])==int(Zcount))
			{
				zMatchi[match_length]=i;
				match_length++;
			}
			i++;
		}
		//printf("test%d",zMatchi);
		//printf("match_length=%d\n",match_length);
        //printf("Zcount=%f\n",Zcount);
		//If there are no bits in the archive for the PRN, kill this
        //process and move to the next PRN.
		if (match_length == 0)
		{
			printf("bitArc does not containing data for PRN %02d at zcount = %f.\n",PRN,Zcount);
			printf("The output file for PRN %02d ends at zcount=%f.\n",PRN,Zcount);
			printf("Moving on to next PRN\n\n");
			break;
		}
		else
		{
			for( i=0 ; i<=match_length-1 ; i++)
			strcpy(matchName[i],bitArcFileName[zMatchi[i]]);
			//for(i=0;i<5;i++) printf("%s\n",matchName[i]);
			if (lin) sprintf(bitArcDir,".\\%s\\%02d\\%02d\\", YMD, arcHOD[zMatchi[0]], PRN);
			//else sprintf(bitArcDir,"%s/%s/%02d/%02d/", bitarcName, YMD, arcHOD[zMatchi[0]], PRN);
            else sprintf(bitArcDir,"%s/%02d/%02d/", bitarcName, arcHOD[zMatchi[0]], PRN);

			//Read the bitArc file(s) corresponding to the current Z-count.
			for(lcv=1; lcv <= match_length; lcv++)
			{
				maxCN0=0;
				sprintf(pathname[lcv-1], "%s%s", bitArcDir, matchName[lcv-1]);

				fid = fopen( pathname[lcv-1], "rb");

				fread(hostName[lcv-1],8,1,fid);
				fread(&prn[lcv-1],1,1,fid);
				fread(&CN[lcv-1],1,1,fid);
				fread(&sfMask[lcv-1],1,1,fid);
				fread(&sfParityMask[lcv-1],1,1,fid);
				fread(&frameStart[lcv-1],4,1,fid);
				for (lcv2=1; lcv2 <= subframe_per_frame; lcv2++)
				{
					fread(frame[lcv-1][lcv2-1],4,10,fid);
				}
				fclose(fid);
				//itoa(sfMask[lcv-1], sfMaskb_t[lcv-1], 2);
				unsigned int dec=sfMask[lcv-1];
				int sum=0;
				int rem=0;
				long int k=1;
				do
                {
                    rem=dec%2;
                    sum=sum + (k*rem);
                    dec=dec/2;
                    k=k*10;
                }while(dec>0);
                sprintf(sfMaskb_t[lcv-1],"%d",sum);


				//itoa(sfParityMask[lcv-1], sfParityMaskb_t[lcv-1], 2);
				dec=sfParityMask[lcv-1];
				sum=0;
				rem=0;
				k=1;
				do
                {
                    rem=dec%2;
                    sum=sum + (k*rem);
                    dec=dec/2;
                    k=k*10;
                }while(dec>0);
                sprintf(sfParityMaskb_t[lcv-1],"%d",sum);


				for(i=0 ; i<=7 ; i++)
				{
					if (unsigned(i) < (8-strlen(sfMaskb_t[lcv-1]))) sfMaskb[lcv-1][i] = '0';
					else sfMaskb[lcv-1][i] = sfMaskb_t[lcv-1][i-(8-strlen(sfMaskb_t[lcv-1]))];
				    if (unsigned(i) < (8-strlen(sfParityMaskb_t[lcv-1]))) sfParityMaskb[lcv-1][i] = '0';
					else sfParityMaskb[lcv-1][i] = sfParityMaskb_t[lcv-1][i-(8-strlen(sfMaskb_t[lcv-1]))];
				}


                for(lcv2=1 ; lcv2 <= subframe_per_frame ; lcv2++)
                {
                    strcpy(frameb[lcv2-1][lcv-1],"");
                    for(i=1; i<=10; i++)
                    {
                        char frameb_t[33];
                        char frameb_t1[33];
                        char frameb_t2[]="00000000000000000000000000000000";
                        char frameb_t3[]="000000000000000000000000000000";
                        //itoa(frame[lcv-1][lcv2-1][i-1], frameb_t, 2);
                        dec=frame[lcv-1][lcv2-1][i-1];
                        k=0;
                        do
                        {
                            sprintf(&frameb_t1[k],"%d",dec%2);
                            dec=dec/2;
                            k=k+1;
                        }while(dec>0);
                        for(j=0;j<k;j++) frameb_t[j]=frameb_t1[k-1-j];
                        frameb_t[k]='\0';



                        for(j=32-strlen(frameb_t) ; j<=31 ; j++)
                        {
                            frameb_t2[j]=frameb_t[j+strlen(frameb_t)-32];
                        }
                        for(j=0;j<=29;j++)
                        {
                            frameb_t3[j]=frameb_t2[j+2];
                        }
                        strcat(frameb[lcv2-1][lcv-1],frameb_t3);

                    }
                }
                //Find the frame with greatest CN0.
                if(CN[lcv-1]>maxCN0)
                {
                    maxCN0i=lcv;
                    maxCN0=CN[lcv-1];
                }
			}
            //printf("%d\n",maxCN0i);

			//Check frame presence and parity.
            if (sfMaskb[maxCN0i-1][7]!='1' ||
                sfMaskb[maxCN0i-1][6]!='1' ||
                sfMaskb[maxCN0i-1][5]!='1' ||
                sfMaskb[maxCN0i-1][4]!='1' ||
                sfMaskb[maxCN0i-1][3]!='1')
                {
                    printf("One or more subframes missing in %s. \n", pathname[maxCN0i-1]);
                    printf("Did not find a frame that contained all subframes or \n");
                    printf("passed parity for Z count %f... continuing.\n", Zcount);
                }
            if (sfParityMaskb[maxCN0i-1][7]!='1' ||
                sfParityMaskb[maxCN0i-1][6]!='1' ||
                sfParityMaskb[maxCN0i-1][5]!='1' ||
                sfParityMaskb[maxCN0i-1][4]!='1' ||
                sfParityMaskb[maxCN0i-1][3]!='1')
                {
                    printf("One or more subframes did not pass parity in %s. \n", pathname[maxCN0i-1]);
                    printf("Did not find a frame that contained all subframes or \n");
                    printf("passed parity for Z count %f... continuing.\n", Zcount);
                }

            for (lcv2=1; lcv2<=subframe_per_frame; lcv2++)
            {
                strcpy(COSMICframeBits_t[lcv2-1],frameb[lcv2-1][maxCN0i-1]);
            }

            for (lcv2=1; lcv2<=subframe_per_frame; lcv2++)
            {
                Z = Zcount + (lcv2-1)*4;
                fwrite(&Z,4,1,output_file);
                for (lcv=1; lcv<=Bits_Per_Frame; lcv++)
                {
                    if(COSMICframeBits_t[lcv2-1][lcv-1]=='1') COSMICframeBits[lcv2-1][lcv-1]=-1;
                    else COSMICframeBits[lcv2-1][lcv-1]=1;
                }
                for (lcv=1; lcv<=Bits_Per_Frame; lcv++)
                {
                    fwrite(&COSMICframeBits[lcv2-1][lcv-1],2,1,output_file);
                }
                //printf("Z=%d  ",Z);
            }
            //printf("%s\n ",matchName);
			Zcount = Zcount + 20;
			//printf("%f",Zcount);
		}

	}
    //printf("%d",sizeof(unsigned short));
	fclose(output_file);
	//printf("Zcount_b=%f\n",Zcount);


    //Read the newly generated bit file.
    sprintf(OLBitFileName, "DataBitSV%02d.in", PRN);
    CLB OLBits[610];     // Think of a way to know the size of matrix previously!!!
    fid = fopen(OLBitFileName, "rb");
    i=0;
	if (!fid)
	{
		cout << "Data Bit file " << CLBitFileName << " not found." << endl;
	}
	else
	{
		while (!feof(fid))
		{
			OLBits[i].zcount=0;

			fread(&OLBits[i].zcount, 4, 1, fid);

			for (j = 0; j <= 299; j++)
			{
			    fread(&OLBits[i].databit[j], 2, 1, fid);
			    if (OLBits[i].databit[j]==65535) OLBits[i].databit[j]=-1;
			}

			i=i+1;
			//printf("%X ",DataBits_zcount);
			//printf("%d ",CLBits[i].zcount);
		}
	}
	OLBit_Length = i-1;
	fclose(fid);
    //for (i = 0; i <= 604; i++) printf("%d ",OLBits[i].zcount);

    i=0;
    while (OLBits[i].zcount != CLBits[0].zcount && i <= (OLBit_Length-1))
    {
        i++;
        //printf("OLBits[%d].zcount=%f  ",i,CLBits[0].zcount);
    }
    offset=i;
    //printf("offset=%d\n",offset);
    //printf("OLBit_Length=%d\n",OLBit_Length);
    //printf("CLBits size=%d\n",CLBit_Length);


    // Haven't check the correctness of this part below.
    if (offset != OLBit_Length)
    {
        for (lcv=1; lcv<=CLBit_Length; lcv++)
        {
            if (OLBits[lcv+offset-1].zcount != CLBits[lcv-1].zcount)
            {
                printf("Z count does not match!");
            }
            for (lcv2=1; lcv2<=Bits_Per_Frame; lcv2++)
            {
                if (OLBits[lcv+offset-1].databit[lcv2-1] != CLBits[lcv-1].databit[lcv2-1])
                {
                    bad=bad+1;
                }
                else
                {
                    good=good+1;
                }
            }
        }
        if(bad == 0)
        {
            printf("New bits and CL bits match for PRN %02d\n\n", PRN);
        }
        else
        {
            printf("New bits and CL bits DO NOT match for PRN %2d\n", PRN);
            printf("   There are %d bad, %d good bits.\n", bad, good);
            if (good == 0)
            {
                printf("Flipping OL bits to match CL bits.\n\n");
                output_file = fopen(outfilestr, "wb");

                for (i=1; i<=OLBit_Length; i++)
                {
                    fwrite(&OLBits[i-1].zcount,4,1,output_file);

                    for (lcv=1; lcv<=Bits_Per_Frame; lcv++)
                    {
                        OLBits[i-1].databit[lcv-1] = short(OLBits[i-1].databit[lcv-1]*(-1));
                        fwrite(&OLBits[i-1].databit[lcv-1],2,1,output_file);
                    }
                }
                fclose(output_file);

            }
            else
            {
                printf("Bit data corrupt, OL tracking quality will be degraded.\n\n");
            }
        }
    }
    else
    {
        printf("Zcount of OL file and CL file are not match, no comparison available.\n");
    }

    printf("************* PSRBitGen finished a job *************\n");


}

//void psrbitgen::doy2date()


