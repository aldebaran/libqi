#!/bin/bash -xe

# required because we install qibuild with
# pip install qibuild --user
export PATH=$HOME/local/bin:$PATH

TRAVIS_CLONE=$(pwd)
mkdir -p /tmp/work
cd /tmp/work
qisrc init git://github.com/aldebaran/manifest.git
qitoolchain create --name linux64 linux64 git://github.com/aldebaran/toolchains.git
qibuild add-config linux64 -t linux64
cd sdk/libqi
git fetch ${TRAVIS_CLONE}
git reset --hard FETCH_HEAD
qibuild configure --config linux64 --release
qibuild make --config linux64 --njobs `nproc`
qitest run --config linux64
