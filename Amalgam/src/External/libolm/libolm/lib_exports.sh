#!/bin/sh
# Print public API (ground truth list of functions exported from the built library)
readelf -W --dyn-syms "$(dirname "$0")"/build/libolm.so | grep -v UND | grep olm_ | awk '{ print $8 }' | sort -u
