#!/bin/bash
root_dir=$(cd `dirname $0`/.. && pwd -P)

export YUI_LOG='true'
export ELECTRON_RUN_AS_NODE=1 
"$root_dir/ntqq/v3.2.5-21357-deb/qq" /home/msojocs/ntqq/nt-native/ntqq/v3.2.5-21357-deb/resources/app/app_launcher/index.js
