#!/bin/sh

set -e

make clean
rm -f olm-*.tgz

make lib
make test

. ~/.emsdk_set_env.sh
make js
(cd javascript && npm install && npm run test)
npm pack javascript
