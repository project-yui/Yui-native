#!/bin/bash

root_dir=$(cd `dirname $0`/.. && pwd -P)

db_file="$root_dir/tmp/msg-example/nt_msg.db"
db_clean_file="$root_dir/tmp/msg-example/nt_msg.clean.db"
db_sql_file="$root_dir/tmp/msg-example/nt_msg.sql"
db_decrypt_file="$root_dir/tmp/msg-example/nt_msg.decrypt.db"

password="U2_Hs=p\`sy2{TUf="

cat "$db_file" | tail -c +1025 > "$db_clean_file"

if [ -f "$db_decrypt_file" ];then
  echo "删除旧的解密文件"
  rm $db_decrypt_file
fi

sqlcipher  <<EOF
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
EOF
