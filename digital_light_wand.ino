/* ---- Includes ---- */
#include <SPI.h>           // For SPI communications (sd card & led strip)
#include <SD.h>            // For SD card reader
#include <LiquidCrystal.h> // For LCD display

/* ---- Sketch global declarations ---- */
/** SD chip select pin */
const byte SDCARD_CHIP_SELECT_PIN = 2; // WARNING NOT STANDARD !

/** Led strip chip pins */
const byte LED_STRIP_CHIP_DATA_PIN = 3;
const byte LED_STRIP_CHIP_CLOCK_PIN = A1;

/** Enable or disable gamma correction */
#define USE_GAMMA_CORRECTION 1

/** Delay table */
const unsigned int delayTable[] = {
  1000, // 1 fps
  500, // 2 fps
  333, // 3 fps
  250, // 4 fps
  200, // 5 fps
  166, // 6 fps
  142, // 7 fps
  125, // 8 fps
  111, // 9 fps
  100, // 10 fps
};
const byte DELAY_TABLE_SIZE = 10;

/** LiquidCrystal setup for DFRobots lcd shield */
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

/** Enumeration of possible error codes */
enum {
  NO_ERROR, /*!< No error */
  FORMAT_ERROR, /*!< File format error */
  UNSUPPORTED_OPTION, /*!< Unsupported file option error */
  FILE_ERROR, /*!< Something goes wrong with the file body (maybe corrupted ?) */
};

/** Led strip size in pixels */
static const byte LED_STRIP_SIZE = 64;

/** Analog pin used for the keypad */
static const byte KEYPAD_OUT_PIN = A0;

/** Enumeration of buttons */
enum {
  BP_NONE, /*!< No button pressed */
  BP_SELECT, /*!< Button select pressed */
  BP_LEFT, /*!< Button left pressed */
  BP_UP, /*!< Button up pressed */
  BP_DOWN, /*!< Button down pressed */
  BP_RIGTH /*!< Button right pressed */
};

/* ---- Begin main program ---- */
/* SD card root directory */
File root;

void setup() {

  /* Initialize hardware */
  beginScreen();
  beginPixels();
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  if (!SD.begin(SDCARD_CHIP_SELECT_PIN)) {
    displayMessage("SD init failed");
    for(;;);
  }
  
  /* Open root directory */
  root = SD.open("/");
  if (!root) {
    displayMessage("/ open failed");
    for(;;);
  }
}

void loop() {

  /* Get user file choice */
  int width, height;
  byte padding;
  File fi = chooseFile(&width, &height, &padding);

  /* Get user delay choice */
  byte delayMode = chooseDelay();

  /* Start drawing ! */
  if(startPainting(fi, width, height, padding, delayMode)) {
    displayMessage("FATAL ERROR :(");
    delay(5000);
  }

  /* Free ressource */
  fi.close();
}

File chooseFile(int *width, int *height, byte *padding) {

  /* Misc. variables */
  File f;
  byte c, bitmapOk = false, selectOk = false;

  /* Display prompt */
  displayMessage("File choice -> O");

  /* Wait for key to be pressed */
  while(getKey() == BP_NONE);

  /* Get user input */
  do {

    /* Get the current pressed key */
    c = getKey();

    /* Check for input */
    if(c == BP_NONE)
      continue;

    /* Process input */
    if(c == BP_RIGTH) {

      /* Unselect */
      selectOk = false;

      /* Open next entry */
      if(f) f.close(); /* Avoid memory leak */
      f = root.openNextFile();
      if(!f) { /* No more files */
        displayMessage("NO MORE FILES");
        root.rewindDirectory();
        goto end_do;
      }

      /* Check for directory */
      if(f.isDirectory()) {
        displayInvalidFileInfo(f.name());
        goto end_do;
      }

      /* Check for bitmap */
      byte err = readBitmapHeader(f, width, height, padding);
      switch(err) {
      case NO_ERROR:
        displayFileInfo(f.name(), *width, *height);
        bitmapOk = true;
        break;

      case FORMAT_ERROR:
        displayInvalidFileInfo(f.name());
        bitmapOk = false;
        break;

      case UNSUPPORTED_OPTION:
        displayUnsuportedFileInfo(f.name());
        bitmapOk = false;
        break;
      }

    } 
    else if (c == BP_SELECT) { /* Time to go ! */
      selectOk = true;
    }

    /* Wait for key to be released */
end_do:
    while(getKey() != BP_NONE);

  } 
  while(!selectOk || !bitmapOk); /* Wait for user to press select */

  /* Return the user choice */
  return f;
}

/**
 * Prompt user to choose the delay between frames
 */
byte chooseDelay() {

  /* Misc. variables */
  byte c, index = 0, selectOk = false;

  /* Display prompt */
  displayDelayChoiceValue(index);

  /* Wait for key to be pressed */
  while(getKey() == BP_NONE);

  /* Get user input */
  do {

    /* Get the current pressed key */
    c = getKey();

    /* Check for input */
    if(c == BP_NONE)
      continue;

    /* Process input */
    switch(c) {
    case BP_UP: /* Increment index with anti-overflow */
      selectOk = false;
      if(++index == DELAY_TABLE_SIZE)
        --index;
      break;

    case BP_DOWN: /* Decrement index with anti-underflow */
      selectOk = false;
      if(index > 0)
        --index;
      break;

    case BP_SELECT: /* Time to go ! */
      selectOk = true;
      break;
    }

    /* Display current delay value */
    displayDelayChoiceValue(index);

    /* Wait for key to be released */
    while(getKey() != BP_NONE);

  } 
  while(!selectOk); /* Wait for user to press select */

  /* Return the user choice */
  return index;
}

/**
 * Display one line at the time
 */
byte _displayOneLine(File &fi, int width, byte padding) {

  /* Number of visible pixel */
  byte realWidth = (width > LED_STRIP_SIZE) ? LED_STRIP_SIZE : width;
  byte r, g, b, padBuffer[4];

  /* For each visible pixels of the line */
  for(byte i = 0; i < realWidth; ++i) {

    /* Get the pixel data */
    if(readNextPixel(fi, &r, &g, &b))
      return FILE_ERROR;

    /* Store them in display buffer */
    setPixelAt(i, r, g, b);
  }

  /* Turn off unused pixels */
  for(byte i = realWidth; i < LED_STRIP_SIZE; ++i) {
    setPixelAt(i, 0, 0, 0);
  }

  /* For each hidden pixels of the line */
  for(int i = realWidth; i < width; ++i) {

    /* Get the pixel without displaying it */
    if(readNextPixel(fi, &r, &g, &b))
      return FILE_ERROR;
  }

  /* Handle padding byte */
  if(padding)
    fi.read(padBuffer, padding);
    
  /* Show line pixels */
  displayPixels();

  /* No error */
  return NO_ERROR;
}

/**
 * Start displaying of a whole image
 */
byte startPainting(File &fi, int width, int height, byte padding, byte delayMode) {

  /* For each lines of the image */
  for(int i = 0; i < height; ++i) {

    /* Display the progression */
    displayProgressInfos(fi.name(), (i * 100) / height);

    /* Display the line */
    if(_displayOneLine(fi, width, padding))
      return FILE_ERROR;

    /* Wait for the next frame */
    delay(delayTable[delayMode]);
  }
  
  /* Clear pixels */
  clearPixels();
  displayPixels();

  /* No error */
  return NO_ERROR;
}

