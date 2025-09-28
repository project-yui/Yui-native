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
root_dir=$(cd `dirname $0`/../.. && pwd -P)

# 检查cmake装了没有
if ! command -v cmake &> /dev/null; then
    echo "cmake could not be found, installing it now..."
    sed -i 's@//.*archive.ubuntu.com@//mirrors.ustc.edu.cn@g' /etc/apt/sources.list
    sed -i 's/security.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list
    # 没装说明是github action环境
    apt update && DEBIAN_FRONTEND=noninteractive apt-get install -y cmake ninja-build zip pkg-config gcc g++ curl git
fi

export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion

# 检查nvm是否安装
if ! command -v nvm &> /dev/null; then
    echo "nvm could not be found, installing it now..."
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash
    export NVM_DIR="$HOME/.nvm"
    [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
    [ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion
fi

# 检查node是否安装
if ! command -v node &> /dev/null; then
    echo "Node.js could not be found, installing it now..."
    nvm install --lts
    nvm use --lts
fi
node --version
corepack enable
export SHELL=/bin/bash
pnpm setup
source ~/.bashrc
pnpm --version
cd $root_dir
pnpm install --force

rm -rf build
mkdir -p build
# cd build
cmake -S./ -B./build -DCMAKE_BUILD_TYPE:STRING=Release -G Ninja
cmake --build ./build --config Release --target nt_native
cd $root_dir
mkdir -p tmp/build
cp build/*.node "tmp/build/telecord-native-linux-$1-$2.node"