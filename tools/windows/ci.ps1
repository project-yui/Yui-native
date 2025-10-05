param($arch, $tag)

# Fail fast: convert non-terminating errors to terminating and stop on any error
$ErrorActionPreference = 'Stop'

$root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")

cd $root_dir
try
{
    cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S"$root_dir" -B"$root_dir/build" -G Ninja
    cmake --build "$root_dir/build" --config Release --target nt_native --
    cmake --build "$root_dir/build" --config Release --target QQNT --
    mkdir "$root_dir/tmp/build"
    mv "$root_dir/build/nt_native.node" "$root_dir/tmp/build/yui-native-win32-$arch-$tag.node"
    mv "$root_dir/build/QQNT.dll" "$root_dir/tmp/build/node-win32-$arch-$tag.dll"
}catch{
    exit 1
}