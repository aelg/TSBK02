#ifndef WAVE_H
#define WAVE_H

#include <inttypes.h>
#include <fstream>

using namespace std;

typedef uint32_t uint32;
typedef int16_t sample;

struct Format{
  int numChannels, sampleRate, byteRate, blockAlign, bitsPerSample, totBytes, totBlocks;
};


int read_header(fstream &f, Format *fmt);
uint32* read_to_memory(fstream &f, Format *fmt);
sample getRightSample(uint32 block);
sample getLeftSample(uint32 block);
void printRightChannel(uint32 *data);
void printLeftChannel(uint32 *data);
#endif
