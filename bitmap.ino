/**
 * Bitmap File Format Driver For Arduino Boards
 * (based on Watterott mSD-shield "BMPdemo" source code)
 */

/** Bitmap header structure */
typedef struct {
  uint8_t  magic[2]; /*!< Signature */
  uint32_t size; /*!< File size */
  uint16_t rsrvd1; /*!< Reserved */
  uint16_t rsrvd2; /*!< Reserved */
  uint32_t offset; /*!< File offset to pixels array */
} 
BMP_Header_t;

/** Bitmap DIP header structure */
typedef struct {
  uint32_t size; /*!< DIP header size */
  uint32_t width; /*!< Image width */
  uint32_t height; /*!< Image height */
  uint16_t nplanes; /*!< Number of planes */
  uint16_t bitspp; /*!< Bits per pixels */
  uint32_t compress; /*!< Compression */
  uint32_t isize; /*!< Image size */
  uint32_t hres; /*!< X pixels per meter */
  uint32_t vres; /*!< Y pixels per meter */
  uint32_t colors; /*!< Number of color in color table */
  uint32_t impcolors; /*!< Important color count */
} 
BMP_DIPHeader_t;

/** 
 * Try to open a file as bitmap and check for bitmap header
 */
byte readBitmapHeader(File &fi, int *width, int *height, byte *padding) {

  /* Misc. variables */
  BMP_Header_t bmpHeader;
  BMP_DIPHeader_t dipHeader;

  /* Try to read BMP header */
  if(fi.read((byte*) &bmpHeader, sizeof(BMP_Header_t)) != sizeof(BMP_Header_t))
    return FORMAT_ERROR;

  /* Check header signature */
  if(bmpHeader.magic[0] != 'B' | bmpHeader.magic[1] != 'M' | bmpHeader.offset != 54)
    return FORMAT_ERROR;

  /* Try to read BMP DIP header */
  if(fi.read((byte*) &dipHeader, sizeof(BMP_DIPHeader_t)) != sizeof(BMP_DIPHeader_t))
    return FORMAT_ERROR;

  /* Check DIP infos */
  if(dipHeader.size != sizeof(BMP_DIPHeader_t))
    return FORMAT_ERROR;
  if(dipHeader.bitspp != 24)
    return UNSUPPORTED_OPTION;
  if(dipHeader.compress != 0)
    return UNSUPPORTED_OPTION;

  /* Set image props. */
  *width = dipHeader.width;
  *padding = dipHeader.width & 3; //% 4;
  *height = dipHeader.height;

  /* No error */
  return NO_ERROR;
}

/**
 * Read the next pixel from the file
 */
byte readNextPixel(File &fi, byte *r, byte *g, byte *b) {

  /* Check for enought data */
  if(fi.available() < 3)
    return FILE_ERROR;

  /* Get pixel data */
  *b = fi.read();
  *g = fi.read();
  *r = fi.read();
  
  /* No error */
  return NO_ERROR;
}

