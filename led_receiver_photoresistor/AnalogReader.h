#ifndef ANALOGREADER_H
#define ANALOGREADER_H

#include "BitPattern.h"

enum ParseType {
  ASCII,
  INTEGER
};

struct Parser {
  ParseType type;
  int dataLength; // length in bytes
};

struct AnalogReader {
  int high;
  int low;

  unsigned int* preambleChkArr; // doubled pattern for ease of checking
  BitPattern* startFrame;

  AnalogReader();
  AnalogReader(int high, int low);
  void calibrate(int sampleDelay, int threshold);

  BitPattern* getNextByte(int sampleDelay);
  // measures twice with one delay and returns value
  int measureNext();

  Parser parseHeader(BitPattern* header);

  // 1 is 1, 0 is 0, -1 if ambiguous <- caused by error with setup
  static int analogToDigital(int high, int low, int readval);

private:
  bool readPreamble(int preambleLen, int sampleDelay) {
    int calibratearr[preambleLen*2];
    BitPattern calb(preambleLen*2);
    calb.setBit(0);
    calibratearr[0] = 1;
    for(int i = 1; i < calb.size; i++) {
      int read = AnalogReader::analogToDigital(this->high, this->low, analogRead(A0));
      calibratearr[i] = read;
      if(read == 1) calb.setBit(i);
      else if(read == 0) calb.clearBit(i);
      delay(sampleDelay);
    }

    // check if equal to expected preamble
    for(int i = 0; i < ((preambleLen*2)/16); i++) {
      if(preambleChkArr[i] != calb.bitArray[i]) return false;
    }
    return true;
  }
};



#endif

