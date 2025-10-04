#!/bin/bash
set -ex
trap 'catchError $LINENO "$BASH_COMMAND"' ERR # 捕获错误情况
catchError() {
    exit_code=$?
    # tail -f /etc/issue
    if [ $exit_code -ne 0 ]; then
        fail "\033[31mcommand: $2\n  at $0:$1\n  at $STEP\033[0m"
    fi
    exit $exit_code
}
fail() {
    echo -e "\033[41;37m 失败 \033[0m $1"
}
root_dir=$(cd `dirname $0`/../.. && pwd -P)

export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
[ "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"
node --version
pnpm --version
cd $root_dir
pnpm install --force

# rm -rf build
mkdir -p build
# cd build
cmake -S./ -B./build -DCMAKE_BUILD_TYPE:STRING=Release -G Ninja
cmake --build ./build --config Release --target nt_native
cmake --build ./build --config Release --target preload
cd $root_dir
mkdir -p tmp/build
cp build/nt_native.node "tmp/build/nt-native-linux-$1-$2.node"
cp build/preload.node "tmp/build/nt-preload-linux-$1-$2.node"