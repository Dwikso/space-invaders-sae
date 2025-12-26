#!/bin/bash
set -e

ROOT_DIR=$(pwd)

echo "🔁 Mise à jour des submodules"
git submodule update --init --recursive

build_one () {
  echo "🔧 Build $1"
  pushd "$ROOT_DIR/$1" > /dev/null
  mkdir -p build
  pushd build > /dev/null
  cmake ..
  make -j$(nproc)
  popd > /dev/null
  popd > /dev/null
}

build_one external/SDL3
build_one external/SDL3_image
build_one external/SDL3_mixer
build_one external/SDL3_ttf

echo "🚀 Build du projet"
make
