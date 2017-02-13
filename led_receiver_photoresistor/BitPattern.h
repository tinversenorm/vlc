#ifndef BITPATTERN_H
#define BITPATTERN_H

struct BitPattern {
  int size;
  int arrSize;
  int* bitArray;

  BitPattern() : size(0) {
  }

  BitPattern(int size) {
    this->size = size;
    arrSize = size/16;
    arrSize += size % 16 == 0 ? 0 : 1;
    this->bitArray = new int[arrSize];
    for(int i = 0; i < arrSize; i++) this->bitArray[i] = 0;
  }

  void setHalfWord(int halfword, int index) {
    if(index < arrSize) {
      bitArray[index] = halfword;
    }
  }

  void setWithArray(const unsigned int* data, int datalen) {
    int len = datalen < arrSize ? datalen : arrSize;
    for(int i = 0; i < len; i++) {
      bitArray[i] = data[i]; 
    }
  }

  // indexed left to right
  int getBit(int index) const {
    if(index < size) {
      int arrIndex = index / 16;
      return (bitArray[arrIndex] & (1 << (15 - (index % 16)))) != 0;
    } else {
      return -1;
    }
  }

  void setBit(int index) {
    if(index < size) {
      int arrIndex = index / 16;
      bitArray[arrIndex] = bitArray[arrIndex] | (1 << (15 - (index % 16)));
    }
  }

  void clearBit(int index) {
    if(index < size) {
      int arrIndex = index / 16;
      bitArray[arrIndex] = bitArray[arrIndex] & ~(1 << (15 - (index % 16)));
    }
  }

  bool equals(const BitPattern& other) {
    if(other.size != size) return false;
    for(int i = 0; i < size; i++) {
      if(getBit(i) != other.getBit(i)) return false;
    }
    return true;
  }
};

#endif
