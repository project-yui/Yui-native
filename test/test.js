const native = require('../build/nt_native.node')
const os = require('os')
const home = os.homedir()
const wrapper = require(`${home}/ntqq/qq/opt/QQ/resources/app/wrapper.node`)


console.log(wrapper)
console.log('custom:', native.test())
process.exit(1)