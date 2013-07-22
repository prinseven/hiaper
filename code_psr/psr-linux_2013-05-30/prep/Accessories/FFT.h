#ifndef FFT_H
#define FFT_H
#include "Includes.h"

class FFT
{


	private:

		int64 *W;					//Twiddle lookup array for FFT
		int64 *iW;				//Twiddle lookup array for iFFT
		int *BR;					//Re-order index array
		int *BRX;					//Re-order temp array

		int N;						//Power
		int L;						//Length = 2^N

		void initW();				//Initialize twiddles
		void initBR();				//Initialize re-order array
		void doShuffle(int * _x);	//Do bit-reverse shuffling

	public:

		FFT();								//Initialize FFT
		FFT(int _N);						//Initialize FFT for 2^N
		~FFT();								//Destructor
		void doFFT(int *_x, bool _shuf);	//Forward FFT, decimate in time
		void doiFFT(int *_x, bool _shuf);	//Inverse FFT, decimate in time
		void doFFTdf(int *_x, bool _shuf);	//Forward FFT, decimate in frequency
		void doiFFTdf(int *_x, bool _shuf);	//Inverse FFT, decimate in frequency

};

#endif // FFT_H
