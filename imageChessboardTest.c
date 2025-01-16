// This module creates various Chessboard pattern images
// with different number of rows (m), columns (n) and square_edge values.
//
// List of arguments passed in:
//
// variant : holds the variable that will be studied
// Arguments passed in when m is variant
// min_m : minimum value that m should have;
// inc_m : increment value in between iterations for m;
// max_m : maximum value that m should have;
// n : hardcoded value of n
// s : hardcoded value of s
//
// Arguments passed in when n is variant
// min_n : minimum value that n should have;
// inc_n : increment value in between iterations for n;
// max_n : maximum value that n should have;
// m : hardcoded value of m
// s : hardcoded value of s
//
// Arguments passed in when s is variant
// min_s : minimum value that s should have;
// inc_s : increment value in between iterations for s;
// max_s : maximum value that s should have;
// m : hardcoded value of m
// n : hardcoded value of n

// Created by : Marta Cruz (119572)
// 26/11/2024

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imageBW.h"
#include "instrumentation.h"

int main(int argc, char *argv[])
{
  if (argc < 7)
  {
    fprintf(stderr, "Usage: ./program variant min_m[n s] inc_m[n s] max_m[n s] [m n s]\n");
    return 1;
  }

  ImageInit();

  char variant = argv[1][0];
  int min_m, inc_m, max_m, min_n, inc_n, max_n, min_s, inc_s, max_s, n, s, m;

  switch (variant)
  {
  case 'm':
    // For 'm' variant (n and s constant)
    min_m = atoi(argv[2]);
    inc_m = atoi(argv[3]);
    max_m = atoi(argv[4]);
    n = atoi(argv[5]);
    s = atoi(argv[6]);

    for (m = min_m; m < max_m; m += inc_m)
    {
      // s = m < n ? m : n; // for best case
      // s = 1; // for worst case
      int randomBit = rand() % 2;
      InstrReset();
      Image img = ImageCreateChessboard(n, m, s, randomBit);
      InstrPrintTest();
      ImageDestroy(&img);
    }
    break;

  case 'n':
    // For 'n' variant (m and s constant)
    min_n = atoi(argv[2]);
    inc_n = atoi(argv[3]);
    max_n = atoi(argv[4]);
    m = atoi(argv[5]);
    s = atoi(argv[6]);

    for (n = min_n; n < max_n; n += inc_n)
    {
      // s = m < n ? m : n; // for best case
      // s = 1; // for worst case
      int randomBit = rand() % 2;
      InstrReset();
      Image img = ImageCreateChessboard(n, m, s, randomBit);
      InstrPrintTest();
      ImageDestroy(&img);
    }
    break;

  case 's':
    // For 's' variant (m and n constant)
    min_s = atoi(argv[2]);
    inc_s = atoi(argv[3]);
    max_s = atoi(argv[4]);
    m = atoi(argv[5]);
    n = atoi(argv[6]);

    for (s = min_s; s < max_s; s += inc_s)
    {
      int randomBit = rand() % 2;
      InstrReset();
      Image img = ImageCreateChessboard(n, m, s, randomBit);
      InstrPrintTest();
      ImageDestroy(&img);
    }
    break;

  default:
    fprintf(stderr, "Unknown variant '%c'. Use 'm', 'n', or 's'.\n", variant);
    return 1;
  }

  return 0;
}
