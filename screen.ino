/**
 * Lcd Screen High-Level Routines Driver For Arduino Boards
 */

/* Custom char design for progressbar display */
static byte r0[8] = {
  B00000, 
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
}; // 0 / 5
static byte r1[8] = {
  B10000, 
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
}; // 1 / 5
static byte r2[8] = {
  B11000, 
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000
}; // 2 / 5 
static byte r3[8] = {
  B11100, 
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
}; // 3 / 5
static byte r4[8] = {
  B11110, 
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
}; // 4 / 5
static byte r5[8] = {
  B11111, 
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
}; // 5 / 5

/**
 * Screen initialization routine
 */
void beginScreen() {

  /* Register custom char into the lcd */
  lcd.createChar(0, r0);
  lcd.createChar(1, r1);
  lcd.createChar(2, r2);
  lcd.createChar(3, r3);
  lcd.createChar(4, r4);
  lcd.createChar(5, r5);

  /* Initialize the screen */
  lcd.begin(16, 2); 
  lcd.clear();
}

/**
 * Display bitmap file informations
 */
void displayFileInfo(const char* filename, int width, int height) {

  /* Clear the screen */
  lcd.setCursor(0, 0);
  lcd.clear();

  /* Display the infos */
  lcd.print(filename);
  lcd.setCursor(0, 1);
  lcd.print(width);
  lcd.print('x');
  lcd.print(height);
  lcd.print(F("px"));

  /* Display the "CUT" warning if image size don't fit */
  if(width > LED_STRIP_SIZE)
    lcd.print(F(" CUT"));
}

/**
 * Display regular file informations
 */
void displayInvalidFileInfo(const char* filename) {

  /* Clear the screen */
  lcd.setCursor(0, 0);
  lcd.clear();

  /* Display the infos */
  lcd.print(filename);
  lcd.setCursor(0, 1);
  lcd.print(F("NOT A VALID BMP"));
}

/**
 * Display regular file informations
 */
void displayUnsuportedFileInfo(const char* filename) {

  /* Clear the screen */
  lcd.setCursor(0, 0);
  lcd.clear();

  /* Display the infos */
  lcd.print(filename);
  lcd.setCursor(0, 1);
  lcd.print(F("UNSUPPORTED BMP"));
}

/**
 * Display the progress bar for the current image
 */
static void _displayProgress(byte percent) {
  byte i, cp, cl;

  /* Got to line 1 */
  lcd.setCursor(0, 1);

  /* Map percent from (0 ~ 100) to (0 ~ 80) (1 char = 5 columns, 16 * 5 = 80) */
  percent = map(percent, 0, 100, 0, 80);

  /* Compute number of full cell and the number of columns of the last cell */
  cp = percent / 5;
  cl = percent % 5;

  /* Draw the full cells */
  for(i = 0; i < cp; ++i)
    lcd.write(5);

  /* Draw the last cell */
  lcd.write(cl);

  /* Draw empty cells if any */
  for(i = 0; i < 16 - (cp + (cl ? 1 : 0)); ++i)
    lcd.write((uint8_t) 0);
}

/**
 * Display the progression informations of the current image
 */
void displayProgressInfos(const char* filename, byte percent) {

  /* Clear the screen */
  lcd.setCursor(0, 0);
  lcd.clear();

  /* Display the infos */
  lcd.print(filename);
  lcd.print(' ');
  lcd.print(percent);
  lcd.print('%');

  /* Draw the progress bar */
  _displayProgress(percent);
}

/**
 * Display the current delay choice value
 */
void displayDelayChoiceValue(byte fps) {

  /* Clear the screen */
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(F("Delay:"));

  /* Display the delay value */
  lcd.setCursor(0, 1);
  lcd.print(fps + 1);
  lcd.print(F(" FPS"));
}

/**
 * Display debug log
 */
void displayMessage(const char* msg) {

  /* Clear the screen */
  lcd.setCursor(0, 0);
  lcd.clear();

  /* Display the message */
  lcd.print(msg);
}

