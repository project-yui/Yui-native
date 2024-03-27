#!/bin/bash

root_dir=$(cd `dirname $0`/../.. && pwd -P)

protoc --decode_raw < "$root_dir/tmp/pic-only.bin"
