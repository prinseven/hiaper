#ifndef INCLUDES_H
#define INCLUDES_H

// system includes
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stddef.h>
#include <float.h>

/*Herein Lies Many Important Files*/
/*Note thy order is important!*/
/********************************************************************************/
#include "Defines.h"
#include "CommonDefines.h"
#include "Structs.h"
#include "Protos.h"
#include "macros.h"
/********************************************************************************/

/* Class definitions */
/********************************************************************************/
#include "simd.h"
#include "FFT.h"
//#include "Channel.h"
//#include "OLChannel.h"
//#include "OLChanBck.h"
#include "Acquisition.h"
#include "Open_Loop.h"
//#include "psrbitgen.h"
/********************************************************************************/

// Global object definitions
#include "Globals.h"

#endif // INCLUDES_H
