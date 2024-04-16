$root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")

cd $root_dir
try
{
    cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S"$root_dir" -B"$root_dir/build" -G Ninja
    cmake --build "$root_dir/build" --config Debug --target nt_native --
}catch{
    exit 1
}