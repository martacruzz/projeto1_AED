#!/usr/bin/env bash

cd ..
make imageChessboardTest &> /dev/null #don't appear
./imageChessboardTest $1 $2 $3 $4 $5 $6 > testImageChessboard/data_imageChessboardTest.txt