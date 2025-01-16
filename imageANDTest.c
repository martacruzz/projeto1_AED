// This module creates various Chessboard pattern images
// with different number of rows (m), columns (n) and square_edge values
// and calculates the logical and operation on both images.
//
// List of arguments passed in:
//
// variant : holds the variable that will be studied
//
// Arguments passed in when m is variant
// min_m : minimum value that m should have;
// inc_m : increment value in between iterations for m;
// max_m : maximum value that m should have;
// n : hardcoded number of columns
// s : hardcoded number of square edge
//
// Arguments passed in when n is variant
// min_n : minimum value that n should have;
// inc_n : increment value in between iterations for n;
// max_n : maximum value that n should have;
// m : hardcoded number of rows
// s : hardcoded number of square edge
//
// Created by : Marta Cruz (119572)
// 27/11/2024

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imageBW.h"
#include "instrumentation.h"

int main(int argc, char *argv[])
{
  if (argc != 7)
  {
    fprintf(stderr, "Usage: ./program variant min_m[n] inc_m[n] max_m[n] [m n] V1_or_V2[V1 V2]\n");
    return 1;
  }

  ImageInit();

  char variant = argv[1][0];
  int min_m, inc_m, max_m, min_n, inc_n, max_n, n, m, s;

  switch (variant)
  {
  case 'm':
    // For 'm' variant (n and s constant)
    min_m = atoi(argv[2]);
    inc_m = atoi(argv[3]);
    max_m = atoi(argv[4]);
    n = atoi(argv[5]);

    for (m = min_m; m < max_m; m += inc_m)
    {
      s = 1; // stabilize s as worst case

      InstrReset();
      int randomBit = rand() % 2;
      // Image img1 = ImageCreateChessboard(n, m, s, randomBit);
      Image img1 = ImageCreate(n, m, 0); // for best case

      randomBit = rand() % 2;
      Image img2 = ImageCreateChessboard(n, m, s, randomBit);
      // Image img2 = ImageCreate(n, m, 1); // for best case

      ImageAND(img1, img2);

      InstrPrintTest();
      ImageDestroy(&img1);
      ImageDestroy(&img2);
    }
    break;

  case 'n':
    // For 'n' variant (m and s constant)
    min_n = atoi(argv[2]);
    inc_n = atoi(argv[3]);
    max_n = atoi(argv[4]);
    m = atoi(argv[5]);

    for (n = min_n; n < max_n; n += inc_n)
    {
      s = 1; // stabilize s as worst case

      InstrReset();
      int randomBit = rand() % 2;
      // Image img1 = ImageCreateChessboard(n, m, s, randomBit);
      Image img1 = ImageCreate(n, m, 0); // for best case

      randomBit = rand() % 2;
      // Image img2 = ImageCreateChessboard(n, m, s, randomBit);
      Image img2 = ImageCreate(n, m, 1); // for best case

      ImageAND(img1, img2);
      
      InstrPrintTest();
      ImageDestroy(&img1);
      ImageDestroy(&img2);
    }
    break;

  default:
    fprintf(stderr, "Unknown variant '%c'. Use 'm' or 'n'.\n", variant);
    return 1;
  }

  return 0;
}