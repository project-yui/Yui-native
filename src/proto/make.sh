#!/bin/bash
root_dir=$(cd `dirname $0` && pwd -P)


protoc -I=$root_dir/data --cpp_out=$root_dir $root_dir/data/message.proto