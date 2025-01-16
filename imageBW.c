/// imageBW - A simple image processing module for BW images
///           represented using run-length encoding (RLE)
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
/// 2024

// Student authors (fill in below):
// NMec: 119572
// Name: Marta Cruz
// NMec: 119467
// Name: Catarina Ribeiro
//
// Date: 25/11/2024
//

#include "imageBW.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instrumentation.h"

// The data structure
//
// A BW image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the pointers
// to the RLE compressed image rows.
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Constant value --- Use them throughout your code
// const uint8 BLACK = 1;  // Black pixel value, defined on .h
// const uint8 WHITE = 0;  // White pixel value, defined on .h
const int EOR = -1; // Stored as the last element of a RLE row

// Internal structure for storing RLE BW images
struct image
{
  uint32 width;
  uint32 height;
  int **row; // pointer to an array of pointers referencing the compressed rows
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or
// file (I/O) operations use defensive techniques.
// When one of these functions fails,
// it immediately prints an error and exits the program.
// This fail-fast approach to error handling is simpler for the programmer.

// Use the following function to check a condition
// and exit if it fails.

// Check a condition and if false, print failmsg and exit.
static void check(int condition, const char *failmsg)
{
  if (!condition)
  {
    perror(failmsg);
    exit(errno || 255);
  }
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void)
{ ///
  InstrCalibrate();
  InstrName[0] = "pixmem"; // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  InstrName[1] = "numruns";  // InstrCount[1] will count the number of runs in an image
  InstrName[2] = "memspace"; // InstrCount[2] will keep track of memory space an image ocuppies
  InstrName[3] = "numops";   // InstrCount[3] will keep track of pixelwise operations in ImageAND()
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...
#define NUMRUNS InstrCount[1]
#define MEMSPACE InstrCount[2]
#define NUMOPS InstrCount[3]

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Auxiliary (static) functions

/// Create the header of an image data structure
/// And allocate the array of pointers to RLE rows
static Image AllocateImageHeader(uint32 width, uint32 height)
{
  assert(width > 0 && height > 0);
  Image newHeader = malloc(sizeof(struct image));
  check(newHeader != NULL, "malloc");

  newHeader->width = width;
  newHeader->height = height;

  // Allocating the array of pointers to RLE rows
  newHeader->row = malloc(height * sizeof(int *));
  check(newHeader->row != NULL, "malloc");

  return newHeader;
}

/// Allocate an array to store a RLE row with n elements
static int *AllocateRLERowArray(uint32 n)
{
  assert(n > 2);
  int *newArray = malloc(n * sizeof(int));
  check(newArray != NULL, "malloc");

  return newArray;
}

/// Compute the number of runs of a non-compressed (RAW) image row
static uint32 GetNumRunsInRAWRow(uint32 image_width, const uint8 *RAW_row)
{
  assert(image_width > 0);
  assert(RAW_row != NULL);

  // How many runs?
  uint32 num_runs = 1;
  for (uint32 i = 1; i < image_width; i++)
  {
    if (RAW_row[i] != RAW_row[i - 1])
    {
      num_runs++;
    }
  }

  return num_runs;
}

/// Get the number of runs of a compressed RLE image row
static uint32 GetNumRunsInRLERow(const int *RLE_row)
{
  assert(RLE_row != NULL);

  // Go through the RLE_row until EOR is found
  // Discard RLE_row[0], since it is a pixel color

  uint32 num_runs = 0;
  uint32 i = 1;
  while (RLE_row[i] != EOR)
  {
    num_runs++;
    i++;
  }

  return num_runs;
}

/// Get the number of elements of an array storing a compressed RLE image row
static uint32 GetSizeRLERowArray(const int *RLE_row)
{
  assert(RLE_row != NULL);

  // Go through the array until EOR is found
  uint32 i = 0;
  while (RLE_row[i] != EOR)
  {
    i++;
  }

  return (i + 1);
}

/// Compress into RLE format a RAW image row
/// Allocates and returns the array storing the image row in RLE format
static int *CompressRow(uint32 image_width, const uint8 *RAW_row)
{
  assert(image_width > 0);
  assert(RAW_row != NULL);

  // How many runs?
  uint32 num_runs = GetNumRunsInRAWRow(image_width, RAW_row);

  // Allocate the RLE row array ->> could have used AllocateRLERowArray() function
  int *RLE_row = malloc((num_runs + 2) * sizeof(int));
  check(RLE_row != NULL, "malloc");

  // Go through the RAW_row
  RLE_row[0] = (int)RAW_row[0]; // Initial pixel value
  uint32 index = 1;
  int num_pixels = 1;
  for (uint32 i = 1; i < image_width; i++)
  {
    if (RAW_row[i] != RAW_row[i - 1])
    {
      RLE_row[index++] = num_pixels;
      num_pixels = 0;
    }
    num_pixels++;
    NUMOPS++; // increment to account for the comparison made
  }
  RLE_row[index++] = num_pixels;
  RLE_row[index] = EOR; // Reached the end of the row

  return RLE_row;
}

static uint8 *UncompressRow(uint32 image_width, const int *RLE_row)
{
  assert(image_width > 0);
  assert(RLE_row != NULL);

  // The uncompressed row
  uint8 *row = (uint8 *)malloc(image_width * sizeof(uint8));
  check(row != NULL, "malloc");

  // Go through the RLE_row until EOR is found
  int pixel_value = RLE_row[0];
  uint32 i = 1;
  uint32 dest_i = 0;
  while (RLE_row[i] != EOR)
  {
    // For each run
    for (int aux = 0; aux < RLE_row[i]; aux++)
    {
      row[dest_i++] = (uint8)pixel_value;
      NUMOPS++; // increment to account for pixel assignments
    }
    // Next run
    i++;
    pixel_value ^= 1;
  }

  return row;
}

// Add your auxiliary functions here...
static uint32 lineIsEqual(int *line1, int *line2, uint32 size)
{
  for (uint32 i = 0; i < size; i++)
  {
    if (line1[i] != line2[i])
    {
      return 1;
    }
  }
  return 0;
}

// this version computes the logical operation AND between 2 encoded rows using RLE
static int *rowAND(int *arr1, int *arr2, uint32 size)
{
  int *rslt = AllocateRLERowArray(size + 2); // allocate for worst case
  uint32 index1 = 2, index2 = 2, rslt_index = 1;
  int value1 = arr1[0];
  int value2 = arr2[0];
  int len1 = arr1[1];
  int len2 = arr2[1];
  int prev_value;

  // calculate first pixel value of rslt
  rslt[0] = value1 & value2;

  while (len1 > 0 && len2 > 0)
  {
    int newValue = value1 & value2;
    NUMOPS++;
    int minlen = (len1 < len2) ? len1 : len2;

    if (rslt_index == 1 || (newValue != prev_value))
    {
      // append new run only if different from the last run
      rslt[rslt_index++] = minlen;
      // comparison between 2 pixel values -> increment NUMOPS
      NUMOPS++;
    }
    else
    {
      // extend the last run if equal to last run
      rslt[rslt_index - 1] += minlen;
      // comparison between 2 pixel values -> increment NUMOPS
      NUMOPS++;
    }

    // update lengths
    len1 -= minlen;
    len2 -= minlen;

    // if one run is exhausted, move to the next
    if (len1 == 0 && arr1[index1] != EOR)
    {
      value1 ^= 1;
      NUMOPS++; // pixel value toggle
      len1 = arr1[index1++];
    }
    if (len2 == 0 && arr2[index2] != EOR)
    {
      value2 ^= 1;
      NUMOPS++; // pixel value toggle
      len2 = arr2[index2++];
    }

    prev_value = newValue;
  }

  rslt[rslt_index++] = EOR;

  // resize the result array to match its actual size
  int *temp = realloc(rslt, rslt_index * sizeof(int));
  assert(rslt != NULL);
  rslt = temp; // to make sure rslt is not overwritten if realloc fails

  return rslt;
}

// this version computes the logical OR operation directly into RLE compressed rows
static int *rowOR(int *arr1, int *arr2, uint32 size)
{
  int *rslt = AllocateRLERowArray(size + 2); // allocate for worst case
  uint32 index1 = 2, index2 = 2, rslt_index = 1;
  int value1 = arr1[0];
  int value2 = arr2[0];
  int len1 = arr1[1];
  int len2 = arr2[1];
  int prev_value;

  // calculate first pixel value of rslt
  rslt[0] = value1 | value2;

  while (len1 > 0 && len2 > 0)
  {
    int newValue = value1 | value2;
    int minlen = (len1 < len2) ? len1 : len2;

    if (rslt_index == 1 || (newValue != prev_value))
    {
      // append new run only if different from the last run
      rslt[rslt_index++] = minlen;
    }
    else
    {
      // extend the last run if equal to last run
      rslt[rslt_index - 1] += minlen;
    }

    // update lengths
    len1 -= minlen;
    len2 -= minlen;

    // if one run is exhausted, move to the next
    if (len1 == 0 && arr1[index1] != EOR)
    {
      value1 ^= 1;
      len1 = arr1[index1++];
    }
    if (len2 == 0 && arr2[index2] != EOR)
    {
      value2 ^= 1;
      len2 = arr2[index2++];
    }

    prev_value = newValue;
  }

  rslt[rslt_index++] = EOR;

  // resize the result array to match its actual size
  int *temp = realloc(rslt, rslt_index * sizeof(int));
  assert(rslt != NULL);
  rslt = temp; // to make sure rslt is not overwritten if realloc fails

  return rslt;
}

// this version computes the logical OR operation directly into RLE compressed rows
static int *rowXOR(int *arr1, int *arr2, uint32 size)
{
  int *rslt = AllocateRLERowArray(size + 2); // allocate for worst case
  uint32 index1 = 2, index2 = 2, rslt_index = 1;
  int value1 = arr1[0];
  int value2 = arr2[0];
  int len1 = arr1[1];
  int len2 = arr2[1];
  int prev_value;

  // calculate first pixel value of rslt
  rslt[0] = value1 ^ value2;

  while (len1 > 0 && len2 > 0)
  {
    int newValue = value1 ^ value2;
    int minlen = (len1 < len2) ? len1 : len2;

    if (rslt_index == 1 || (newValue != prev_value))
    {
      // append new run only if different from the last run
      rslt[rslt_index++] = minlen;
    }
    else
    {
      // extend the last run if equal to last run
      rslt[rslt_index - 1] += minlen;
    }

    // update lengths
    len1 -= minlen;
    len2 -= minlen;

    // if one run is exhausted, move to the next
    if (len1 == 0 && arr1[index1] != EOR)
    {
      value1 ^= 1;
      len1 = arr1[index1++];
    }
    if (len2 == 0 && arr2[index2] != EOR)
    {
      value2 ^= 1;
      len2 = arr2[index2++];
    }

    prev_value = newValue;
  }

  rslt[rslt_index++] = EOR;

  // resize the result array to match its actual size
  int *temp = realloc(rslt, rslt_index * sizeof(int));
  assert(rslt != NULL);
  rslt = temp; // to make sure rslt is not overwritten if realloc fails

  return rslt;
}

/// Image management functions

/// Create a new BW image, either BLACK or WHITE.
///   width, height : the dimensions of the new image.
///   val: the pixel color (BLACK or WHITE).
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreate(uint32 width, uint32 height, uint8 val)
{
  assert(width > 0 && height > 0);
  assert(val == WHITE || val == BLACK);

  Image newImage = AllocateImageHeader(width, height);

  // All image pixels have the same value
  int pixel_value = (int)val;

  // Creating the image rows, each row has just 1 run of pixels
  // Each row is represented by an array of 3 elements [value,length,EOR]
  for (uint32 i = 0; i < height; i++)
  {
    newImage->row[i] = AllocateRLERowArray(3);
    newImage->row[i][0] = pixel_value;
    newImage->row[i][1] = (int)width;
    newImage->row[i][2] = EOR;
  }

  return newImage;
}

/// Create a new BW image, with a perfect CHESSBOARD pattern.
///   width, height : the dimensions of the new image.
///   square_edge : the lenght of the edges of the sqares making up the
///   chessboard pattern.
///   first_value: the pixel color (BLACK or WHITE) of the
///   first image pixel.
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
/// Requires: for the squares, width and height must be multiples of the
/// edge lenght of the squares
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreateChessboard(uint32 width, uint32 height, uint32 square_edge,
                            uint8 first_value)
{
  assert(width > 0 && height > 0);
  assert(first_value == WHITE || first_value == BLACK);
  // assert(width % square_edge == 0 && height % square_edge == 0);

  Image newImage = AllocateImageHeader(width, height);

  // number of rows considering the squares as base measurement
  uint32 n_square_cols = (width + square_edge - 1) / square_edge; // ceiling division to account for images where width is not multiple of square_edge
  int pixel_value = (int)first_value;

  // fill up the rows
  for (uint32 i = 0; i < height; i++)
  {
    newImage->row[i] = AllocateRLERowArray(2 + n_square_cols);
    MEMSPACE += (2 + n_square_cols) * sizeof(int);

    newImage->row[i][0] = pixel_value;

    // fill up runs
    for (uint8 k = 1; k <= n_square_cols; k++)
    {

      uint32 runlen;
      if (k == n_square_cols && width % square_edge != 0)
      {
        runlen = width % square_edge;
      }
      else
      {
        runlen = square_edge;
      }

      newImage->row[i][k] = runlen;
      NUMRUNS++; // incrememnt number of runs in image
    }

    newImage->row[i][1 + n_square_cols] = EOR; // 2 + n_cols - 1

    // if i is multiple of square_edge -> toggle pixel_value
    if ((i + 1) % square_edge == 0) // i+1 to avoid toggling when i = 0;
    {
      pixel_value ^= 1;
    }
  }

  return newImage;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail.
void ImageDestroy(Image *imgp)
{
  assert(imgp != NULL);

  Image img = *imgp;

  for (uint32 i = 0; i < img->height; i++)
  {
    free(img->row[i]);
  }
  free(img->row);
  free(img);

  *imgp = NULL;
}

/// Printing on the console

/// Output the raw BW image
void ImageRAWPrint(const Image img)
{
  assert(img != NULL);

  printf("width = %d height = %d\n", img->width, img->height);
  printf("RAW image:\n");

  // Print the pixels of each image row
  for (uint32 i = 0; i < img->height; i++)
  {
    // The value of the first pixel in the current row
    int pixel_value = img->row[i][0];
    for (uint32 j = 1; img->row[i][j] != EOR; j++)
    {
      // Print the current run of pixels
      for (int k = 0; k < img->row[i][j]; k++)
      {
        printf("%d", pixel_value);
      }
      // Switch (XOR) to the pixel value for the next run, if any
      pixel_value ^= 1;
    }
    // At current row end
    printf("\n");
  }
  printf("\n");
}

/// Output the compressed RLE image
void ImageRLEPrint(const Image img)
{
  assert(img != NULL);

  printf("width = %d height = %d\n", img->width, img->height);
  printf("RLE encoding:\n");

  // Print the compressed rows information
  for (uint32 i = 0; i < img->height; i++)
  {
    uint32 j;
    for (j = 0; img->row[i][j] != EOR; j++)
    {
      printf("%d ", img->row[i][j]);
    }
    printf("%d\n", img->row[i][j]);
  }
  printf("\n");
}

/// PBM BW file operations

// See PBM format specification: http://netpbm.sourceforge.net/doc/pbm.html

// Auxiliary function
static void unpackBits(int nbytes, const uint8 bytes[], uint8 raw_row[])
{
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8)
  { // or (mask > 0)
    for (int b = 0; b < nbytes; b++)
    {
      raw_row[8 * b + offset] = (bytes[b] & mask) != 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Auxiliary function
static void packBits(int nbytes, uint8 bytes[], const uint8 raw_row[])
{
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8)
  { // or (mask > 0)
    for (int b = 0; b < nbytes; b++)
    {
      if (offset == 0)
        bytes[b] = 0;
      bytes[b] |= raw_row[8 * b + offset] ? mask : 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE *f)
{
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n')
  {
    i++;
  }
  return i;
}

/// Load a raw PBM file.
/// Only binary PBM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageLoad(const char *filename)
{ ///
  int w, h;
  char c;
  FILE *f = NULL;
  Image img = NULL;

  check((f = fopen(filename, "rb")) != NULL, "Open failed");
  // Parse PBM header
  check(fscanf(f, "P%c ", &c) == 1 && c == '4', "Invalid file format");
  skipComments(f);
  check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width");
  skipComments(f);
  check(fscanf(f, "%d", &h) == 1 && h >= 0, "Invalid height");
  check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected");

  // Allocate image
  img = AllocateImageHeader(w, h);

  // Read pixels
  int nbytes = (w + 8 - 1) / 8; // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  uint8 raw_row[nbytes * 8];
  for (uint32 i = 0; i < img->height; i++)
  {
    check(fread(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
          "Reading pixels");
    unpackBits(nbytes, bytes, raw_row);
    img->row[i] = CompressRow(w, raw_row);
  }

  fclose(f);
  return img;
}

/// Save image to PBM file.
/// On success, returns nonzero.
/// On failure, returns 0, and
/// a partial and invalid file may be left in the system.
int ImageSave(const Image img, const char *filename)
{ ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  FILE *f = NULL;

  check((f = fopen(filename, "wb")) != NULL, "Open failed");
  check(fprintf(f, "P4\n%d %d\n", w, h) > 0, "Writing header failed");

  // Write pixels
  int nbytes = (w + 8 - 1) / 8; // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  // unit8 raw_row[nbytes*8];
  for (uint32 i = 0; i < img->height; i++)
  {
    // UncompressRow...
    uint8 *raw_row = UncompressRow(nbytes * 8, img->row[i]);
    // Fill padding pixels with WHITE
    memset(raw_row + w, WHITE, nbytes * 8 - w);
    packBits(nbytes, bytes, raw_row);
    check(fwrite(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
          "Writing pixels failed");
    free(raw_row);
  }

  // Cleanup
  fclose(f);
  return 0;
}

/// Information queries

/// Get image width
int ImageWidth(const Image img)
{
  assert(img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(const Image img)
{
  assert(img != NULL);
  return img->height;
}

/// Image comparison

// returns 1 if equal, 0 otherwise
int ImageIsEqual(const Image img1, const Image img2)
{
  assert(img1 != NULL && img2 != NULL);

  // first we check if width and height of images are the same -> otherwise already know they're different
  if ((img1->height != img2->height) || (img1->width != img2->width))
  {
    return 0;
  }

  // check row by row if encoding is equal
  for (uint32 i = 0; i < img1->height; i++)
  {
    if (lineIsEqual(img1->row[i], img2->row[i], GetSizeRLERowArray(img1->row[i])) != 0)
    {
      return 0;
    }
  }

  return 1;
}

int ImageIsDifferent(const Image img1, const Image img2)
{
  assert(img1 != NULL && img2 != NULL);
  return !ImageIsEqual(img1, img2);
}

/// Boolean Operations on image pixels

/// These functions apply boolean operations to images,
/// returning a new image as a result.
///
/// Operand images are left untouched and must be of the same size.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

Image ImageNEG(const Image img)
{
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // Directly copying the rows, one by one
  // And changing the value of row[i][0]

  for (uint32 i = 0; i < height; i++)
  {
    uint32 num_elems = GetSizeRLERowArray(img->row[i]);
    newImage->row[i] = AllocateRLERowArray(num_elems);
    memcpy(newImage->row[i], img->row[i], num_elems * sizeof(int));
    newImage->row[i][0] ^= 1; // Just negate the value of the first pixel run
  }

  return newImage;
}

// This is the optimized version of the algorithm
Image ImageAND(const Image img1, const Image img2)
{
  assert(img1 != NULL && img2 != NULL);
  assert((img1->height == img2->height) && (img1->width == img2->width));

  // allocate memory for the resulting image
  Image rslt = AllocateImageHeader(img1->width, img1->height);

  for (uint32 row_index = 0; row_index < img1->height; row_index++)
  {
    rslt->row[row_index] = rowAND(img1->row[row_index], img2->row[row_index], img1->width);
  }

  return rslt;
}

// This is the non-optimized version of imageAND()
// Image ImageAND(const Image img1, const Image img2)
// {
//   assert(img1 != NULL && img2 != NULL);
//   assert((img1->height == img2->height) && (img1->width == img2->width));

//   // allocate memory for the resulting image
//   Image rslt = AllocateImageHeader(img1->width, img1->height);

//   for (uint32 row_index = 0; row_index < img1->height; row_index++)
//   {
//     uint8 *row1 = UncompressRow(img1->width, img1->row[row_index]);
//     uint8 *row2 = UncompressRow(img2->width, img2->row[row_index]);

//     uint8 *rslt_row = (uint8 *)malloc(img1->width * sizeof(uint8));
//     check(rslt_row != NULL, "malloc");

//     // perform and operation pixelwise
//     for (uint32 j = 0; j < img1->width; j++)
//     {
//       rslt_row[j] = row1[j] & row2[j];
//       NUMOPS++; // increment due to logic and operation
//     }

//     int *aux = CompressRow(img1->width, rslt_row);
//     rslt->row[row_index] = aux;

//     free(rslt_row);
//     free(row1);
//     free(row2);
//     // we don't free aux as it is being assigned to rlst->row. Therefore it will only be freed when calling imageDestroy
//   }

//   return rslt;
// }

Image ImageOR(const Image img1, const Image img2)
{
  assert(img1 != NULL && img2 != NULL);
  assert((img1->height == img2->height) && (img1->width == img2->width));

  Image rslt = AllocateImageHeader(img1->width, img1->height);

  for (uint32 row_index = 0; row_index < img1->height; row_index++)
  {
    rslt->row[row_index] = rowOR(img1->row[row_index], img2->row[row_index], img1->width);
  }

  return rslt;

  return NULL;
}

Image ImageXOR(Image img1, Image img2)
{
  assert(img1 != NULL && img2 != NULL);
  assert((img1->height == img2->height) && (img1->width == img2->width));

  Image rslt = AllocateImageHeader(img1->width, img1->height);

  for (uint32 row_index = 0; row_index < img1->height; row_index++)
  {
    rslt->row[row_index] = rowXOR(img1->row[row_index], img2->row[row_index], img1->width);
  }

  return rslt;

  return NULL;
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

/// Mirror an image = flip top-bottom.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageHorizontalMirror(const Image img)
{
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // Iterates through each row in the original image, in reverse order.
  for (uint32 i = 0; i < height; i++)
  {
    int size = GetSizeRLERowArray(img->row[height - i - 1]); // Gets the size of the row (RLE compressed).
    newImage->row[i] = AllocateRLERowArray(size);            // Allocates memory for the new row.

    // Copies the data from the original image row to the new row.
    for (int j = 0; j < size; j++)
    {
      newImage->row[i][j] = img->row[height - i - 1][j];
    }
  }

  return newImage;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageVerticalMirror(const Image img)
{
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // Iterates through each row in the original image.
  for (uint32 i = 0; i < height; i++)
  {
    int num_runs = GetNumRunsInRLERow(img->row[i]);       // Gets the number of RLE runs in the row.
    newImage->row[i] = AllocateRLERowArray(num_runs + 2); // Allocates memory for the new row.

    // Handles the first RLE run differently based on its parity.
    if (num_runs % 2 != 0)
    {
      newImage->row[i][0] = img->row[i][0];
    }
    else
    {
      newImage->row[i][0] = img->row[i][0] ^ 1; // Flips the first run's parity.
    }

    // Reverses the order of the runs for the new image.
    for (int j = 1; j <= num_runs; j++)
    {
      newImage->row[i][j] = img->row[i][num_runs + 1 - j];
    }

    newImage->row[i][num_runs + 1] = -1; // End marker for the row.
  }

  return newImage;
}

/// Replicate img2 at the bottom of imag1, creating a larger image
/// Requires: the width of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtBottom(const Image img1, const Image img2)
{
  assert(img1 != NULL && img2 != NULL);
  assert(img1->width == img2->width);

  uint32 new_width = img1->width;
  uint32 new_height = img1->height + img2->height;

  Image newImage = AllocateImageHeader(new_width, new_height);

  // Copies the rows of the first image (img1) into the new image.
  for (uint32 i = 0; i < img1->height; i++)
  {
    // Allocates memory for the RLE row array for the current row of img1.
    newImage->row[i] = AllocateRLERowArray(GetSizeRLERowArray(img1->row[i]));

    // Copies each element of the current RLE row from img1 into the new image.
    for (uint32 j = 0; j < GetSizeRLERowArray(img1->row[i]); j++)
    {
      newImage->row[i][j] = img1->row[i][j];
    }
  }

  // Copies the rows of the second image (img2) into the new image, starting after img1's rows.
  for (uint32 i = 0; i < img2->height; i++)
  {
    // Allocates memory for the RLE row array for the current row of img2.
    newImage->row[img1->height + i] = AllocateRLERowArray(GetSizeRLERowArray(img2->row[i]));

    // Copies each element of the current RLE row from img2 into the new image.
    for (uint32 j = 0; j < GetSizeRLERowArray(img2->row[i]); j++)
    {
      newImage->row[img1->height + i][j] = img2->row[i][j];
    }
  }

  return newImage;
}

/// Replicate img2 to the right of imag1, creating a larger image
/// Requires: the height of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtRight(const Image img1, const Image img2)
{
  assert(img1 != NULL && img2 != NULL);
  assert(img1->height == img2->height);

  uint32 new_width = img1->width + img2->width;
  uint32 new_height = img1->height;

  Image newImage = AllocateImageHeader(new_width, new_height);

  for (uint32 i = 0; i < new_height; i++)
  {
    uint32 num_runs_img1 = GetNumRunsInRLERow(img1->row[i]);
    uint32 num_runs_img2 = GetNumRunsInRLERow(img2->row[i]);

    // Determines how to handle overlapping or adjoining RLE runs based on their properties.
    if ((num_runs_img1 % 2 != 0 && img1->row[i][0] != img2->row[i][0]) || (num_runs_img1 % 2 == 0 && img1->row[i][0] == img2->row[i][0]))
    {
      // Case 1: No merging of the last run from img1 and the first run from img2 is needed.
      uint32 num_runs_total = num_runs_img1 + num_runs_img2;      // Total number of runs in the combined row.
      newImage->row[i] = AllocateRLERowArray(num_runs_total + 2); // Allocates memory for the combined row.

      // Copies all runs from img1 into the new image's row.
      newImage->row[i][0] = img1->row[i][0]; // Copies the starting value.
      for (uint32 j = 1; j <= num_runs_img1; j++)
      {
        newImage->row[i][j] = img1->row[i][j];
      }

      // Copies all runs from img2 into the new image's row, starting where img1's runs end.
      newImage->row[i][num_runs_img1 + 1] = img2->row[i][1]; // Starts with the first run of img2.

      for (uint32 j = 2; j <= num_runs_img2; j++)
      {
        newImage->row[i][num_runs_img1 + j] = img2->row[i][j];
      }

      newImage->row[i][num_runs_total + 1] = -1; // Adds the end marker for the row.
    }
    else
    {
      // Case 2: Merges the last run from img1 with the first run from img2.
      uint32 num_runs_total = num_runs_img1 + num_runs_img2 - 1;  // Total number of runs after merging.
      newImage->row[i] = AllocateRLERowArray(num_runs_total + 2); // Allocates memory for the combined row.

      // Copies all runs from img1 into the new image's row.
      newImage->row[i][0] = img1->row[i][0]; // Copy the starting value.
      for (uint32 j = 1; j <= num_runs_img1; j++)
      {
        newImage->row[i][j] = img1->row[i][j];
      }

      // Merges the last run of img1 with the first run of img2.
      newImage->row[i][num_runs_img1] += img2->row[i][1]; // Add the lengths of the overlapping runs.

      // Copies the remaining runs from img2 into the new image's row.
      for (uint32 j = 1; j <= num_runs_img2; j++)
      {
        newImage->row[i][num_runs_img1 + j] = img2->row[i][j];
      }

      newImage->row[i][num_runs_total + 1] = -1; // Adds the end marker for the row.
    }
  }

  return newImage;
}
