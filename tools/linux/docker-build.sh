#!/bin/bash
set -ex
root_dir=$(cd `dirname $0`/../.. && pwd -P)

docker build -t yui-native-build:latest --build-arg BUILD_USER=docker -f $root_dir/Dockerfile .
docker run --rm -i -w /workspace -v "$root_dir:/workspace/source" -v "$VCPKG_ROOT:/workspace/vcpkg" -e VCPKG_ROOT=/workspace/vcpkg yui-native-build:latest \
    bash /workspace/source/tools/linux/build.sh $@