#!/bin/bash
root_dir=$(cd `dirname $0`/../.. && pwd -P)

set -e
trap 'catchError $LINENO "$BASH_COMMAND"' ERR # 捕获错误情况
catchError() {
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        fail "\033[31mcommand: $2\n  at $0:$1\n  at $STEP\033[0m"
    fi
    exit $exit_code
}
notice() {
    echo -e "\033[36m $1 \033[0m "
}
fail() {
    echo -e "\033[41;37m 失败 \033[0m $1"
}

cd $root_dir
rm -rf build
mkdir -p build
# cd build
cmake -S./ -B./build -DCMAKE_BUILD_TYPE:STRING=Release -G Ninja
cmake --build ./build --config Release --target nt_native
cd $root_dir
mkdir -p tmp/build
cp build/*.node "tmp/build/telecord-native-linux-$1-$2.node"
