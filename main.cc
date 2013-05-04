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
  //printLeftChannel(data);

  double outFast[block_size*2];
  initFFT();
  cout << "fastDCT" << endl;
  rep(i, 0, 2000){
    if(i%100 == 0) cout << i << endl;
    fastDCT(data + block_size*i, outFast, block_size, 12);
  }
  fastDCT(data, outFast, block_size, 12);
  cout << "fastDCT done" << endl;
  /*cout << "slowDCT" << endl;
  complex<double> complexOutSlow[block_size];
  slowDCT(data, complexOutSlow, block_size);
  cout << "slowDCT done" << endl;
  rep(i, 0, block_size) if (abs(outFast[i+block_size] - complexOutSlow[i].real()) > 0.0001) cout << outFast[i+block_size] - complexOutSlow[i].real() << endl;*/

  return 0;
}
    
