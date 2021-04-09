#!/usr/bin/bash
set -exu

export CMAKE_GENERATOR="Visual Studio 16 2019"
export CMAKE_GENERATOR_ARCH="x64"

export PATH=$QTDIR/bin:$PATH

export BUILD_PATH=$APPVEYOR_BUILD_FOLDER/build
export CMAKE_PREFIX_PATH=$QTDIR/lib/cmake

APP_VERSION=$(git describe --tags --always HEAD)
export APP_VERSION
export APP=horizonengine-$APP_VERSION