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

#define HIST_SIZE 20000

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


void transformData(uint32 *data, double* out, int blocksize, int bits, int n){
  int i;

  n = fitIntoBlocks(n, blocksize);

  fprintf(stderr, "Starting transformation.\n");
  for(i = 0; i < n; i += blocksize){
    fastDCT(data + i, out + i*2, blocksize, bits);
  }
  fprintf(stderr, "Transformed %d blocks.\n", i/blocksize);
}

void inverseTransformData(uint32 *data, double* in, int blocksize, int bits, int n){
  int i;

  n = fitIntoBlocks(n, blocksize);

  fprintf(stderr, "Starting inverse transformation.\n");
  for(i = 0; i < n; i += blocksize){
    fastIDCT(in + i*2, data + i, blocksize, bits);
  }
  fprintf(stderr, "Inverse transformed %d blocks.\n", i/blocksize);
}

void quant(double *data, int factor, int n){
  fprintf(stderr, "Quantifying.\n");

  rep(i, 0, 2*n){
    data[i] = round(data[i]/factor);
  }
}
void dequant(double *data, int factor, int n){
  fprintf(stderr, "Dequantifying.\n");

  rep(i, 0, 2*n){
    data[i] = data[i]*factor;
  }
}

int hist[HIST_SIZE];
void fill_hist(double *data, int n){
  fprintf(stderr, "Making histogram.\n");
  rep(i, 0, HIST_SIZE) hist[i] = 0;
  rep(i, 0, 2*n){
    if((int) abs(data[i]) < HIST_SIZE/2)
      ++hist[(int)data[i]+HIST_SIZE/2];
    else cerr << "Hist out of bounds: " << (int) data[i] << endl;
  }
}

double calc_entropy(int n){
  double H = 0;
  fprintf(stderr, "Calculating entropy.\n");
  rep(i, 0, HIST_SIZE){
    double p = hist[i]/(2.0*n);
    if(p) H += p*log2(p);
  }
  return -H;
}

double sq(double x) {return x*x;}

double SNR(uint32 *signal, uint32 *noise, int n, double maxValue){
  double sumNoise = 0, sumEnergy = 0;
  rep(i, 0, n){
    //cout << getLeftSample(signal[i]) << " " << getLeftSample(noise[i]) << endl;
    sumNoise += sq((getLeftSample(signal[i]) - getLeftSample(noise[i]))/maxValue);
    sumEnergy += sq(getLeftSample(signal[i])/maxValue);
    sumNoise += sq((getRightSample(signal[i]) - getRightSample(noise[i]))/maxValue);
    sumEnergy += sq(getRightSample(signal[i])/maxValue);
  }
  return 10 * log10(sumEnergy/sumNoise);
}

int const blocksize = 1024;
int const bits = 10;
int const quantFactor = 200;

/* Faster if not the whole file is needed */
int shorten = 1;

int main(int argc, char *argv[]){
  Format fmt;
  uint32 *dataIn, *dataOut;
  double *transformedData;
  double snr;

  if(argc != 2){
    cerr << "Usage " << argv[0] << " filename.";
    return 0;
  }
  fstream f(argv[1], fstream::binary | fstream::in);

  fstream fOut("out.wav", fstream::binary | fstream::out);

  if(!read_header(f, &fmt)) return 1;
  if(!(dataIn = read_to_memory(f, &fmt))) return 1;
  
  initFFT();

  transformedData = (double*) malloc(fitIntoBlocks(2 * fmt.totBlocks, blocksize) * sizeof(double)/sizeof(char));

  dataOut = (uint32*) malloc(fitIntoBlocks(fmt.totBlocks, blocksize) * sizeof(uint32)/sizeof(char));

  transformData(dataIn, transformedData, blocksize, bits, fmt.totBlocks/shorten);

  quant(transformedData, quantFactor, fmt.totBlocks/shorten);

  rep(i, 0, fmt.totBlocks/shorten){
    cout << transformedData[i] << " ";
  }
  cout << endl;

  fill_hist(transformedData, fmt.totBlocks/shorten);
  cerr << "Entropy: " << calc_entropy(fmt.totBlocks/shorten) << endl;

  dequant(transformedData, quantFactor, fmt.totBlocks/shorten);

  inverseTransformData(dataOut, transformedData, blocksize, bits, fmt.totBlocks/shorten);
  
  write_to_file(fOut, dataOut, &fmt);
  
  //printRightChannel(dataIn);
  //printRightChannel(dataOut);

  snr = SNR(dataIn, dataOut, fmt.totBlocks/shorten,1);
  cerr << "SNR is " << snr << endl;

  free(dataIn);
  free(dataOut);
  return 0;
}

