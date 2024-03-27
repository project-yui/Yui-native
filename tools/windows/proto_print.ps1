$root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")
$cache_dir = Join-Path $root_dir "cache"
Write-Host $root_dir

$Env:Path="$cache_dir/protoc-${protoc_ver}/bin;$Env:Path"

protoc --decode_raw < "$root_dir/tmp/msg-example/multi-msg.bin"
