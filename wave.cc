#include <iostream>
#include <cstdlib>
#include <cstring>
#include "wave.h"

using namespace std;

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)
#define all(x) (x).begin(),(x).end()
#define PB(x) push_back((x))
#define print(x) printf("%d\n", x)

int read_header(fstream &f, Format *fmt){
  char buff[100];
  uint32 a;

  /* Read RIFF header */
  f.read(buff, 4);
  buff[4] = 0;
  if (strcmp(buff, "RIFF")){
    cout << "Not a wave file, exiting.\n";
    return 0;
  }
  f.read((char*)&a, 4);
  cout << "RIFF size: " << a << '\n';

  f.read(buff, 4);
  if (strcmp(buff, "WAVE")){
    cout << "Not a wave file, exiting.\n";
    return 0;
  }

  /* Read fmt subchunk */
  f.read(buff, 4);
  if (strcmp(buff, "fmt ")){
    cout << "Unrecognized header, exiting.\n";
    return 0;
  }
  f.read((char*)&a, 4);
  cout << "Subchunk1 size: " << a << '\n';
  if (a != 16){
    cout << "Unrecognized header, exiting.\n";
    return 0;
  }
  a = 0;
  f.read((char*)&a, 2);
  if (a != 1){
    cout << "Not a PCM-file, exiting.\n";
    return 0;
  }
  a = 0;
  f.read((char*)&a, 2);
  cout << "Channels: " << a << '\n';
  fmt->numChannels = a;
  f.read((char*)&a, 4);
  cout << "Sample rate: " << a << '\n';
  fmt->sampleRate = a;
  f.read((char*)&a, 4);
  cout << "Byte rate: " << a << '\n';
  fmt->byteRate = a;
  a = 0;
  f.read((char*)&a, 2);
  cout << "Block align: " << a << '\n';
  fmt->blockAlign = a;
  a = 0;
  f.read((char*)&a, 2);
  cout << "Bits per sample: " << a << '\n';
  fmt->bitsPerSample = a;

  /* Read start of data subchunk */
  f.read(buff, 4);
  if (strcmp(buff, "data")){
    cout << "Unrecognized header, exiting.\n";
    return 0;
  }
  f.read((char*)&a, 4);
  cout << "Data size: " << a << '\n';
  fmt->totBytes = a;
  
  return 1;
}

uint32* read_to_memory(fstream &f, Format *fmt){
  uint32 *data;
  if(fmt->blockAlign != sizeof(uint32) || fmt->bitsPerSample != 16){
    cout << "Unsupported file-format, needs 16 bits per sample, stereo.\n";
    return 0;
  }
  if(fmt->totBytes % 4){
    cout << "Bad file.\n";
    return 0;
  }
  fmt->totBlocks = fmt->totBytes/4;
  data = (uint32*) malloc(fmt->totBytes);
  f.read((char*)data, fmt->totBytes);
  return data;
}

sample getRightSample(uint32 block){
  return (sample) block & 0xffff;
}

sample getLeftSample(uint32 block){
  return (sample) (block >> 16);
}

void printRightChannel(uint32 *data){
  cout << "Right channel raw data: \n";
  rep(i, 0, 150){
    cout << getRightSample(data[i]) << " ";
  }
  cout << '\n';
}
void printLeftChannel(uint32 *data){
  cout << "Left channel raw data: \n";
  rep(i, 0, 150){
    cout << getLeftSample(data[i]) << " ";
  }
  cout << '\n';
}
