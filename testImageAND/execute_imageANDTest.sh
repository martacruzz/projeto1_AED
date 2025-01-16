#!/usr/bin/env bash

cd ..
make imageANDTest &> /dev/null #don't appear
./imageANDTest $1 $2 $3 $4 $5 $6 > testImageAND/data_imageANDTest.txt