const fs = require('fs');
const path = require('path');
const { promisify } = require('util');
const rimraf = promisify(require('rimraf'));

async function firstFunction(){
    await rimraf(path.join(process.cwd(), "temp"));
    if(process.argv[2] == "all"){
        await rimraf(path.join(process.cwd(), "tools"));
    }
    try {
        fs.unlinkSync(path.join(process.cwd(), "stub-generator.exe"));   
    } catch (error) {    
    }
}
firstFunction();