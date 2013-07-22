#include "FFT.h"


void bfly(void *_A, void *_B, void *_W)__attribute__ ((noinline));
void bflydf(void *_A, void *_B, void *_W)__attribute__ ((noinline));


void bfly(void *_A, void *_B, void *_W)
{

	__asm
	(
		".intel_syntax noprefix		\n\t"
		"mov		ebx, [ebp+8]		\n\t"
		"mov		edi, [ebp+12]		\n\t"
		"mov		esi, [ebp+16]		\n\t"

		"movd		mm0, [ebx]	\n\t"	/*move 32 bits from A to bottom 32 bits of mm0*/
		"movd		mm1, [edi]	\n\t"	/*move 32 bits from B to bottom 32 bits of mm1*/
		"movq		mm2, [esi]	\n\t"	/*move 64 bits from W to mm2*/
		"psraw		mm0, (0x1)	\n\t"
		"psraw		mm1, (0x1)	\n\t"
		"movq		mm3, mm0	\n\t"	/*copy A to mm3*/
		"punpckldq	mm1, mm1	\n\t"	/*copy bottom 32 bits of B data into high 32 bits*/
		"pmaddwd	mm1, mm2	\n\t"   /*complex multiply, real now 0..31 of mm1, imag 32..63 of mm1*/
		"psrad		mm1, (0xf)	\n\t"	/*right shift 0..31 by 16, 32..63 by 16*/
		"packssdw	mm1, mm1	\n\t"	/*pack bits 0..31 to 0..16, bits 32..63 to  16..31*/
		"paddw		mm0, mm1	\n\t"
		"psubw		mm3, mm1	\n\t"
		"movd		[ebx], mm0	\n\t"
		"movd		[edi], mm3	\n\t"

		"EMMS					\n\t"
		".att_syntax				\n\t"
		:
		: "m" (_A), "m" (_B), "m" (_W)
		:"%ebx", "%edi", "%esi"
	);
}

void bflydf(void *_A, void *_B, void *_W)
{

	__asm
	(
		".intel_syntax noprefix		\n\t"
		"mov		ebx, [ebp+8]		\n\t"
		"mov		edi, [ebp+12]		\n\t"
		"mov		esi, [ebp+16]		\n\t"

		"movd		mm0, [ebx]	\n\t"	/*move 32 bits from A to bottom 32 bits of mm0*/
		"movd		mm1, [edi]	\n\t"	/*move 32 bits from B to bottom 32 bits of mm1*/
		"movq		mm2, [esi]	\n\t"	/*move 64 bits from W to mm2*/
		"psraw		mm0, (0x1)	\n\t"
		"psraw		mm1, (0x1)	\n\t"
		"movq		mm3, mm0	\n\t"	/*copy A to mm3*/
		"paddw		mm0, mm1	\n\t"	/*A+B*/
		"psubw		mm3, mm1	\n\t"	/*A-B*/
		"punpckldq	mm3, mm3	\n\t"	/*copy bottom 32 bits of B data into high 32 bits*/
		"pmaddwd	mm3, mm2	\n\t"   /*complex multiply, real now 0..31 of mm1, imag 32..63 of mm1*/
		"psrad		mm3, (0xf)	\n\t"	/*right shift 0..31 by 16, 32..63 by 16*/
		"packssdw	mm3, mm3	\n\t"	/*pack bits 0..31 to 0..16, bits 32..63 to  16..31*/
		"movd		[ebx], mm0	\n\t"
		"movd		[edi], mm3	\n\t"

		"EMMS					\n\t"
		".att_syntax				\n\t"
		:
		: "m" (_A), "m" (_B), "m" (_W)
		:"%ebx", "%edi", "%esi"
	);
}


FFT::FFT()
{

	N = 0;

}


FFT::FFT(int _N)
{

	N = 0;

	if(_N > 20)
	{
		while(_N >>= 1)
			N++;
	}
	else
		N = _N;

	L = pow(2.0, N);

	W = (int64 *)malloc(L/2*sizeof(int64));  // forward twiddle lookup
	iW = (int64 *)malloc(L/2*sizeof(int64)); // inverse twiddle lookup
	BR  = (int *)malloc(L *sizeof(int));  //shuffle
	BRX  = (int *)malloc(L *sizeof(int)); //shuffle

	initW();
	initBR();
}


FFT::~FFT()
{
	free(BRX);
	free(BR);
	free(W);
	free(iW);
}

void FFT::initW()
{

	int lcv;
	short *p;
	double s;
	double c;
	double phase;

    const double pi=3.14159265358979323846264338327;

	for(lcv = 0; lcv < L/2; lcv++)
	{
		//Forward twiddles
		p =     (short *)&W[lcv];
		phase = (-2*pi*lcv)/L;
		c =		cos(phase);
		s =		sin(phase);
		p[0] = (short)(c*32767);
		p[1] = (short)(-s*32767);
		p[2] = (short)(s*32767);
		p[3] = (short)(c*32767);

		//Reverse twiddles
		p =     (short *)&iW[lcv];
		phase = (2*pi*lcv)/L;
		c =		cos(phase);
		s =		sin(phase);
		p[0] = (short)(c*32767);
		p[1] = (short)(-s*32767);
		p[2] = (short)(s*32767);
		p[3] = (short)(c*32767);

	}

}




void FFT::initBR()
{
	int lcv, lcv2, index;

	for(lcv = 0; lcv < L; lcv++)
	{
		index = 0;
		for(lcv2 = 0; lcv2 < N; lcv2++)
		{
			index += ((lcv >> lcv2) & 0x1);
			index <<= 1;
		}
		index >>= 1;

		BR[lcv] = index;
	}

}

void FFT::doFFT(int *_x, bool _shuf)
{

	int lcv, lcv2, lcv3;
	int bsize, nblocks;
	int *a, *b;
	int64 *w;

	if(_shuf)
		doShuffle(_x);	//bit reverse the array

	bsize = 1;
	nblocks = L >> 1;

	for(lcv = 0; lcv < N; lcv++)					//Loop over N stages
	{

		a = _x;
		b = _x+bsize;
		w = W;		

		for(lcv2 = 0; lcv2 < nblocks; lcv2++)		//Loop over blocks
		{

	        for(lcv3 = 0; lcv3 < bsize; lcv3++)		//Butterflies within block
			{
				bfly(a,b,w);
				w += nblocks;
				a++; b++;
			}
			a += bsize;
			b += bsize;
			w = W;
		}

		bsize <<= 1;
		nblocks >>= 1;

	}

}


void FFT::doiFFT(int *_x, bool _shuf)
{

	int lcv, lcv2, lcv3;
	int *a, *b;
	int64 *w;

	int bsize, nblocks;

	if(_shuf)
		doShuffle(_x);	//bit reverse the array


	

	bsize = 1;
	nblocks = L >> 1;

	for(lcv = 0; lcv < N; lcv++)					//Loop over N stages
	{

		a = _x;
		b = _x+bsize;
		w = iW;		

		for(lcv2 = 0; lcv2 < nblocks; lcv2++)		//Loop over blocks
		{

	        for(lcv3 = 0; lcv3 < bsize; lcv3++)		//Butterflies within block
			{
				bfly(a,b,w);
				w += nblocks;
				a++; b++;
			}
			a += bsize;
			b += bsize;
			w = iW;
		}

		bsize <<= 1;
		nblocks >>= 1;

	}

}


void FFT::doFFTdf(int *_x, bool _shuf)
{


	int lcv, lcv2, lcv3;
	int bsize, nblocks;
	int *a, *b;
	int64 *w;

	bsize = L >> 1;
	nblocks = 1;

	for(lcv = 0; lcv < N; lcv++)					//Loop over N stages
	{

		a = _x;
		b = _x+bsize;
		w = W;		

		for(lcv2 = 0; lcv2 < nblocks; lcv2++)		//Loop over blocks
		{
	        for(lcv3 = 0; lcv3 < bsize; lcv3++)		//Butterflies within block
			{
				bflydf(a,b,w);
				w += nblocks;
				a++; b++;
			}
			a += bsize;
			b += bsize;
			w = W;
		}

		bsize >>= 1;
		nblocks <<= 1;

	}

	if(_shuf)
		doShuffle(_x);	//bit reverse the array


}


void FFT::doiFFTdf(int *_x, bool _shuf)
{


	int lcv, lcv2, lcv3;
	int bsize, nblocks;
	int *a, *b;
	int64 *w;

	bsize = L >> 1;
	nblocks = 1;

	for(lcv = 0; lcv < N; lcv++)					//Loop over N stages
	{

		a = _x;
		b = _x+bsize;
		w = iW;		

		for(lcv2 = 0; lcv2 < nblocks; lcv2++)		//Loop over blocks
		{
	        for(lcv3 = 0; lcv3 < bsize; lcv3++)		//Butterflies within block
			{
				bflydf(a,b,w);
				w += nblocks;
				a++; b++;
			}
			a += bsize;
			b += bsize;
			w = W;
		}

		bsize >>= 1;
		nblocks <<= 1;

	}

	if(_shuf)
		doShuffle(_x);	//bit reverse the array


}

void FFT::doShuffle(int *_x)
{

	int lcv;

	memcpy(BRX, _x, L*sizeof(int));

	for(lcv = 0; lcv < L; lcv++)
		_x[lcv] = BRX[BR[lcv]];

}
