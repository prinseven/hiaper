#include "Includes.h"


/*----------------------------------------------------------------------------------------------*/
void x86_conj(CPX *_A, int _cnt)
{
	int lcv;

	for(lcv = 0; lcv < _cnt; lcv++)
		_A[lcv].i = -_A[lcv].i;

}
/*----------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------*/
void x86_cmul(CPX *_A, CPX *_B, int _cnt)
{

	int lcv;
	int ai, aq;
	int bi, bq;
	int ti, tq;

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ai = _A[lcv].r;
		aq = _A[lcv].i;
		bi = _B[lcv].r;
		bq = _B[lcv].i;

		ti = ai*bi-aq*bq;
		tq = ai*bq+aq*bi;

		_A[lcv].r = (short)(ti);
		_A[lcv].i = (short)(tq);
	}

}
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
void x86_cmuln(DATA_TYPE *_A, DATA_CPX *_B, DATA_CORR *_C, int _cnt)
{

	int lcv;
	int ar,ai;
	int br, bi;
	int tr, ti;

	for(lcv = 0; lcv < _cnt; lcv++)
	{
		ar = _A[2*lcv];
		ai = _A[2*lcv+1];
		br = _B[lcv].r;
		bi = _B[lcv].i;

		tr = ar*br;
		ti = ai*bi;

		_C[lcv].rr = (short)(tr);
		_C[lcv].ii = (short)(ti);
	}

}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
void x86_crot(CPX *_A, CPX *_B, int _cnt)
{

	int lcv;
	int ar, ai;
	int br, bi;
	int tr, ti;

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ar = _A[lcv].r;
		ai = _A[lcv].i;
		br = _B[0].r;
		bi = _B[0].i;

		tr = ar*br-ai*bi;
		ti = ar*bi+ai*br;


		_A[lcv].r = (short)(tr);
		_A[lcv].i = (short)(ti);
	}

}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
void x86_cmuls(CPX *_A, CPX *_B, int _cnt, int _shift)
{

	int lcv;
	int ai, aq;
	int bi, bq;
	int ti, tq;
	int shift;
	int round;

	shift = _shift;
	round = 1 << (shift-1);

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ai = _A[lcv].r;
		aq = _A[lcv].i;
		bi = _B[lcv].r;
		bq = _B[lcv].i;

		ti = ai*bi-aq*bq;
		tq = ai*bq+aq*bi;

		ti += round;
		tq += round;

		ti >>= shift;
		tq >>= shift;

		_A[lcv].r = (short)ti;
		_A[lcv].i = (short)tq;
	}

}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
void x86_cmulsc(CPX *_A, CPX *_B, CPX *_C, int _cnt, int _shift)
{

	int lcv;
	int ai, aq;
	int bi, bq;
	int ti, tq;
	int shift;
	int round;

	shift = _shift;
	round = 1 << (shift-1);

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ai = _A[lcv].r;
		aq = _A[lcv].i;
		bi = _B[lcv].r;
		bq = _B[lcv].i;

		ti = ai*bi-aq*bq;
		tq = ai*bq+aq*bi;

		ti += round;
		tq += round;

		ti >>= shift;
		tq >>= shift;

		_C[lcv].r = (short)ti;
		_C[lcv].i = (short)tq;
	}

}
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
void x86_cmulc(CPX *_A, CPX *_B, CPX *_C, int _cnt)
{

	int lcv;
	int ai, aq;
	int bi, bq;
	int ti, tq;

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ai = _A[lcv].r;
		aq = _A[lcv].i;
		bi = _B[lcv].r;
		bq = _B[lcv].i;

		ti = ai*bi-aq*bq;
		tq = ai*bq+aq*bi;


		_C[lcv].r = (short)ti;
		_C[lcv].i = (short)tq;
	}

}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
CPX_ACCUM x86_cacc(DATA_CORR *_A, DATA_TYPE *_B, int _cnt)
{


	int lcv;
	int ar, ai;
	int br, bi;
	int tr, ti;
	int raccum, iaccum;
	CPX_ACCUM value;
	raccum = iaccum = 0;

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ar = _A[lcv].rr;
		ai = _A[lcv].ii;
		br = _B[4*lcv];
		bi = _B[4*lcv+2];

		tr = ar*br-ai*bi;
		ti = ar*bi+ai*br;

		raccum += tr;
		iaccum += ti;

	}

	value.i=raccum;
	value.q=iaccum;
	return(value);

}
/*----------------------------------------------------------------------------------------------*/
CPX_ACCUM x86_cacc2(DATA_CORR *_A, cpx_sine *_B, int _cnt)
{


	int lcv;
	int ar, ai;
	int br, bi;
	int tr, ti;
	int raccum, iaccum;
	CPX_ACCUM value;
	raccum = iaccum = 0;

	for(lcv = 0; lcv < _cnt; lcv++)
	{

		ar = _A[lcv].rr;
		ai = _A[lcv].ii;
		br = _B[lcv].r1;
		bi = _B[lcv].ni;

		tr = ar*br-ai*bi;
		ti = ar*bi+ai*br;

		raccum += tr;
		iaccum += ti;

	}

	value.i=raccum;
	value.q=iaccum;
	return(value);

}
