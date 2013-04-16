/**
 * LCD Analog Keypad Driver for Arduino Board
 * (based on DFrobots source code)
 */

/**
 * Get the current pressed key (if any)
 */
byte getKey(void) {

  /* Read the analog pin of keypad */
  unsigned int val = analogRead(KEYPAD_OUT_PIN);

  /* Test the value against pre-defined buttons values */
  if (val > 1000) return BP_NONE;
  if (val < 50) return BP_RIGTH;  
  if (val < 195) return BP_UP; 
  if (val < 380) return BP_DOWN; 
  if (val < 555) return BP_LEFT; 
  if (val < 790) return BP_SELECT;

  /* By default no button are pressed */
  return BP_NONE;
}

