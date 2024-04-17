param($arch, $tag)
$root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")

cd $root_dir
try
{
    cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S"$root_dir" -B"$root_dir/build" -G Ninja
    cmake --build "$root_dir/build" --config Debug --target nt_native --
    mkdir "$root_dir/tmp/build"
    mv "$root_dir/build/nt_native.node" "$root_dir/tmp/build/telecord-native-win32-$arch-$tag.node"
}catch{
    exit 1
}