/**
 * LPD8806 RGB Led Strip Driver for Arduino Board
 * (based on Adafruit source code)
 */

/** Latch count macro */
#define LATCH_COUNT(n) (((n + 63) / 64) * 3)

/** Display buffer */
static byte _displayBuffer[LED_STRIP_SIZE * 3];

/**
 * Utilities - Send N zeros over the SPI bus
 */
static inline void _writeZero(uint8_t n) {
  while(n--)
    lpd8806Write(0);
}

/**
 * Show pixels currently in the display buffer
 */
void displayPixels(void) {
  for (byte i = 0; i < LED_STRIP_SIZE * 3; ++i) 
    lpd8806Write(_displayBuffer[i]);
  _writeZero(LATCH_COUNT(LED_STRIP_SIZE));
  delay(10);
}

/**
 * Reset pixels values to "off" state
 */
void clearPixels() {
  for (byte i = 0; i < LED_STRIP_SIZE * 3; ++i) 
    _displayBuffer[i] = 0x80;
}

/**
 * Initialize the LPD8806 led strip
 */
void beginPixels(void) {
  pinMode(LED_STRIP_CHIP_DATA_PIN, OUTPUT);
  pinMode(LED_STRIP_CHIP_CLOCK_PIN, OUTPUT);
  digitalWrite(LED_STRIP_CHIP_CLOCK_PIN, HIGH);
  _writeZero(LATCH_COUNT(LED_STRIP_SIZE));
  clearPixels();
  displayPixels();
}

// Gamma correction compensates for our eyes' nonlinear perception of
// intensity.  It's the LAST step before a pixel value is stored, and
// allows intermediate rendering/processing to occur in linear space.
// The table contains 256 elements (8 bit input), though the outputs are
// only 7 bits (0 to 127).  This is normal and intentional by design: it
// allows all the rendering code to operate in the more familiar unsigned
// 8-bit colorspace (used in a lot of existing graphics code), and better
// preserves accuracy where repeated color blending operations occur.
// Only the final end product is converted to 7 bits, the native format
// for the LPD8806 LED driver.  Gamma correction and 7-bit decimation
// thus occur in a single operation.
PROGMEM prog_uchar gammaTable[]  = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
    7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
   11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
   16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
   23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
   30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
   40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
   50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
   62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
   76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
   92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};

// This function (which actually gets 'inlined' anywhere it's called)
// exists so that gammaTable can reside out of the way down here in the
// utility code...didn't want that huge table distracting or intimidating
// folks before even getting into the real substance of the program, and
// the compiler permits forward references to functions but not data.
inline byte gamma(byte x) {
  return pgm_read_byte(&gammaTable[x]);
}

/**
 * Change the color of pixel at index N
 */
void setPixelAt(byte index, byte r, byte g, byte b) {
  index *= 3;
#ifdef USE_GAMMA_CORRECTION
  _displayBuffer[index] = gamma(g) | 0x80; // LPD8806 color order is GRB
  _displayBuffer[index + 1] = gamma(r) | 0x80;
  _displayBuffer[index + 2] = gamma(b) | 0x80; 
#else
  _displayBuffer[index] = (g >> 1) | 0x80; // LPD8806 color order is GRB
  _displayBuffer[index + 1] = (r >> 1) | 0x80;
  _displayBuffer[index + 2] = (b >> 1) | 0x80; 
#endif
}

/**
 * Shift out data to the led strip using software SPI
 */
void lpd8806Write(uint8_t val) {
  // Optimized shiftOut() function
  static uint8_t bitData = digitalPinToBitMask(LED_STRIP_CHIP_DATA_PIN);
  static uint8_t portData = digitalPinToPort(LED_STRIP_CHIP_DATA_PIN);
  static uint8_t bitClock = digitalPinToBitMask(LED_STRIP_CHIP_CLOCK_PIN);
  static uint8_t portClock = digitalPinToPort(LED_STRIP_CHIP_CLOCK_PIN);
  static volatile uint8_t *outData = portOutputRegister(portData);
  static volatile uint8_t *outClock = portOutputRegister(portClock);
  for (uint8_t i = 0; i < 8; i++)  {	
    if(val & (1 << (7 - i)))
      *outData |= bitData;
    else
      *outData &= ~bitData;
    *outClock |= bitClock;
    delayMicroseconds(50);
    *outClock &= ~bitClock;		
  }
}



