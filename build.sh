#!/bin/bash

INIT_PATH=$PWD

if [ ! -d 3rdParty/SDL3/build ]
then
cd 3rdParty/SDL3/
mkdir build
cd build
cmake ..
make -j
fi

if [ ! -d $INIT_PATH/3rdParty/SDL3_image/build ]
then
cd $INIT_PATH/3rdParty/SDL3_image/
mkdir build
cd build
cmake .. -DSDL3_DIR=../SDL3/build
make -j
fi

if [ ! -d $INIT_PATH/3rdParty/SDL3_mixer/build ]
then
cd $INIT_PATH/3rdParty/SDL3_mixer/
mkdir build
cd build
cmake .. -DSDL3_DIR=../SDL3/build
make -j
fi

if [ ! -d $INIT_PATH/3rdParty/SDL3_ttf/build ]
then
cd $INIT_PATH/3rdParty/SDL3_ttf/
mkdir build
cd build
cmake .. -DSDL3_DIR=../SDL3/build
make -j
fi

cd $INIT_PATH
make -j
