#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "wave.h"
#include "transform.h"

using namespace std;

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)
#define all(x) (x).begin(),(x).end()
#define PB(x) push_back((x))
#define print(x) printf("%d\n", x)

int read_header(fstream &f, Format *fmt, bool noprint){
  char buff[100];
  uint32 a;

  /* Read RIFF header */
  f.read(buff, 4);
  buff[4] = 0;
  if (strcmp(buff, "RIFF")){
    if(!noprint)
      cerr << "Not a wave file, exiting.\n";
    return 0;
  }
  f.read((char*)&a, 4);
  if(!noprint)
    cerr << "RIFF size: " << a << '\n';

  f.read(buff, 4);
  if (strcmp(buff, "WAVE")){
    if(!noprint)
      cerr << "Not a wave file, exiting.\n";
    return 0;
  }

  /* Read fmt subchunk */
  f.read(buff, 4);
  if (strcmp(buff, "fmt ")){
    if(!noprint)
      cerr << "Unrecognized header, exiting.\n";
    return 0;
  }
  f.read((char*)&a, 4);
  if(!noprint)
    cerr << "Subchunk1 size: " << a << '\n';
  if (a != 16){
    cerr << "Unrecognized header, exiting.\n";
    return 0;
  }
  a = 0;
  f.read((char*)&a, 2);
  if (a != 1){
    if(!noprint)
      cerr << "Not a PCM-file, exiting.\n";
    return 0;
  }
  a = 0;
  f.read((char*)&a, 2);
  if(!noprint)
    cerr << "Channels: " << a << '\n';
  fmt->numChannels = a;
  f.read((char*)&a, 4);
  if(!noprint)
    cerr << "Sample rate: " << a << '\n';
  fmt->sampleRate = a;
  f.read((char*)&a, 4);
  if(!noprint)
    cerr << "Byte rate: " << a << '\n';
  fmt->byteRate = a;
  a = 0;
  f.read((char*)&a, 2);
  if(!noprint)
    cerr << "Block align: " << a << '\n';
  fmt->blockAlign = a;
  a = 0;
  f.read((char*)&a, 2);
  if(!noprint)
    cerr << "Bits per sample: " << a << '\n';
  fmt->bitsPerSample = a;

  /* Read start of data subchunk */
  f.read(buff, 4);
  if (strcmp(buff, "data")){
    if(!noprint)
      cerr << "Unrecognized header, exiting.\n";
    return 0;
  }
  f.read((char*)&a, 4);
  if(!noprint)
    cerr << "Data size: " << a << '\n';
  fmt->totBytes = a;

  return 1;
}

uint32* read_to_memory(fstream &f, Format *fmt){
  uint32 *data;
  if(fmt->blockAlign != sizeof(uint32) || fmt->bitsPerSample != 16){
    cerr << "Unsupported file-format, needs 16 bits per sample, stereo.\n";
    return 0;
  }
  if(fmt->totBytes % (2*fmt->bitsPerSample/8)){
    cerr << "Bad file.\n";
    return 0;
  }
  fmt->totBlocks = fmt->totBytes/4;
  if(fmt->totBytes % MAX_BLOCK_SIZE == 0) data = (uint32*) malloc(fmt->totBytes);
  else data = (uint32*) calloc((fmt->totBytes/MAX_BLOCK_SIZE + 1) * MAX_BLOCK_SIZE, 1);
  f.read((char*)data, fmt->totBytes);
  return data;
}

void write_to_file(fstream &f, uint32 *data, Format *fmt){
  uint32 a;

  /* Read RIFF header */
  f.write("RIFF", 4);
  a = 36+fmt->totBytes;
  f.write((char*)&a, 4);

  f.write("WAVE", 4);

  /* Write fmt subchunk */
  f.write("fmt ", 4);

  a = 16;
  f.write((char*)&a, 4);
  a = 1;
  f.write((char*)&a, 2);
  a = fmt->numChannels;
  f.write((char*)&a, 2);
  a = fmt->sampleRate;
  f.write((char*)&a, 4);
  a = fmt->byteRate;
  f.write((char*)&a, 4);
  a = fmt->blockAlign;
  f.write((char*)&a, 2);
  a = fmt->bitsPerSample;
  f.write((char*)&a, 2);

  /* Write start of data subchunk */
  f.write("data", 4);
  a = fmt->totBytes;
  f.write((char*)&a, 4);

  f.write((char*)data, fmt->totBytes);
  return;
}

sample getRightSample(uint32 block){
  return (sample) block & 0xffff;
}

sample getLeftSample(uint32 block){
  return (sample) (block >> 16);
}

void printRightChannel(uint32 *data){
  cerr << "Right channel raw data: \n";
  rep(i, 0, 45){
    cerr << setw(6) << getRightSample(data[i]) << " ";
  }
  cerr << '\n';
}
void printLeftChannel(uint32 *data){
  cerr << "Left channel raw data: \n";
  rep(i, 0, 45){
    cerr << setw(6) << getLeftSample(data[i]) << " ";
  }
  cerr << '\n';
}
