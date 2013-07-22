#ifndef PSRBITGEN_H
#define PSRBITGEN_H
#endif

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <stddef.h>
#include <dirent.h>

#define Sec_Per_Week       (604800)  // Secs per GPS_Week
#define	Sec_Per_Frame      (30)      // Secs per frame
#define	nameLength         (34)      // Number of chars in bitArc file name
#define	subframe_per_frame (5)       // Subframes per frame
#define	Bits_Per_Frame     (300)     // Bits per frame

class psrbitgen
{
    private:

    //Constants
	/*int Sec_Per_Week;         // Secs per GPS_Week
	int Sec_Per_Frame;            // Secs per frame
	int nameLength;              // Number of chars in bitArc file name
	int subframe_per_frame;        // Subframes per frame
	int Bits_Per_Frame;          // Bits per frame
	int i;
	int j;*/





	public:

	psrbitgen();
	void bitgen(int PRN, int startSOW, int GPS_Week, int Year, int Month, int Day, int File_Len_Sec, int lin, const char *bitarcName);
	//void bitgen(int PRN, int startSOW, int GPS_Week, int Year, int Month, int Day, int File_Len_Sec, int lin);

};
