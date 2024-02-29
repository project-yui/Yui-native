const fs = require('fs')
const path = require('path')

const target = path.resolve(__dirname, '../thirds/sqlite3/sqlite3.c')

const c = fs.readFileSync(target).toString()
const m = c.matchAll(/[ |=]([a-zA-Z0-9_]+)\(/gm)
let d = m.next()
let funcList = []
const ignore = [
  'defined',
  'warning',
  'assert',
  'strncmp',
  'malloc',
  'free',
  'printf',
  'void',
  'int',
  'if',
  'for',
  'memset',
  'sizeof',
  'while',
  'memcpy',
]
while (!d.done) {
  const f = d.value[1]
  if (!ignore.includes(f)) {
    if (!funcList.includes(f)){
      console.log(f)
      funcList.push(f)
    }
  }
  d = m.next()
}
funcList = funcList.sort()
fs.writeFileSync(path.resolve(__dirname, './func.json'), JSON.stringify(funcList, null, 4))