#include <stdlib.h>
#include "AnalogReader.h"
#define PREAMBLE_LEN 8
#define SAMPLE_DELAY 150
#define THRESHOLD 35
#define START_FRAME 0xAB00

AnalogReader::AnalogReader(int high, int low) {
  this->high = high;
  this->low = low;
  
  // set up checking the preamble
  preambleChkArr = new unsigned int[(PREAMBLE_LEN * 2)/16];
  for(int i = 0; i < ((PREAMBLE_LEN * 2)/16); i++) preambleChkArr[i] = 0xCCCC;
  for(int i = 0; i < ((PREAMBLE_LEN * 2)/16); i++) Serial.println(preambleChkArr[i]);

  // set up start frame check
  startFrame = new BitPattern(8);
  startFrame->setHalfWord(START_FRAME, 0); 
}

void AnalogReader::calibrate(int sampleDelay, int threshold) {
  bool done = false;
  int prev = analogRead(A0);
  while(!done) {
    // loop until flash detected
    int read = analogRead(A0);
    Serial.println(read);
    if(read > high) high = read;
    else if(read < low) low = read;
    if(((high - low > threshold) || (low - high > threshold)) && (read - prev > threshold || prev - read > threshold)) {
      done = true;
    }
    delay(sampleDelay);
    if(done) {
      done = readPreamble(PREAMBLE_LEN, sampleDelay);
      Serial.println(done);
    }
    prev = read;
  }
  return true;
}

int AnalogReader::analogToDigital(int high, int low, int readval) {
  if(readval <= low) return 0;
  else if(readval >= high) return 1;
  else {
    if((high - readval) <= ((high - low) * 0.4)) return 1;
    else if((readval - low) <= ((high - low) * 0.4)) return 0;
    else return -1;
  }
}

// used after calibration
int AnalogReader::measureNext() {
  int val = analogToDigital(high, low, analogRead(A0));
  delay(150);
  int chk = analogToDigital(high, low, analogRead(A0));
  if(val == chk) {
    return val;
  } else {
    return -1;
  }
}

BitPattern* AnalogReader::getNextByte(int sampleDelay) {
  BitPattern* byte = new BitPattern(8);
  for(int i = 0; i < 8; i++) {
    int bit = measureNext();
    if(bit) byte->setBit(i);
    else byte->clearBit(i);
    delay(sampleDelay);
  }
  return byte;
}

Parser AnalogReader::parseHeader(BitPattern* header) {
  int size = 0;
  for(int i = 4; i < 8; i++) {
    size += (header->getBit(i) << (i - 4));
  }
  int type = header->getBit(3); // 4th bit ascii or integer
  ParseType t = type ? INTEGER : ASCII;
  return Parser{t, size};
}

AnalogReader* current;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(10, INPUT);
  pinMode(A0, INPUT);
  int hl = analogRead(A0);
  current = new AnalogReader(hl, hl);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(analogRead(A0));
  /*
  current->calibrate(SAMPLE_DELAY, THRESHOLD);
  Serial.println("Calibrated.");
  BitPattern* sFrame = current->getNextByte(SAMPLE_DELAY);
  printBitPattern(sFrame);
  if(!sFrame->equals(*(current->startFrame))) {
    Serial.println("No start frame detected.");
  } else {
    Serial.println("Start frame detected.");
    BitPattern* header = current->getNextByte(SAMPLE_DELAY);
    printBitPattern(header);
    Parser p = current->parseHeader(header);
    printParser(p);
    BitPattern* next;
    for(int i = 0; i < p.dataLength; i++) {
      next = current->getNextByte(SAMPLE_DELAY);
      char c = 0;
      for(int i = 0; i < 8; i++) {
        c += (next->getBit(i) << (7 - i));
      }
      Serial.write(c);
    }
    Serial.println();
  }
  */
}

void printBitPattern(BitPattern* b) {
  Serial.print("[ ");
  for(int i = 0; i < b->size; i++) {
    Serial.print(b->getBit(i));
  }
  Serial.println("]");
}

void printParser(Parser p) {
  Serial.print("Type: ");
  if(p.type == INTEGER) Serial.print("Integer, ");
  else if(p.type == ASCII) Serial.print("Ascii, ");
  Serial.print("Size: ");
  Serial.println(p.dataLength);
}



