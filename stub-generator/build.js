const { execSync } = require('node:child_process');
const hasha = require('hasha');
const fs = require('fs');
const pj = require('postject');
const Downloader = require('nodejs-file-downloader');
const yauzl = require("yauzl");
const fsp = require('fs/promises');
const path = require('path');

const upx = {
    32: 'https://github.com/upx/upx/releases/download/v4.2.4/upx-4.2.4-win32.zip',
    64: 'https://github.com/upx/upx/releases/download/v4.2.4/upx-4.2.4-win64.zip'
}
const upxHash = {
    32: '27bbae0b0c39cd8d95c0495d9d440304',
    64: '5e65d48b95cb1d9b633f22204dafee2c',
}

const resourceHacker = 'https://www.angusj.com/resourcehacker/resource_hacker.zip';
const resourceHackerHash = 'b1f79f26558459d1b401ba9ece2e8d66';

const h = {
    drawProgressbar: function(progress){
        var progressString = " " + progress + "%";
        var drawindLength = process.stdout.columns - 2 - progressString.length;
        var progressBar = '[';
        var barLength = Math.round(parseFloat(progress) * drawindLength / 100);
        var emptyLength = drawindLength - barLength;
        for(let i = 0; i < barLength; i++){
            if(i+1 == barLength && parseFloat(progress) < 99.9){
                progressBar = progressBar + '>';
            }else{
                progressBar = progressBar + '=';
            }
        }
        for(let i = 0; i < emptyLength; i++){
            progressBar = progressBar + ' ';
        }
        progressBar = progressBar + ']';
        //process.stdout.clearLine();
        process.stdout.cursorTo(0);
        process.stdout.write(progressBar + progressString);
    },
    downloadFile: async function (toDownload, saveLocation, toSave, progressReport){
        var lastRemainingSize = 1;
        
        const downloader = new Downloader({     
            url: toDownload,     
            directory: saveLocation,
            fileName: toSave,
            maxAttempts: 3,
            cloneFiles:false,
            onProgress:function(percentage, chunk, remainingSize){
                progressReport(percentage);
                lastRemainingSize = remainingSize;
            }      
        });
        
        await downloader.download();
    
        if(lastRemainingSize == 0){
        }else{
            await fsp.unlink(path.join(saveLocation, toSave));
            throw new Error("Error on downloading the \""+toDownload+"\" file, the download was interrupted unexpectedly.");
        }
    },
    extractZip: function(fileToExtract, whereExtract, skipRoot = false){
        const promise = new Promise((resolve, reject) => { 
            var zipBase = null;
            yauzl.open(fileToExtract, {lazyEntries: true, decodeStrings: true, strictFileNames: false}, function(err, zipfile) {
                if (err) reject(err);
                zipfile.on("entry", async function(entry) {
                    if (/\/$/.test(entry.fileName)) { //Do nothing if it is a directory, we don't want empty directories
                        zipfile.readEntry();
                    } else {
                        var splitDir = entry.fileName.split('/');
                        var rootDir = splitDir[0];
                        let toSlice = skipRoot ? 1 : 0;
                        if(splitDir.length < 2 && skipRoot){ //Don't extract root files
                            zipfile.readEntry();
                            return;
                        }
                        if(skipRoot){
                            //We only want to extract files from one directory on the root of the zip file
                            if(zipBase === null){
                                zipBase = rootDir;
                            }
                            if(zipBase != rootDir){
                                zipfile.readEntry();
                                return;
                            }
                        }
                        // file entry
                        await fsp.mkdir(path.join(whereExtract, path.posix.dirname(splitDir.slice(toSlice).join('/'))),{recursive: true});
                        zipfile.openReadStream(entry, function(err, readStream) {
                            var sStopFinishEvent = false;
                            if (err){
                                reject(err);
                                return;
                            }
                            var ws = fs.createWriteStream(path.join(whereExtract, splitDir.slice(toSlice).join('/')));
                            ws.on("finish", function() {
                                if(sStopFinishEvent == true) return;
                                zipfile.readEntry();
                            });
                            readStream.on("error", function(e) {
                                sStopFinishEvent = true;
                                ws.end();
                                reject(e);
                            });
                            ws.on("error", function(e) {
                                sStopFinishEvent = true;
                                reject(e);
                            });
                            if(entry.uncompressedSize <= 0){
                                sStopFinishEvent = true;
                                zipfile.readEntry();
                            }
                            readStream.pipe(ws);
                        });
                    }
                });
                zipfile.on("error", function(e) {
                    reject(e);
                });
                zipfile.once("end", function() {
                    resolve();
                });
                zipfile.readEntry();
            });
        });
        return promise;
    }
}

let bit = process.arch == 'x64' ? 64 : 32;

function copyStream(source, dest) {
    return new Promise((resolve, reject) => {
        let reader = fs.createReadStream(source);
        let writer = fs.createWriteStream(dest);
        reader.pipe(writer);
        reader.on('error', reject);
        writer.on('finish', resolve);
    });
}

async function firstFunction(){
    console.log('Recomended node version: 22.8.0');
    if(process.arch != 'x64' && process.arch != 'ia32'){
        console.log("You can't build this stub generator on this cpu architecture.");
        process.exit(72);
    }

    console.log('Creating required directories...');
    fs.mkdirSync('./temp', { recursive: true });
    fs.mkdirSync('./tools', { recursive: true });

    let tempFiles = fs.readdirSync('./temp');
    if(tempFiles.length > 0){
        console.log('Deleting temporary files...');
        tempFiles.forEach((file) => {
            fs.unlinkSync('./temp/' + file);
        });
    }

    if(!fs.existsSync('./tools/upx/upx.exe')){
        if(!fs.existsSync('./tools/upx.zip')){
            console.log('Downloading upx ...');
            await h.downloadFile(upx[bit], './tools', 'upx.zip', h.drawProgressbar);
        }
        console.log('Checking integrity of upx.zip ...');
        let hash = await hasha.fromFile('./tools/upx.zip', {algorithm: 'md5'});
        if(hash != upxHash[bit]){
            console.log('The file upx.zip is corrupted. Delete it and try again.');
            process.exit(72);
        }
        console.log('Extracting upx.zip ...');
        fs.mkdirSync('./tools/upx', { recursive: true });
        await h.extractZip('./tools/upx.zip', './tools/upx', true);
    }

    if(!fs.existsSync('./tools/rh/ResourceHacker.exe')){
        if(!fs.existsSync('./tools/resourcehacker.zip')){
            console.log('Downloading ResourceHacker ...');
            await h.downloadFile(resourceHacker, './tools', 'resourcehacker.zip', h.drawProgressbar);
        }
        console.log('Checking integrity of resourcehacker.zip ...');
        let hash = await hasha.fromFile('./tools/resourcehacker.zip', {algorithm: 'md5'});
        if(hash != resourceHackerHash){
            console.log('The file resourcehacker.zip is corrupted. Delete it and try again.');
            process.exit(72);
        }
        console.log('Extracting resourcehacker.zip ...');
        fs.mkdirSync('./tools/rh', { recursive: true });
        await h.extractZip('./tools/resourcehacker.zip', './tools/rh', false);
    }

    console.log('Building blob ...');
    execSync('node --experimental-sea-config sea-config.json', {stdio: 'inherit'});

    console.log('Copying node binary ...');
    fs.copyFileSync(process.execPath, './temp/node.exe');

    console.log('Compiling version resource ...');
    execSync('tools\\rh\\ResourceHacker.exe -open info.rc -save temp\\info.res -action compile', {stdio: 'inherit'});

    console.log('Adding icon resource ...');
    execSync('tools\\rh\\ResourceHacker.exe -open temp\\node.exe -save temp\\node.exe -action addoverwrite -res dataFrog.ico -mask ICONGROUP,IDR_MAINFRAME,', {stdio: 'inherit'});

    console.log('Adding version resource ...');
    execSync('tools\\rh\\ResourceHacker.exe -open temp\\node.exe -save temp\\node.exe -action addoverwrite -res temp\\info.res -mask versioninfo,1,', {stdio: 'inherit'});

    console.log('Adding blob ...');
    await pj.inject('./temp/node.exe', 'NODE_SEA_BLOB', fs.readFileSync('./temp/modern-stub.blob'), {
        sentinelFuse: 'NODE_SEA_FUSE_fce680ab2cc467b6e072b8b5df1996b2'
    });

    console.log('Compressing binary ...');
    execSync('tools\\upx\\upx.exe -9 -f ./temp/node.exe', {stdio: 'inherit'});

    console.log('Copying binary ...');
    await copyStream('./temp/node.exe', './stub-generator.exe');

    console.log('Removing temporary files ...');
    tempFiles = fs.readdirSync('./temp');
    tempFiles.forEach((file) => {
        fs.unlinkSync('./temp/' + file);
    });

    console.log('Done.');
}
firstFunction();