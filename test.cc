#include <iostream>
#include <fstream>
#include <cstring>
#include <inttypes.h>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <complex>

#include "wave.h"
#include "transform.h"

using namespace std;

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)
#define all(x) (x).begin(),(x).end()
#define PB(x) push_back((x))
#define print(x) printf("%d\n", x)

/* Some error-checking functions */
void printTransform(double *dct){
  rep(k, 0, block_size){
    rep(l, 0, block_size){
      cout << dct[k*block_size + l] << " ";
    }
    cout << '\n';
  }
}

void printBlock(double* block){
  rep(l, 0, block_size){
    cout << round(block[l]) << " ";
  }
  cout << '\n';
}

complex<double> const j(0,1);
double const eps = 1e-5;
int const test_size = 1024;
int const test_bits = 10;

int main(){
  double fftIn[4] = {1, 7, 20, 45};
  complex<double> fftOut[4];
  fftOut[0] = 73.0;
  fftOut[1] = -19.0 + 38.0*j;
  fftOut[2] = -31.0;
  fftOut[3] = -19.0 - 38.0*j;
  complex<double> fft_test[4];
  sample left[10] = {1, 7, 20, 45, 86, 149, 238, 357, 509, 700};
  sample right[10] = {0, 4, 12, 29, 57, 99, 158, 237, 340, 467};
  bool fail = false;
  Format fmt;
  uint32 *data;
  complex<double> outSlowDCT[4096*2];
  double outFastDCT[4096*2];
  uint32 outIDCT[4096];

  fstream f("heyhey.wav", fstream::binary | fstream::in);

  if(!read_header(f, &fmt)) return 1;
  if(!(data = read_to_memory(f, &fmt))) return 1;

  cout << "\n\nTesting wave read" << endl;
  cout << "Left channel" << endl;
  rep(i, 0, 10){
    if(getLeftSample(data[i]) != left[i]) fail = true;
  }
  if(fail){
    cout << "FAIL!!! " << endl;
    fail = false;
  }
  else cout << "OK" << endl;
  cout << "Right channel" << endl;
  rep(i, 0, 10){
    if(getRightSample(data[i]) != right[i]) fail = true;
  }
  if(fail){
    cout << "FAIL!!! " << endl;
    fail = false;
  }
  else cout << "OK" << endl << endl;

  initFFT();
  cout << "\nTesting fft" << endl;
  cout << "FFT in:  ";
  rep(i, 0, 4){
    fft_test[i] = fftIn[i];
    cout << fftIn[i] << " ";
  }
  cout << endl;
  fft_params p(fft_test, 4, 2, false);
  fft((void*)&p);
  cout << "FFT out: ";
  rep(i, 0, 4){
    cout << fft_test[i] << " ";
    if(abs(fft_test[i] - fftOut[i]) > eps){
      fail = true;
      cout << fft_test[i] - fftOut[i] << " ";
    }
  }
  cout << endl;
  if(fail){
    cout << "FAIL!!!" << endl;
    fail = false;
  }
  else cout << "OK" << endl;

  cout << "\nTesting ifft" << endl;
  p.inverse = true;
  fft((void*)&p);
  cout << "iFFT out: ";
  rep(i, 0, 4){
    cout << fft_test[i] << " ";
    if(fft_test[i].real() != fftIn[i]) fail = true;
  }
  cout << endl;
  if(fail){
    cout << "FAIL!!!" << endl;
    fail = false;
  }
  else cout << "OK" << endl;

  cout << "\nTesting if fastDCT produces the same (nearly) output as slowDCT" << endl;
  slowDCT(data, outSlowDCT, 4096);
  fastDCT(data, outFastDCT, test_size, test_bits);
  rep(i, 0, 2*test_size){
    if(abs(outSlowDCT[i].real() - outFastDCT[i]) > eps){
      fail = true;
      cout << "Pos: " << i << " Diff: " << outSlowDCT[i].real() - outFastDCT[i] << endl;
    }
  }
  if(fail){
    cout << "FAIL!!!" << endl;
    fail = false;
  }
  else cout << "OK" << endl;

  cout << "\nTesting fastIDCT" << endl;
  fastIDCT(outFastDCT, outIDCT, test_size, test_bits);
  rep(i, 0, test_size){
    if(getLeftSample(outIDCT[i]) != getLeftSample(data[i])){
      fail = true;
      cout << "Left Pos: " << i << " data: " << getLeftSample(data[i]) << " IDCT: " << getLeftSample(outIDCT[i]) << endl;
    }
    if(getRightSample(outIDCT[i]) != getRightSample(data[i])){
      fail = true;
      cout << "Right Pos: " << i << " data: " << getRightSample(data[i]) << " IDCT: " << getRightSample(outIDCT[i]) << endl;
    }
  }
  if(fail){
    cout << "FAIL!!!" << endl;
    fail = false;
  }
  else cout << "OK" << endl;

  cout << endl;
  return 0;
}
    
