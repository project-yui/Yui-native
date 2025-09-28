#!/bin/bash
set -ex
root_dir=$(cd `dirname $0`/../.. && pwd -P)

docker run --rm -i -w /workspace -v "$root_dir:/workspace/source" -v "$VCPKG_ROOT:/workspace/vcpkg" -e VCPKG_ROOT=/workspace/vcpkg ubuntu:20.04 \
    bash /workspace/source/tools/linux/build.sh $@