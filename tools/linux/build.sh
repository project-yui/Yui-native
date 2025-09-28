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

apt update && DEBIAN_FRONTEND=noninteractive apt-get install -y cmake ninja-build zip pkg-config gcc g++ curl git

curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.3/install.sh | bash

export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion

nvm install --lts
nvm use --lts
node --version
corepack enable
export SHELL=/bin/bash
pnpm setup
source ~/.bashrc
pnpm --version
cd $root_dir
pnpm install

$root_dir/tools/linux/ci.sh $@
