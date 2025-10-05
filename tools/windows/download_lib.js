
const http = require('https');
const fs = require('fs');
const path = require('path');

// https://nodejs.org/dist/v22.13.1/win-x64/node.lib
function downloadFileAsync(uri, dest){
  console.log(`Downloading ${uri} to ${dest}`);
  return new Promise((resolve, reject)=>{
      // 确保dest路径存在
    const file = fs.createWriteStream(dest);

    http.get(uri, (res)=>{
      if(res.statusCode !== 200){
        reject(response.statusCode);
        return;
      }

      res.on('end', ()=>{
        console.log('download end');
      });

      // 进度、超时等

      file.on('finish', ()=>{
        console.log('finish write file')
        file.close(resolve);
      }).on('error', (err)=>{
        fs.unlink(dest);
        reject(err.message);
      })

      res.pipe(file);
    });
  });
}

;(async () => {
    await downloadFileAsync(`https://nodejs.org/dist/v${process.versions.node}/win-x64/node.lib`, path.resolve(__dirname, '../../build/node.lib'))
})()