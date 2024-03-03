#!/bin/bash

root_dir=$(cd `dirname $0` && pwd -P)


# https://github.com/protocolbuffers/protobuf/releases/tag/v21.12
protoc -I=$root_dir/data --cpp_out=$root_dir $root_dir/data/message.proto
