#include "transform.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <bitset>
#include <pthread.h>

#define PI 3.14159265359

#define rep(i, a, b) for(unsigned int i = (a); i < (unsigned int)(b); ++i)
using namespace std;

void createDCT(double *dct, int N){
  rep(l, 0, N){
    dct[l] = sqrt(1.0/N);
  }
  rep(k, 1, N){
    rep(l, 0, N){
      dct[k*N + l] = sqrt(2.0/N) * cos(PI*k/(2*N) * (2*l+1));
    }
  }
}

void transformBlock(double *dct, uint32* block, double *leftOut, double *rightOut, int N){
  rep(k, 0, N){
    leftOut[k] = 0;
    rightOut[k] = 0;
  }
  rep(k, 0, N){
    rep(l, 0, N){
      sample left = getLeftSample(block[l]);
      sample right = getRightSample(block[l]);
      leftOut[k] += dct[k*N + l]*left;
      rightOut[k] += dct[k*N + l]*right;
    }
  }
}
void inverseTransformBlock(double *dct, double *in, double *out, int N){
  rep(k, 0, N){
    out[k] = 0;
  }
  rep(k, 0, N){
    rep(l, 0, N){
      out[l] += dct[k*N + l]*round(in[k]); // Make sure to traverse dct in order to not lose speed from cache.
    }
  }
}

complex<double> const j(0, 1);
complex<double> fftTwiddle[block_size*4][14];

void initFFT(int N){
  // Precompute Twiddle factors.
  int i = 0, n = 1;
  do{
    n *= 2;
    rep(k, 0, N)
      fftTwiddle[k][i] = exp(-2.0*j*(PI*k)/(double)n);
    ++i;
  }while(n != N);

}

unsigned int const bb[16] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
inline unsigned int reverseBits(unsigned int w, int n){
  return ((bb[0xf & w] << 28) | 
         (bb[0xf & (w>>4)] << 24) |
         (bb[0xf & (w>>8)] << 20) | 
         (bb[0xf & (w>>12)] << 16) | 
         (bb[0xf & (w>>16)] << 12) | 
         (bb[0xf & (w>>20)] << 8) | 
         (bb[0xf & (w>>24)] << 4) | 
         (bb[0xf & (w>>28)])) >> (32-n);
}

void reversedBitReorder(complex<double>* a, int n, int bits){
  complex<double> t;
  unsigned int r;
  rep(i, 0, n){
    if(i < (r = reverseBits(i, bits))){
      t = a[i];
      a[i] = a[r];
      a[i] = t;
    }
  }
}
      

void* fft(void * p){
  fft_params *params = (fft_params*) p;
  unsigned int n = params->N, bits = params->bits, steps = 1, count = 0;
  complex<double> *a = params->a;
  if(!params->reordered) reversedBitReorder(a, n, bits);
  while(n > 1){
    n = n/2;
    steps *= 2;
    rep(i, 0, n){
      rep(k, 0, steps/2){
        int kpos = i*steps+k;
        complex<double> t = a[kpos];
        a[kpos] = t + a[kpos+steps/2]* fftTwiddle[k][count];// exp(-(2.0*j*PI*(double)(k))/(double)(steps));
        a[kpos + steps/2] = t - a[kpos+steps/2]* fftTwiddle[k][count];//exp(-(2.0*j*PI*(double)(k))/(double)(steps));
        //a[kpos] = t + a[kpos+steps/2]* exp(-(2.0*j*PI*(double)(k))/(double)(steps));
        //a[kpos + steps/2] = t - a[kpos+steps/2]* exp(-(2.0*j*PI*(double)(k))/(double)(steps));
      }
    }
    ++count;
  }
  return 0;
}

complex<double> dctBuffLeft[MAX_BLOCK_SIZE*4];
complex<double> dctBuffRight[MAX_BLOCK_SIZE*4];
void fastDCT(uint32 *in, double *out, int N, int bits){
  double c0 = sqrt(1.0/N)/2, ck = sqrt(2.0/N)/2;
  unsigned long threadLeft, threadRight;
  // Left channel.
  rep(i, 0, 4*N) dctBuffLeft[i] = 0;

  rep(i, 0, N){
    //dctBuffLeft[reverseBits(i*2+1, bits+2)] = getLeftSample(in[i]);
    //dctBuffLeft[reverseBits(4*N-i*2-1, bits+2)] = getLeftSample(in[i]);
    dctBuffLeft[i*2+1] = getLeftSample(in[i]);
    dctBuffLeft[4*N-i*2-1] = getLeftSample(in[i]);
  }

  fft_params p1(dctBuffLeft, N*4, bits+2, false);
  //fft((void*)&p1);
  pthread_create(&threadLeft, NULL, fft, (void*)&p1);

  //Right channel.
  rep(i, 0, 4*N) dctBuffRight[i] = 0;
  rep(i, 0, N){
    //dctBuffRight[reverseBits(i*2+1, bits+2)] = getRightSample(in[i]);
    //dctBuffRight[reverseBits(4*N-i*2-1, bits+2)] = getRightSample(in[i]);
    dctBuffRight[i*2+1] = getRightSample(in[i]);
    dctBuffRight[4*N-i*2-1] = getRightSample(in[i]);
  }
  fft_params p2(dctBuffRight, N*4, bits+2, false);
  //fft((void*)&p2);
  pthread_create(&threadRight, NULL, fft, (void*)&p2);

  // Wait for both threads to finish.
  void *dummy;
  pthread_join(threadLeft, &dummy);
  pthread_join(threadRight, &dummy);
  out[0] = dctBuffLeft[0].real() * c0;
  rep(i, 1, N) out[i] = dctBuffLeft[i].real() * ck;
  out[N] = dctBuffRight[0].real() * c0;
  rep(i, 1, N) out[i+N] = dctBuffRight[i].real() * ck;
}
      

void slowDCT(uint32* block, complex<double> *out, int N){
  double c0 = sqrt(1.0/N), ck = sqrt(2.0/N);
  rep(k, 0, N){
    out[k] = 0;
    rep(l, 0, N){
      //out[k] += (double)getLeftSample(block[l])*(complex<double>(cos((PI*k*l)/N), sin((PI*k*l)/(N)))) * complex<double>(cos((PI*k)/(2.0*N)), sin((PI*k)/(2.0*N)));
      out[k] += (double)getRightSample(block[l]) * exp(-(j*(PI*k*l))/(double)N) * exp(-j*(PI*k)/(2.0*N));
    }
    if(k == 0)
      out[k] *= c0;
    else 
      out[k] *= ck;
    //cout << out[k].real() << " " ;
  }
  //cout << endl;
}

void DCTTransformBlock(uint32* block, double *leftOut, double *rightOut, int N){
  double c0 = sqrt(1.0/N), ck = sqrt(2.0/N);
  //double dct;
  rep(k, 0, N){
    leftOut[k] = 0;
    rightOut[k] = 0;
    rep(l, 0, N){
      double left = getLeftSample(block[l]);
      double right = getRightSample(block[l]);
      leftOut[k] += left*cos(PI*l*k/N + PI*k/(2*N));
      rightOut[k] += right*cos(PI*l*k/N + PI*k/(2*N));
    }
    if(k == 0){
      leftOut[k] *= c0;
      rightOut[k] *= c0;
    }
    else{
      leftOut[k] *= ck;
      rightOut[k] *= ck;
    }
  }
}
