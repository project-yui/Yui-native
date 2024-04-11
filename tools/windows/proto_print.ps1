$root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")
Write-Host $root_dir

$protoc_ver = "21.12"

$Env:Path="$cache_dir/protoc-${protoc_ver}/bin;$Env:Path"

# cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-1.bin"
# cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-2.bin"
# cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-3.bin"
# cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-4.bin"
# cmd /C "protoc --decode_raw < $root_dir/test/msg_example/pic-only-gen.bin"

# 335438501
# 00010011111111100110001010100101
# 0001 0011111 1111001 1000101 0100101
# 0100101 1000101 1111001 0011111 0001
# 10100101 11000101 11111001 10011111 00000001
# 1010010111000101111110011001111100000001
# A5 C5 F9 9F 01

# 1690127128
# 01100100101111010100101100011000
# 0110 0100101 1110101 0010110 0011000
# 0011000 0010110 1110101 0100101 0110
# 10011000 10010110 11110101 10100101 00000110
# 1001100010010110111101011010010100000110
# 98 96 F5 A5 06

Write-Host "-----------test1:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test1.bin"
# 0A 17 08 B3 A7 83 BD 03 12 09(str_len) 39 33 33 32 38 36
# 38 33 35(str_end) 28 A5 C5 F9 9F 01 12 1C 
# 08 DC 05 10 14
# 18 14 28 AA 59 30 D2 E5 9A B0 06 60 DC 85 CC 88
# 80 80 80 80 02 68 04


Write-Host "-----------test2:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test2.bin"
# 0A 17 08 B3 A7 83 BD 03 12 09 39 33 33 32 38 36    ...3'.=...933286
# 38 33 35 28 A5 C5 F9 9F 01 12 1C 08 DC 05 10 14    835(%Ey.....\...
# 18 14 28 C9 48 30 D2 E5 9A B0 06 60 DC 85 B8 8B    ..(IH0Re.0.`\.8.
# 80 80 80 80 02 68 04                               .....h.

Write-Host "-----------test3:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test3.bin"
Write-Host "-----------test4:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test4.bin"
Write-Host "-----------test5:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test5.bin"
Write-Host "-----------test6:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test6.bin"
Write-Host "-----------test7:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test7.bin"
Write-Host "-----------test8:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test8.bin"
Write-Host "-----------test9:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test9.bin"
Write-Host "-----------test10:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test10.bin"

Write-Host "-----------test:"
cmd /C "protoc --decode_raw < $root_dir/test/msg_example/test.bin"
