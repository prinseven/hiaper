#ifndef ACQUISITION_H
#define ACQUISITION_H

#include "Includes.h"


class Acquisition
{



	private:

		//FFT *pFFT;
		DATA_CPX *FFT_PRN[NUM_CODES];	//fft of PRN codes
		DATA_CPX *FFT_S2;
		DATA_CPX **sines;
		DATA_CPX *data_buff;

		int *power;						//Incoherent accumulator (Re^2+Im^2 only)
		int *re_cb;						//Real coherent accumulator
		int *im_cb;						//Imaginary coherent accumulator

		//Control Variables
		Acq_Options Opt;

	public:

		Acquisition();
		Acquisition(Acq_Options _Opt);											//Initialize the acquisition
		~Acquisition();
		void Cold_Acquire(void *data, Acq_Result *_results);						//Acquire all 32 PRNs
		
};
#endif // ACQUISITION_H


