#!/bin/sh

cd $(dirname $0) || exit

if [ ! -d build ]; then
  mkdir ./src/libfacedetection/build
else
  rm -r ./src/libfacedetection/build
fi

cd ./src/libfacedetection/build || exit
cmake .. -DCMAKE_INSTALL_PREFIX=../../../build -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DDEMO=OFF || exit
cmake --build . --config Release || exit
cmake --build . --config Release --target install || exit
