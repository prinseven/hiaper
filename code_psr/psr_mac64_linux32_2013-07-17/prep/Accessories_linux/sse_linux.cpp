
#include "Includes.h"
/* This file includes SIMD functions
 *
 * sse_conj - takes complex conjugate of a given data (used in Acquisition)
 * sse_cmul - multiply two complex data
 * sse_cmuls - multiply and shift two complex data (used in Acquisition)
 * sse_cmulsc- Multiply and shift two complex data (used in Acquisition)
 * sse_cacc - complex accumulation function (used in CL and OL Channels)
 * sse_mulc - complex multiplication function (used in CL and OL Channels)
 */

void sse_conj(void *A, int cnt)
{

	int cnt1, cnt2;

	cnt1 = cnt/28;
	cnt2 = cnt - 28*cnt1;

	short M[8] = {1,-1,1,-1,1,-1,1,-1};

	if((int)A%16)
	{

		__asm
		(

			".intel_syntax noprefix			\n\t" //Set up for loop
			"mov edi, [ebp+8]						\n\t"// Address of A	source1
			"mov ecx, [ebp-8]					\n\t"// Counter 1
			"movq mm7, [ebp-28]					\n\t"// Move the multiply thingie
			"movupd xmm7, [ebp-28]					\n\t"// Move the multiply thingie
			"jecxz Z%=						\n\t"

			"L%=:							\n\t"

				"movupd		xmm0, [edi]		\n\t"	//Load from A
				"movupd		xmm1, [edi+16] \n\t"//Load from A
				"movupd		xmm2, [edi+32] \n\t"//Load from A
				"movupd		xmm3, [edi+48] \n\t"//Load from A
				"movupd		xmm4, [edi+64] \n\t"//Load from A
				"movupd		xmm5, [edi+80] \n\t"//Load from A
				"movupd		xmm6, [edi+96] \n\t"//Load from A

				"pmullw		xmm0, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
				"pmullw		xmm1, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
				"pmullw		xmm2, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
				"pmullw		xmm3, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
				"pmullw		xmm4, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
				"pmullw		xmm5, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
				"pmullw		xmm6, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]

				"movupd		[edi],    xmm0	\n\t"//Move into A
				"movupd		[edi+16], xmm1	\n\t"//Move into A
				"movupd		[edi+32], xmm2	\n\t"//Move into A
				"movupd		[edi+48], xmm3	\n\t"//Move into A
				"movupd		[edi+64], xmm4	\n\t"//Move into A
				"movupd		[edi+80], xmm5	\n\t"//Move into A
				"movupd		[edi+96], xmm6	\n\t"//Move into A

				"add		edi, 112		\n\t"	//Move in array

			"loop L%=						\n\t"// Loop if not done
			"Z%=:							\n\t"

			"mov ecx, [ebp-12]				\n\t"// Counter
			"jecxz ZZ%=						\n\t"
			"LL%=:						\n\t"

				"movd	mm0, [edi]			\n\t"
				"pmullw	mm0, mm7			\n\t"
				"movd	[edi], mm0			\n\t"

				"add		edi, 4			\n\t"

			"loop LL%=						\n\t"
			"ZZ%=:							\n\t"

			"EMMS							\n\t"
			".att_syntax					\n\t"
			:
			: "m" (A), "m" (cnt), "m" (cnt1), "m" (cnt2)
			: "%ecx", "%edi"

		);//end __asm
	}
	else
	{
		__asm
				(

					".intel_syntax noprefix			\n\t" //Set up for loop
					"mov edi, [ebp+8]						\n\t"// Address of A	source1
					"mov ecx, [ebp-8]					\n\t"// Counter
					"movq mm7, [ebp-28]					\n\t"// Move the multiply thingie
					"movupd xmm7, [ebp-28]					\n\t"// Move the multiply thingie
					"jecxz Z%=						\n\t"

					"L%=:							\n\t"

						"movapd		xmm0, [edi]		\n\t"	//Load from A
						"movapd		xmm1, [edi+16] \n\t"//Load from A
						"movapd		xmm2, [edi+32] \n\t"//Load from A
						"movapd		xmm3, [edi+48] \n\t"//Load from A
						"movapd		xmm4, [edi+64] \n\t"//Load from A
						"movapd		xmm5, [edi+80] \n\t"//Load from A
						"movapd		xmm6, [edi+96] \n\t"//Load from A

						"pmullw		xmm0, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
						"pmullw		xmm1, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
						"pmullw		xmm2, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
						"pmullw		xmm3, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
						"pmullw		xmm4, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
						"pmullw		xmm5, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]
						"pmullw		xmm6, xmm7		\n\t"//Multiply to get [Re -Im Re -Im]

						"movapd		[edi],    xmm0	\n\t"//Move into A
						"movapd		[edi+16], xmm1	\n\t"//Move into A
						"movapd		[edi+32], xmm2	\n\t"//Move into A
						"movapd		[edi+48], xmm3	\n\t"//Move into A
						"movapd		[edi+64], xmm4	\n\t"//Move into A
						"movapd		[edi+80], xmm5	\n\t"//Move into A
						"movapd		[edi+96], xmm6	\n\t"//Move into A

						"add		edi, 112		\n\t"	//Move in array

					"loop L%=						\n\t"// Loop if not done
					"Z%=:							\n\t"

					"mov ecx, [ebp-12]				\n\t"// Counter
					"jecxz ZZ%=						\n\t"
					"LL%=:						\n\t"

						"movd	mm0, [edi]			\n\t"
						"pmullw	mm0, mm7			\n\t"
						"movd	[edi], mm0			\n\t"

						"add		edi, 4			\n\t"

					"loop LL%=						\n\t"
					"ZZ%=:							\n\t"

					"EMMS							\n\t"
					".att_syntax					\n\t"
					:
					: "m" (A), "m" (cnt), "m" (cnt1), "m" (cnt2)
					: "%ecx", "%edi"
			); //end _asm

	}

}


void sse_cmul(CPX *A, CPX *B, int32 cnt)
{

	int32 cnt1;
	int32 cnt2;

	//short M[8] = {0x0001, 0xffff, 0x0001, 0x0001, 0x0001, 0xffff, 0x0001, 0x0001}; //{1,-1,1,1,1,-1,1,1};
	short M[8] = {1,-1,1,1,1,-1,1,1};

	cnt1 = cnt/4;
	cnt2 = cnt-4*cnt1;

	cnt1 = 0;
	cnt2 = cnt;

	__asm
	(
		".intel_syntax noprefix			\n\t" //Set up for loop
		"mov edi, [ebp+8]				\n\t" //Address of A
		"mov esi, [ebp+12]				\n\t" //Address of B
		"mov ecx, [ebp-12]				\n\t" //Counter 1
		"movupd xmm7,[ebp-32]			\n\t" // Move the multiply thingie
		"jecxz Z%=						\n\t"
		"L%=:							\n\t"
			"movlpd xmm0, [edi]			\n\t" //Copy from A
			"movlpd xmm1, [edi+8]		\n\t" //Copy from A
			"movlpd xmm3, [esi]			\n\t" //Copy from B
			"movlpd xmm4, [esi+8]		\n\t" //Copy from B
			"punpckldq xmm0, xmm0		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm1, xmm1		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm3, xmm3		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm4, xmm4		\n\t" //Copy low 32 bits to high 32 bits
			"pshuflw xmm3, xmm3, 0x14	\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pshuflw xmm4, xmm4, 0x14	\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pshufhw xmm3, xmm3, 0x14	\n\t" //Shuffle High 64 bits to get [Re Im Im Re]
			"pshufhw xmm4, xmm4, 0x14	\n\t" //Shuffle High 64 bits to get [Re Im Im Re]
			"pmullw xmm3, xmm7			\n\t" //Multiply to get [Re Im -Im Re]
			"pmullw xmm4, xmm7			\n\t" //Multiply to get [Re Im -Im Re]
			"pmaddwd xmm0, xmm3			\n\t" //Complex multiply and add
			"pmaddwd xmm1, xmm4			\n\t" //Complex multiply and add
			"packssdw xmm0, xmm0		\n\t" //Get into low 64 bits
			"packssdw xmm1, xmm1		\n\t" //Get into low 64 bits
			"movsd [edi],   xmm0		\n\t" //Move into A
			"movsd [edi+8], xmm1		\n\t" //Move into A
			"add edi, 16				\n\t" //Move in array
			"add esi, 16				\n\t" //Move in array
		"loop L%=						\n\t" // Loop if not done
		"Z%=:							\n\t"
		"mov ecx, [ebp-16]				\n\t"
		"jecxz ZZ%=						\n\t"
		"LL%=:							\n\t"
			"movlpd		xmm0, [edi]		\n\t" //Copy from A
			"movlpd		xmm1, [esi]		\n\t" //Copy from B
			"punpckldq	xmm0, xmm0		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq	xmm1, xmm1		\n\t" //Copy low 32 bits to high 32 bits
			"pshuflw	xmm1, xmm1, 0x14\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pmullw		xmm1, xmm7		\n\t" //Multiply to get [Re Im -Im Re]
			"pmaddwd	xmm0, xmm1		\n\t" //Complex multiply and add
			"packssdw	xmm0, xmm0		\n\t" //Get into low 32 bits
			"movd		[edi], xmm0		\n\t" //Move into A
			"add edi, 4					\n\t"
			"add esi, 4					\n\t"
			"loop LL%=					\n\t"
		"ZZ%=:							\n\t"
		"EMMS							\n\t"
		".att_syntax					\n\t"
		:
		: "m" (A), "m" (B), "m" (cnt), "m" (cnt1), "m" (cnt2)
		: "%ecx", "%edi", "%esi"
	);

}


void sse_cmuls(void *A, void *B, int32 cnt, int32 shift)
{

	int cnt1,cnt2;

	cnt1 = cnt/4;
	cnt2 = cnt-(4*cnt1);

	short M[8] = {1,-1,1,1,1,-1,1,1};
//    volatile short M[8] = {0x0001, 0xffff, 0x0001, 0x0001, 0x0001, 0xffff, 0x0001, 0x0001}; //{1,-1,1,1,1,-1,1,1};

	__asm
	(
		".intel_syntax noprefix			\n\t" //Set up for loop
		"mov edi, [ebp+8]				\n\t" //Address of A
		"mov esi, [ebp+12]				\n\t" //Address of B
		"mov ecx, [ebp-12]				\n\t" //Counter 1
		"movupd xmm7,[ebp-32]			\n\t" //Move the multiply thingie
		"movss  xmm6, [ebp+20]			\n\t" //Move the round thingie
		"jecxz Z%=						\n\t"
		"L%=:							\n\t"
			"movlpd xmm0, [edi]			\n\t" //Copy from A
			"movlpd xmm1, [edi+8]		\n\t" //Copy from A
			"movlpd xmm3, [esi]			\n\t" //Copy from B
			"movlpd xmm4, [esi+8]		\n\t" //Copy from B
			"punpckldq xmm0, xmm0		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm1, xmm1		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm3, xmm3		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm4, xmm4		\n\t" //Copy low 32 bits to high 32 bits
			"pshuflw xmm3, xmm3, 0x14	\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pshuflw xmm4, xmm4, 0x14	\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pshufhw xmm3, xmm3, 0x14	\n\t" //Shuffle High 64 bits to get [Re Im Im Re]
			"pshufhw xmm4, xmm4, 0x14	\n\t" //Shuffle High 64 bits to get [Re Im Im Re]
			"pmullw xmm3, xmm7			\n\t" //Multiply to get [Re Im -Im Re]
			"pmullw xmm4, xmm7			\n\t" //Multiply to get [Re Im -Im Re]
			"pmaddwd xmm0, xmm3			\n\t" //Complex multiply and add
			"pmaddwd xmm1, xmm4			\n\t" //Complex multiply and add
			"psrad xmm0, xmm6			\n\t" //Shift by X bits
			"psrad xmm1, xmm6			\n\t" //Shift by X bits
			"packssdw xmm0, xmm0		\n\t" //Get into low 64 bits
			"packssdw xmm1, xmm1		\n\t" //Get into low 64 bits
			"movsd [edi],   xmm0		\n\t" //Move into A
			"movsd [edi+8], xmm1		\n\t" //Move into A
			"add edi, 16				\n\t" //Move in array
			"add esi, 16				\n\t" //Move in array
		"loop L%=						\n\t" //Loop if not done
		"Z%=:							\n\t"
		"mov ecx, [ebp-16]				\n\t"
		"jecxz ZZ%=						\n\t"
		"LL%=:							\n\t"
			"movlpd		xmm0, [edi]		\n\t" //Copy from A
			"movlpd		xmm1, [esi]		\n\t" //Copy from B
			"punpckldq	xmm0, xmm0		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq	xmm1, xmm1		\n\t" //Copy low 32 bits to high 32 bits
			"pshuflw	xmm1, xmm1, 0x14\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pmullw		xmm1, xmm7		\n\t" //Multiply to get [Re Im -Im Re]
			"pmaddwd	xmm0, xmm1		\n\t" //Complex multiply and add
			"psrad		xmm0, xmm6		\n\t" //Shift by X bits
			"packssdw	xmm0, xmm0		\n\t" //Get into low 32 bits
			"movd		[edi], xmm0		\n\t" //Move into A
			"add edi, 4					\n\t"
			"add esi, 4					\n\t"
			"loop LL%=					\n\t"
		"ZZ%=:							\n\t"
		"EMMS							\n\t"
		".att_syntax					\n\t"
		:
		: "m" (A), "m" (B), "m" (cnt), "m" (cnt1), "m" (cnt2), "m" (shift)
		: "%ecx", "%edi", "%esi"
	);

}



void sse_cmulsc(void *A, void *B, void *C, int32 cnt, int32 shift)
{

	int cnt1, cnt2;

	cnt1 = cnt/4;
	cnt2 = cnt-(4*cnt1);

	short M[8] = {1,-1,1,1,1,-1,1,1}; //{1,-1,1,1,1,-1,1,1};
//	volatile short M[8] = {0x0001, 0xffff, 0x0001, 0x0001, 0x0001, 0xffff, 0x0001, 0x0001}; //{1,-1,1,1,1,-1,1,1};


	__asm
	(
		".intel_syntax noprefix			\n\t" //Set up for loop
		"mov edi, [ebp+8]				\n\t" //Address of A
		"mov esi, [ebp+12]				\n\t" //Address of B
		"mov eax, [ebp+16]				\n\t" //Address of C
		"mov ecx, [ebp-12]				\n\t" //Counter 1
		"movupd xmm7,[ebp-32]			\n\t" //Move the multiply thingie
		"movss  xmm6, [ebp+24]			\n\t" //Move the round thingie
		"jecxz Z%=						\n\t"
		"L%=:							\n\t"
			"movlpd xmm0, [edi]			\n\t" //Copy from A
			"movlpd xmm1, [edi+8]		\n\t" //Copy from A
			"movlpd xmm3, [esi]			\n\t" //Copy from B
			"movlpd xmm4, [esi+8]		\n\t" //Copy from B
			"punpckldq xmm0, xmm0		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm1, xmm1		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm3, xmm3		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq xmm4, xmm4		\n\t" //Copy low 32 bits to high 32 bits
			"pshuflw xmm3, xmm3, 0x14	\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pshuflw xmm4, xmm4, 0x14	\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pshufhw xmm3, xmm3, 0x14	\n\t" //Shuffle High 64 bits to get [Re Im Im Re]
			"pshufhw xmm4, xmm4, 0x14	\n\t" //Shuffle High 64 bits to get [Re Im Im Re]
			"pmullw xmm3, xmm7			\n\t" //Multiply to get [Re Im -Im Re]
			"pmullw xmm4, xmm7			\n\t" //Multiply to get [Re Im -Im Re]
			"pmaddwd xmm0, xmm3			\n\t" //Complex multiply and add
			"pmaddwd xmm1, xmm4			\n\t" //Complex multiply and add
			"psrad xmm0, xmm6			\n\t" //Shift by X bits
			"psrad xmm1, xmm6			\n\t" //Shift by X bits
			"packssdw xmm0, xmm0		\n\t" //Get into low 64 bits
			"packssdw xmm1, xmm1		\n\t" //Get into low 64 bits
			"movsd [eax],   xmm0		\n\t" //Move into A
			"movsd [eax+8], xmm1		\n\t" //Move into A
			"add edi, 16				\n\t" //Move in array
			"add esi, 16				\n\t" //Move in array
			"add eax, 16				\n\t"
		"loop L%=						\n\t" //Loop if not done
		"Z%=:							\n\t"
		"mov ecx, [ebp-16]				\n\t"
		"jecxz ZZ%=						\n\t"
		"LL%=:							\n\t"
			"movlpd		xmm0, [edi]		\n\t" //Copy from A
			"movlpd		xmm1, [esi]		\n\t" //Copy from B
			"punpckldq	xmm0, xmm0		\n\t" //Copy low 32 bits to high 32 bits
			"punpckldq	xmm1, xmm1		\n\t" //Copy low 32 bits to high 32 bits
			"pshuflw	xmm1, xmm1, 0x14\n\t" //Shuffle Low 64 bits to get [Re Im Im Re]
			"pmullw		xmm1, xmm7		\n\t" //Multiply to get [Re Im -Im Re]
			"pmaddwd	xmm0, xmm1		\n\t" //Complex multiply and add
			"psrad		xmm0, xmm6		\n\t" //Shift by X bits
			"packssdw	xmm0, xmm0		\n\t" //Get into low 32 bits
			"movd		[eax], xmm0		\n\t" //Move into A
			"add edi, 4					\n\t"
			"add esi, 4					\n\t"
			"add eax, 4					\n\t"
			"loop LL%=					\n\t"
		"ZZ%=:							\n\t"
		"EMMS							\n\t"
		".att_syntax					\n\t"
		:
		: "m" (A), "m" (B), "m" (C), "m" (cnt), "m" (cnt1), "m" (cnt2), "m" (shift)
		: "%eax", "%ecx", "%edi", "%esi"
	);

}


int64 sse_cacc(void *A, void *B, int cnt)
{

	int cnt1;
	int cnt2;
	int64 result;

	cnt1 = cnt / 6;
	cnt2 = (cnt - (6*cnt1));

	if(((int)A%16) || ((int)B%16))
	{

		__asm
		(

			".intel_syntax noprefix			\n\t" 	//Set up for loop
			"mov edi, [ebp+8]				\n\t"	// Address of A
			"mov esi, [ebp+12]				\n\t"	// Address of B
			"mov ecx, [ebp-12]				\n\t"	// Counter
			"pxor xmm0, xmm0				\n\t"	// Clear the running sum
			"pxor mm0, mm0					\n\t"	// Clear the running sum
			"jecxz Z%=						\n\t"

			"L%=:							\n\t"

				"movlpd xmm1, [edi]			\n\t"	//load IF data
				"movlpd xmm2, [edi+8]		\n\t"	//load IF data
				"movlpd xmm3, [edi+16]		\n\t"	//load IF data

				"movupd xmm4, [esi]			\n\t"	//load Sine data
				"movupd xmm5, [esi+16]		\n\t"	//load Sine data
				"movupd xmm6, [esi+32]		\n\t"	//load Sine data

				"punpckldq xmm1, xmm1		\n\t"	//copies bits 0..31 to 32..63 and bits 32..63 to 64..95 and 65..127
				"punpckldq xmm2, xmm2		\n\t"	//copies bits 0..63 to 64..127
				"punpckldq xmm3, xmm3		\n\t"	//copies bits 0..63 to 64..127

				"pmaddwd	xmm1, xmm4		\n\t"	//multiply and add, result in xmm1
				"pmaddwd xmm2, xmm5			\n\t"	//multiply and add, result in xmm2
				"pmaddwd	xmm3, xmm6		\n\t"	//multiply and add, result in xmm3

				"paddd xmm0, xmm3			\n\t"	//Add into accumulator (efficiently)
				"paddd xmm1, xmm2			\n\t"
				"paddd xmm0, xmm1			\n\t"

				"add edi, 24				\n\t"	//move in complex sine by 24 bytes
				"add esi, 48				\n\t"	//move in IF array by 48 bytes

			"loop L%=						\n\t"	// Loop if not done

	"Z%=:						\n\t"
			"mov ecx, [ebp-16]					\n\t"
			"jecxz ZZ%=						\n\t"

			"LL%=:							\n\t"

				"movq		mm1, [edi]		\n\t"	//load IF data

				"punpckldq	mm1, mm1		\n\t"	//copy bottom 32 bits of IF data into high 32 bits
				"pmaddwd	mm1, [esi]		\n\t"	//perform mmx complex multiply
				"paddd		mm0, mm1		\n\t"	//add into accumulator

				"add edi, 4					\n\t"	//move in complex sine by 4 bytes
				"add esi, 8					\n\t"	//move in IF array by 8 bytes

			"loop LL%=						\n\t"

	"ZZ%=:					\n\t"


			"movdq2q		mm1, xmm0		\n\t"
			"punpckhqdq 	xmm0, xmm0		\n\t"	//move bits 64..127 of xmm0 into 0..63 of xmm0
			"movdq2q		mm2, xmm0		\n\t"

			"paddd		mm0, mm1			\n\t"	//add together
			"paddd		mm0, mm2			\n\t"	//add together

			"movq		[ebp-24], mm0		\n\t"  //move in result

			"EMMS							\n\t"	// done with MMX
			".att_syntax					\n\t"
			: "=m"(result)
			: "m" (A), "m" (B), "m" (cnt), "m" (cnt1), "m" (cnt2)
			: "%ecx", "%edi", "%esi"

		);//end __asm
	}
	else
	{

		__asm
		(

			".intel_syntax noprefix			\n\t" 	//Set up for loop
			"mov edi, [ebp+8]				\n\t"	// Address of A
			"mov esi, [ebp+12]				\n\t"	// Address of B
			"mov ecx, [ebp-12]				\n\t"	// Counter
			"pxor xmm0, xmm0				\n\t"	// Clear the running sum
			"pxor mm0, mm0					\n\t"	// Clear the running sum
		"jecxz Z%=						\n\t"

			"L%=:							\n\t"

			"movlpd xmm1, [edi]			\n\t"	//load IF data
			"movlpd xmm2, [edi+8]		\n\t"	//load IF data
			"movlpd xmm3, [edi+16]		\n\t"	//load IF data

			"movapd xmm4, [esi]			\n\t"	//load Sine data
			"movapd xmm5, [esi+16]		\n\t"	//load Sine data
			"movapd xmm6, [esi+32]		\n\t"	//load Sine data

			"punpckldq xmm1, xmm1		\n\t"	//copies bits 0..31 to 32..63 and bits 32..63 to 64..95 and 65..127
			"punpckldq xmm2, xmm2		\n\t"	//copies bits 0..63 to 64..127
			"punpckldq xmm3, xmm3		\n\t"	//copies bits 0..63 to 64..127

			"pmaddwd	xmm1, xmm4		\n\t"	//multiply and add, result in xmm1
			"pmaddwd xmm2, xmm5			\n\t"	//multiply and add, result in xmm2
			"pmaddwd	xmm3, xmm6		\n\t"	//multiply and add, result in xmm3

			"paddd xmm0, xmm3			\n\t"	//Add into accumulator (efficiently)
			"paddd xmm1, xmm2			\n\t"
			"paddd xmm0, xmm1			\n\t"

			"add edi, 24				\n\t"	//move in complex sine by 24 bytes
			"add esi, 48				\n\t"	//move in IF array by 48 bytes

			"loop L%=					\n\t"	// Loop if not done

		"Z%=:						\n\t"
			"mov ecx, [ebp-16]			\n\t"
		"jecxz ZZ%=						\n\t"

			"LL%=:						\n\t"

			"movq		mm1, [edi]		\n\t"	//load IF data

			"punpckldq	mm1, mm1		\n\t"	//copy bottom 32 bits of IF data into high 32 bits
			"pmaddwd	mm1, [esi]		\n\t"	//perform mmx complex multiply
			"paddd		mm0, mm1		\n\t"	//add into accumulator

			"add edi, 4					\n\t"	//move in complex sine by 4 bytes
			"add esi, 8					\n\t"	//move in IF array by 8 bytes

			"loop LL%=						\n\t"

		"ZZ%=:					\n\t"


			"movdq2q		mm1, xmm0		\n\t"
			"punpckhqdq 	xmm0, xmm0		\n\t"	//move bits 64..127 of xmm0 into 0..63 of xmm0
			"movdq2q		mm2, xmm0		\n\t"

			"paddd		mm0, mm1			\n\t"	//add together
			"paddd		mm0, mm2			\n\t"	//add together
			"movq		[ebp-24], mm0		\n\t"

			"EMMS							\n\t"	// done with MMX

			".att_syntax					\n\t"
			: "=m"(result)
			: "m" (A), "m" (B), "m" (cnt), "m" (cnt1), "m" (cnt2)
			: "%ecx", "%edi", "%esi"

		);//end __asm
	}//end if

	return(result);

}

void sse_mulc(void *A, void *B, void *C, int cnt)
{

	int cnt1;
	int cnt2;


	cnt1 = cnt / 8;
	cnt2 = (cnt-(8*cnt1));

	if(((int)A%16) || ((int)B%16) || ((int)C%16))
	{

		__asm
		(

			".intel_syntax noprefix			\n\t" 	//Set up for loop
			"mov edi, [ebp+8]				\n\t"	// Address of A, input1
			"mov esi, [ebp+12]				\n\t"	// Address of B, input2
			"mov ebx, [ebp+16]				\n\t"	// Address of C, output1
			"mov ecx, [ebp-12]				\n\t"	// Counter

			"jecxz Z%=						\n\t"

			"L%=:							\n\t"

				"movupd xmm0, [edi]			\n\t"	//Load from A
				"movupd xmm1, [esi]			\n\t"	//Load from B
				"pmullw  xmm0, xmm1			\n\t"	//Multiply A*B
				"movupd [ebx], xmm0			\n\t"	//Move into C
				"add edi, 16				\n\t"
				"add esi, 16				\n\t"
				"add ebx, 16				\n\t"

			"loop L%=					\n\t"

	"Z%=:					\n\t"

			"mov ecx, [ebp-16]			\n\t"
			"jecxz ZZ%=				\n\t"

			"mov eax, 0					\n\t"

			"LL%=:						\n\t"	//Really finish off loop with non SIMD instructions

				"mov ax, [edi]			\n\t"
				"imul ax, [esi]			\n\t"
				"mov [ebx], ax			\n\t"
				"add esi, 2				\n\t"
				"add edi, 2				\n\t"
				"add ebx, 2				\n\t"

			"loop LL%=					\n\t"

	"ZZ%=:						\n\t"

		"EMMS							\n\t"	// done with MMX
		".att_syntax					\n\t"
		:
		: "m" (A), "m" (B), "m" (C), "m" (cnt), "m" (cnt1), "m" (cnt2)
		: "%eax", "%ebx", "%ecx", "%edi", "%esi"

		);//end __asm
	}
		else
		{
			__asm
					(

						".intel_syntax noprefix			\n\t" 	//Set up for loop
						"mov edi, [ebp+8]				\n\t"	// Address of A, input1
						"mov esi, [ebp+12]				\n\t"	// Address of B, input2
						"mov ebx, [ebp+16]				\n\t"	// Address of C, output1
						"mov ecx, [ebp-12]				\n\t"	// Counter

						"jecxz Z%=						\n\t"

						"L%=:							\n\t"

							"movapd xmm0, [edi]			\n\t"	//Load from A
							"movapd xmm1, [esi]			\n\t"	//Load from B
							"pmullw  xmm0, xmm1			\n\t"	//Multiply A*B
							"movapd [ebx], xmm0			\n\t"	//Move into C
							"add edi, 16				\n\t"
							"add esi, 16				\n\t"
							"add ebx, 16				\n\t"

						"loop L%=					\n\t"

				"Z%=:					\n\t"

						"mov ecx, [ebp-16]			\n\t"
						"jecxz ZZ%=				\n\t"

						"mov eax, 0					\n\t"

						"LL%=:						\n\t"	//Really finish off loop with non SIMD instructions

							"mov ax, [edi]			\n\t"
							"imul ax, [esi]			\n\t"
							"mov [ebx], ax			\n\t"
							"add esi, 2				\n\t"
							"add edi, 2				\n\t"
							"add ebx, 2				\n\t"

						"loop LL%=					\n\t"

				"ZZ%=:						\n\t"

					"EMMS							\n\t"	// done with MMX
					".att_syntax					\n\t"
					:
					: "m" (A), "m" (B), "m" (C), "m" (cnt), "m" (cnt1), "m" (cnt2)
					: "%eax", "%ebx", "%ecx", "%edi", "%esi"

					);//end __asm
		}
}



