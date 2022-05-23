#! /bin/bash

set -x

export INSTALLDIR=$(pwd)/mist/

mkdir -p $INSTALLDIR

autoreconf -vfi

./configure --prefix=$INSTALLDIR

make install

cp COPYING AUTHORS $INSTALLDIR

tar cvzf mist_linux.tar.gz mist/

mkdir -p website

cp mist_linux.tar.gz website/

