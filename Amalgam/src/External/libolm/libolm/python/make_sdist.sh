#!/bin/bash
set -e

DIR=$(mktemp -d)
SRC=$(pwd)

echo "Making headers"
make headers

cd $DIR

echo "Copying python module"
cp -a $SRC/* .
mkdir -p libolm
echo "Cleaning sources"
make clean > /dev/null
cp -a $SRC/include .
echo "Copying libolm sources"
for src in cmake CMakeLists.txt common.mk include lib Makefile olm.pc.in src tests; do
    cp -a $SRC/../$src libolm
done
find libolm -name \*~ -delete
find libolm -name \#\*\# -delete

echo "Building"
patch -p1 < $SRC/packaging.diff
python3 -m build -s

echo "Copying result"
mkdir -p $SRC/dist
cp dist/* $SRC/dist

echo "Cleaning up"
cd $SRC
rm -rf $DIR
