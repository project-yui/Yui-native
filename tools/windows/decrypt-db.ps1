try {
    $root_dir = Resolve-Path (Join-Path $PSScriptRoot "../../")
    $cache_dir = Join-Path $root_dir "cache"
    $sqlcipher_dir = Join-Path $cache_dir "sqlcipher"

    # sqlcipher构建参考：
    # https://www.domstamand.com/compiling-sqlcipher-sqlite-encrypted-for-windows-using-visual-studio-2022/
    $Env:Path = "$sqlcipher_dir;$Env:Path"


    $db_dir = Join-Path $root_dir "tmp/db"
    $db_file="$db_dir/nt_msg.db"
    $db_clean_file="$db_dir/nt_msg.clean.db"
    $db_sql_file="$db_dir/nt_msg.sql"
    $db_decrypt_file="$db_dir/nt_msg.decrypt.db"

    $password="U2_Hs=p``sy2{TUf="
    

    function clean {
        param (
            $inputFile,
            $outputFile
        )
        # $inputFile = $db_file
        # $outputFile = $db_clean_file
        $bytesToSkip = 1024
        Write-Host "input: $inputFile"

        $inFileStream = [System.IO.File]::OpenRead($inputFile)
        $outFileStream = [System.IO.File]::Create($outputFile)

        $inFileStream.Seek($bytesToSkip, [System.IO.SeekOrigin]::Begin) | Out-Null

        $buffer = New-Object byte[] 4096
        while (($read = $inFileStream.Read($buffer, 0, $buffer.Length)) -gt 0) {
            $outFileStream.Write($buffer, 0, $read)
        }

        $inFileStream.Close()
        $outFileStream.Close()
    }
    clean $db_file $db_clean_file
    if (Test-Path "$db_decrypt_file")
    {
        Write-Output "删除旧的解密文件"
        Remove-Item "$db_decrypt_file"
    }
    sqlite3 --version

    Write-Output "db_decrypt_file: $db_decrypt_file"
    $SqlCipherCommands = @"
PRAGMA cipher_default_kdf_iter = 4000;
PRAGMA cipher_default_hmac_algorithm = HMAC_SHA1;
.open '$db_clean_file'
PRAGMA kdf_iter = 4000;
PRAGMA hmac_algorithm = HMAC_SHA1;
PRAGMA key = '$password';
ATTACH DATABASE '$db_decrypt_file' AS paintext KEY '';
SELECT sqlcipher_export('paintext');
DETACH DATABASE paintext;
.q
"@
    Write-Output "start executing..."
    $SqlCipherCommands | sqlite3
}
catch {
	Write-Host "发生异常：$_" -ForegroundColor:Red
	exit 1
}
