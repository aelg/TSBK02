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

int fitIntoBlocks(int n, int blocksize){
  if(n % MAX_BLOCK_SIZE) n = (n/blocksize+1)*blocksize;
  return n;
}

double sq(double x) {return x*x;}

int const blocksize = 1024;
int const bits = 10;

int main(int argc, char *argv[]){
  Format fmt1, fmt2;
  uint32 *data1, *data2;
  double sumNoise = 0, sumEnergy = 0, snr;

  if(argc != 3){
    cout << "Usage " << argv[0] << " original distorted.";
    return 0;
  }
  fstream f1(argv[1], fstream::binary | fstream::in);
  fstream f2(argv[2], fstream::binary | fstream::in);

  if(!read_header(f1, &fmt1, true)) return 1;
  if(!(data1 = read_to_memory(f1, &fmt1))) return 1;
  if(!read_header(f2, &fmt2, true)) return 1;
  if(!(data2 = read_to_memory(f2, &fmt2))) return 1;

  if(fmt1.totBlocks != fmt2.totBlocks){
    cout << "Files not equally long.\n" << endl;
    return 1;
  }
  
  rep(i, 0, fmt1.totBlocks){
    sumNoise += sq((getLeftSample(data1[i]) - getLeftSample(data2[i])));
    sumEnergy += sq(getLeftSample(data1[i]));
    sumNoise += sq((getRightSample(data1[i]) - getRightSample(data2[i])));
    sumEnergy += sq(getRightSample(data1[i]));
  }

  snr = 10 * log10(sumEnergy/sumNoise);
  //cout << sumNoise << " " << sumEnergy << endl;
  cout << "SNR is " << snr << endl;

  free(data1);
  free(data2);
  return 0;
}

