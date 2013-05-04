#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <complex>
#include "wave.h"

using namespace std;

int const block_size = 4096;
int const MAX_BLOCK_SIZE = 4096;

void createDCT(double *dct, int N);
void transformBlock(double *dct, uint32* block, double *leftOut, double *rightOut, int N);
void DCTTransformBlock(uint32* block, double *leftOut, double *rightOut, int N);
void inverseTransformBlock(double *dct, double *in, double *out, int N);
void slowDCT(uint32* block, complex<double> *out, int N);
void fastDCT(uint32 *in, complex<double> *out, int N);
void* fft(void *params);
void initFFT(int N = block_size*4);
void fastDCT(uint32 *in, double *out, int N, int bits);

struct fft_params{
  fft_params(complex<double>* a, int N, int bits, bool reordered) : a(a), N(N), bits(bits), reordered(reordered){}
  complex<double> * a;
  int N, bits;
  bool reordered;
};

#endif
