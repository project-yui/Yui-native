const fs = require('fs')
const path = require('path')

const data = fs.readFileSync(path.resolve(__dirname, '../build/output.log')).toString()
const lines = data.split('\n')

const result = {}
for (const line of lines) {
  const m = line.match(/\[([a-z0-9]+-[a-z0-9]+-[a-z0-9]+-[a-z0-9]+-[a-z0-9]+)\]/)
  if (m == null) continue
  const uuid = m[1]
  console.log('uuid:', uuid)
  if (result[uuid] === undefined) {
    result[uuid] = false
  }
  else if (result[uuid] === false) {
    delete result[uuid]
  }
}
console.log(result)