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

int const mdct_len = 16;

int main(int argc, char *argv[]){
  Format fmt;
  uint32 *data;

  if(argc != 2){
    cout << "Usage " << argv[0] << " filename.";
    return 0;
  }
  fstream f(argv[1], fstream::binary | fstream::in);

  if(!read_header(f, &fmt)) return 1;
  if(!(data = read_to_memory(f, &fmt))) return 1;

  //printRightChannel(data);
  printLeftChannel(data);

  double outFast[block_size*2];
  double outmdctSlow[mdct_len*2];
  double outmdctFast[mdct_len*2];
  initFFT();
  cout << "slowMDCT" << endl;
  slowMDCT(data, outmdctSlow, mdct_len);
  rep(i, 0, mdct_len/2)
    cout << outmdctSlow[i] << " ";
  cout << endl;
  rep(i, mdct_len, mdct_len*3/4)
    cout << outmdctSlow[i] << " ";
  cout << endl;
  cout << "fastMDCT" << endl;
  fastMDCT(data, outmdctFast, mdct_len, 4);
  rep(i, 0, mdct_len/2)
    cout << outmdctFast[i] << " ";
  cout << endl;
  rep(i, mdct_len, mdct_len*3/4)
    cout << outmdctFast[i] << " ";
  cout << endl;
  /*rep(i, 0, 2000){
    if(i%100 == 0) cout << i << endl;
    fastDCT(data + block_size*i, outFast, block_size, 12);
    }*/
  /*fastDCT(data, outFast, block_size, nbrBits);
    rep(i, 0, block_size*2) cout << outFast[i] << " ";
    cout << "fastDCT done" << endl;
    cout << "fastIDCT" << endl;
    uint32 dataOut[block_size];
    fastIDCT(outFast, (uint32*)&dataOut, block_size, nbrBits);
    rep(i, 0, block_size) cout << getLeftSample(dataOut[i]) << " ";
    cout << endl;
    cout << "fastIDCT done" << endl;
    cout << "slowDCT" << endl;
    complex<double> complexOutSlow[block_size];
    slowDCT(data, complexOutSlow, block_size);
    cout << "slowDCT done" << endl;
    rep(i, 0, block_size) if (abs(outFast[i+block_size] - complexOutSlow[i].real()) > 0.0001) cout << outFast[i+block_size] - complexOutSlow[i].real() << endl;*/



  free(data);
  return 0;
}

