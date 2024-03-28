$root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")
Write-Host $root_dir

$protoc_ver = "21.12"

$Env:Path="$cache_dir/protoc-${protoc_ver}/bin;$Env:Path"

cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-1.bin"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-2.bin"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-3.bin"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-4.bin"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-gen.bin"
