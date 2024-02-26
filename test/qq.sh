#!/bin/bash

root_dir=$(cd `dirname $0`/.. && pwd -P)

exe_dir=$(echo ~/ntqq/qq/opt/QQ)

export ELECTRON_RUN_AS_NODE=1
"$exe_dir/qq" $root_dir/test/test.js