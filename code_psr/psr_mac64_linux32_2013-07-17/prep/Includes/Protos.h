#ifndef PROTOS_H
#define PROTOS_H

/* Init Protos */
/********************************************************************************/
void Parse_Arguments(int32 _argc, char* _argv[]);	//!< Parse command line arguments to setup functionality
void Object_Init(void);
void L1_PRNGen();	//generate PRN codes
/********************************************************************************/

/*Matrix  */
/********************************************************************************/
int Invert(double**, double **, int dimension);
double **Mult(double** A, int A_rows, int A_cols,double** B, int B_rows, int B_cols);
double **Transpose(double**, int rows, int cols);
/********************************************************************************/

/* Important control Globals */
/********************************************************************************/
bool getRUN();
void setRUN(bool);
/********************************************************************************/
/* Found in x86.cpp */
/*----------------------------------------------------------------------------------------------*/
void  x86_crot(CPX *_A, CPX *_B, int _cnt);  //rotate
void  x86_conj(DATA_CPX *A, int cnt);											//!< Pointwise vector conjugate
void  x86_cmul(DATA_CPX *A, DATA_CPX *B, int cnt);									//!< Pointwise vector multiply
void  x86_cmuls(DATA_CPX *A, DATA_CPX *B, int cnt, int shift);				//!< Pointwise complex multiply with shift
void  x86_cmulsc(DATA_CPX *A, DATA_CPX *B, DATA_CPX *C, int cnt, int shift);		//!< Pointwise vector multiply with shift, dump results into C	
void x86_cmuln(DATA_TYPE *_A, DATA_CPX *_B, DATA_CORR *_C, int _cnt);
void x86_cmulc(CPX *_A, CPX *_B, CPX *_C, int _cnt);
CPX_ACCUM x86_cacc(DATA_CORR *_A, DATA_TYPE *_B, int _cnt);
CPX_ACCUM x86_cacc2(DATA_CORR *_A, cpx_sine *_B, int _cnt);
/*----------------------------------------------------------------------------------------------*/

#endif // PROTOS_H
