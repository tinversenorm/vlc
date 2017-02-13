 #include "BitPattern.h"
#define ledPin 6
#define transmitPd 300 // 300 ms every bit
#define PREAMBLE_LEN 8

BitPattern preamble(PREAMBLE_LEN);
BitPattern startFrame(8);
BitPattern header(8);

enum ParseType {
  ASCII = 0,
  INTEGER = 1
}; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(2, OUTPUT);

  preamble.setHalfWord(0xAAAA, 0);
  preamble.setHalfWord(0xAAAA, 1);
  startFrame.setHalfWord(0xAB00, 0);

  header.setHalfWord(0x0100, 0); // test send letter o
  // "hello, world!" is 12 letters and ascii type

  BitPattern oMessage(8);
  oMessage.setHalfWord(0x6F00, 0);

  sendBitPattern(preamble, transmitPd);
  sendBitPattern(startFrame, transmitPd);
  sendBitPattern(header, transmitPd);
  sendBitPattern(oMessage, transmitPd);

  digitalWrite(ledPin, LOW);
}

void loop() {
  
}

void sendHeader(BitPattern& pattern, int length, ParseType type) {
  
  unsigned int ptype = type;
  
}

void sendBitPattern(BitPattern& pattern, int delayLen) {
  for(int i = 0; i < pattern.size; i++) {
    int bit = pattern.getBit(i);
    if(bit == 1) digitalWrite(ledPin, HIGH);
    else if(bit == 0) digitalWrite(ledPin, LOW);
    delay(delayLen);
  }
}

void sendString(char* message, int length) {
  
}

/*
 * Notes: 
 * Tried Receiving with different color plain leds and none recognize light from led in ambient
 * light and no ambient light even when constantly lit led was placed on the same breadboard
 * Resistance: 800 MOhms
 */

