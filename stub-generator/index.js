const fs = require('fs');
const fsp = fs.promises;
const readline = require('readline');
const path = require('path');
const SingleLine = Symbol();
const Doubleline = Symbol();

const reader = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    historySize: 0,
    prompt: '',
});

function startWithNumber(text){
    let startWithNumber = false;
    for(let j = 0; j < text.length; j++){
        if(text[j] >= '0' && text[j] <= '9'){
            startWithNumber = true;
            break;
        }
        if(text[j] == ' '){continue;}
        break;
    }
    return startWithNumber;
}

function startingNumberLength(text){
    if(!startWithNumber(text)){return 0;}
    let numberPrefixLen = 0;
    for(let j = 0; j < text.length; j++){
        if(text[j] >= '0' && text[j] <= '9'){
            numberPrefixLen++;
            continue;
        }else if(text[j] == '.'){
            numberPrefixLen++;
            continue;
        }else if(text[j] == ' '){
            numberPrefixLen++;
            continue;
        }
        break;
    }
    return numberPrefixLen;
}

function whiteSpaceLength(text){
    let whiteSpaceLen = 0;
    for(let j = 0; j < text.length; j++){
        if(text[j] == ' '){
            whiteSpaceLen++;
            continue;
        }
        break;
    }
    return whiteSpaceLen;
}

function isSingleWord(text){
    for(let j = 0; j < text.length; j++){
        if(text[j] == ' '){
            return false;
        }
    }
    return true;
}

const dashable = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'.split('');

function splitText(text, extra = 6){
    let terminalDimensions = process.stdout.getWindowSize();
    let maxTextSize = terminalDimensions[0] - extra;
    let toReturn = [];
    if(text.length > maxTextSize){
        let firstCharIdx = 0;
        let numberPrefixLen = startingNumberLength(text);
        let whiteSpaceLen = whiteSpaceLength(text);
        let singleWord = isSingleWord(text.slice(numberPrefixLen, text.length));

        while(firstCharIdx < text.length){
            finalIndex = firstCharIdx + maxTextSize;
            let ignoreNumber = false;
            let ignoreLimit = 13 - numberPrefixLen + whiteSpaceLen; 
            if(ignoreLimit < 8){ignoreLimit = 8;}
            let firstLine = firstCharIdx <= 0;
            if(firstCharIdx > 0 && finalIndex - numberPrefixLen > firstCharIdx + ignoreLimit){
                finalIndex = finalIndex - numberPrefixLen;
            }else{
                ignoreNumber = true;
            }
            if(firstCharIdx <= 0 && finalIndex - numberPrefixLen <= firstCharIdx + ignoreLimit){
                while(text[firstCharIdx] == ' '){
                    firstCharIdx++;
                    finalIndex++;
                }
            }
            if(finalIndex > text.length){
                finalIndex = text.length;
            }
            if(finalIndex < text.length && singleWord == false){
                for(let j = finalIndex;j > firstCharIdx;j--){
                    if(text[j] == ' '){
                        if(firstLine && j == numberPrefixLen - 1){
                            break;
                        }
                        finalIndex = j;
                        break;
                    }
                }
            }
            let dash = '';
            if(
                text[finalIndex] != ' ' &&
                text[finalIndex] != undefined &&
                dashable.includes(text[finalIndex-1])
            ){
                if(!firstLine || (finalIndex != numberPrefixLen)){
                    dash = '-';
                    finalIndex--;
                    if(firstLine && finalIndex == numberPrefixLen){
                        dash = ' ';
                    }
                }
            }
            if(finalIndex <= firstCharIdx + 1){
                finalIndex = firstCharIdx + 2;
            }
            if(ignoreNumber == true){
                toReturn.push(text.slice(firstCharIdx, finalIndex) + dash);
            }else{
                toReturn.push(
                    ' '.repeat(numberPrefixLen) +
                    text.slice(firstCharIdx, finalIndex) + dash
                );
            }
            firstCharIdx = finalIndex;
            while(text[firstCharIdx] == ' '){
                firstCharIdx++;
            }
        }
    }else{
        toReturn.push(text);
    }
    return toReturn;
}

function drawLine(){
    let terminalWidth = process.stdout.getWindowSize()[0];
    let toReturn = '='.repeat(terminalWidth);
    console.log(toReturn);
}

function preProcessBox(texts, minwidth=0){
    let toReplace = [];
    for (let i = 0; i < texts.length; i++) {
        if(texts[i] == SingleLine || texts[i] == Doubleline){
            toReplace.push(texts[i]);
            continue;
        }
        if(Array.isArray(texts[i])){
            splitText(texts[i][0]).forEach((text) => {
                toReplace.push([text]);
            });
            continue;
        }
        splitText(texts[i]).forEach((text) => {
            toReplace.push(text);
        });
    }
    texts = toReplace;
    let largestStringLength = minwidth;
    for(let i = 0; i < texts.length; i++){
        if(texts[i] == SingleLine || texts[i] == Doubleline){ continue; }
        if(Array.isArray(texts[i])){
            largestStringLength = Math.max(largestStringLength, texts[i][0].length);
            continue;
        }
        largestStringLength = Math.max(largestStringLength, texts[i].length);
    }
    return {
        texts: texts,
        largestStringLength: largestStringLength
    }
}

function drawBox(processed){
    let terminalWidth = process.stdout.getWindowSize()[0];
    let texts = processed.texts;

    let terminalHeight = process.stdout.getWindowSize()[1];
    let boxHeight = texts.length + 2;
    if(boxHeight < terminalHeight){
        let blankLines = Math.floor((terminalHeight - boxHeight)/2);
        for(let i = 0; i < blankLines; i++){
            console.log('');
        }
    }

    let boxWidth = processed.largestStringLength + 6;
    let outterSpacesCount = Math.floor((terminalWidth - boxWidth) / 2);
    let outterSpaces = ' '.repeat(outterSpacesCount);
    
    let line = outterSpaces;
    line += '╔';
    for(let i = 0; i < boxWidth - 2; i++){
        line += '═';
    }
    line += '╗';
    console.log(line);
    line = outterSpaces;
    for(let i = 0; i < texts.length; i++){
        if(texts[i] == SingleLine){
            line += '╟';
            for(let j = 0; j < boxWidth - 2; j++){
                line += '─';
            }
            line += '╢';
            console.log(line);
            line = outterSpaces;
            continue;
        }
        if(texts[i] == Doubleline){
            line += '╠';
            for(let i = 0; i < boxWidth - 2; i++){
                line += '═';
            }
            line += '╣';
            console.log(line);
            line = outterSpaces;
            continue;
        }
        if(Array.isArray(texts[i])){
            let spacesCount = (boxWidth - texts[i][0].length - 2 ) / 2;
            line += '║';
            for(let i = 0; i < Math.floor(spacesCount); i++){
                line += ' ';
            }
            line += texts[i][0];
            for(let i = 0; i < Math.ceil(spacesCount); i++){
                line += ' ';
            }
            line += '║';
            console.log(line);
            line = outterSpaces;
            continue;
        }
        let spacesCount = (boxWidth - texts[i].length - 2 - 2);
        line += '║  ';
        line += texts[i];
        for(let j = 0; j < spacesCount; j++){
            line += ' ';
        }
        line += '║';
        console.log(line);
        line = outterSpaces;
    }
    line += '╚';
    for(let i = 0; i < boxWidth - 2; i++){
        line += '═';
    }
    line += '╝';
    console.log(line);
    return outterSpacesCount;
}

function moveCursorBy(dx, dy){
    return new Promise((resolve, reject) => {
        readline.moveCursor(process.stdout, dx, dy, resolve);
    });
}

async function anyKey(){
    return new Promise((resolve, reject) => {
        process.stdin.setRawMode(true);
        process.stdin.resume();
        process.stdin.setEncoding('utf8');
        process.stdin.once('data', async (key) => {
            moveCursorBy(-1, 0);
            process.stdout.write(' ');
            resolve(key);
        });
    });
}

async function readChar(lineCount){
    return new Promise((resolve, reject) => {
        process.stdin.setRawMode(true);
        process.stdin.resume();
        process.stdin.setEncoding('utf8');
        process.stdin.once('data', async (key) => {
            if(key == '\u0003') {
                for(let i = 0; i < lineCount; i++){
                    console.log('');
                }
                process.exit(0);
            }
            let toReturn = key.toString().trim();
            resolve(toReturn);         
        });
    });
}

function getCursorLinearPosition(){
    return reader.getCursorPos().cols + (reader.getCursorPos().rows * process.stdout.getWindowSize()[0]);
}

async function readString(
    lineCount,
    redraw = (cursor) => {},
    updateLength = (length, cursor) => {},
    defaultAnswer = ''
){
    reader.line = defaultAnswer;
    reader.cursor = defaultAnswer.length;
    let lastCurRow = reader.getCursorPos().rows;
    redraw(getCursorLinearPosition());
    let safeKeys = '\u000DabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%&*( )-_=+[{]}\\|;:\'",<.>/?'.split('');
    process.stdin.setRawMode(true);
    process.stdin.resume();
    process.stdin.setEncoding('utf8');
    
    let exiter = async (key) => {
        if(key == '\u0003') {
            for(let i = 0; i < lineCount; i++){
                console.log('');
            }
            process.exit(0);
        }
        let curPos = getCursorLinearPosition();
        if(
            !safeKeys.includes(key) ||
            curPos < reader.line.length || 
            lastCurRow != reader.getCursorPos().rows
        ){
            redraw(curPos);
        }else{
            updateLength(reader.line.length, curPos);
        }
        lastCurRow = reader.getCursorPos().rows;
    }
    process.stdin.on('data', exiter);
    return new Promise((resolve, reject) => {
        reader.once('line', (line) => {
            process.stdin.off('data', exiter);
            resolve(line);
        });
    });
}

async function boxedQuestion(
    _before, question, after, validate, multichar = false, defaultAnswer = '',
){
    let between = ['', question];
    let option;
    let before = _before;
    if(Array.isArray(_before)){
        before = () => {return _before};
    }
    
    let preProcessedBox;
    let rowCount;
    let dashedQuestion;
    let db = (cursor = 0) => {
        process.stdout.write('\x1Bc');
        let beforecache = before();
        preProcessedBox = preProcessBox([
            ...beforecache,
            ...between,
            ...after
        ]);

        let questionBox = preProcessBox([question]);
        dashedQuestion = questionBox.texts
            .map((text, i) => i == questionBox.texts.length - 1 ? text : text + ' '.repeat(preProcessedBox.largestStringLength - text.length))
            .join('');
        
        let toAdd = 0;
        if(cursor == reader.line.length){
            toAdd = 1;
        }
        rowCount = Math.max(Math.ceil(
            (dashedQuestion.length + reader.line.length + toAdd) /
            preProcessedBox.largestStringLength
        ), 1);
        let processedBox = preProcessBox([
            ...beforecache,
            ...between,
            ...Array(rowCount - questionBox.texts.length + 1).fill(''),
            ...after
        ]);
        let boxCol = drawBox(processedBox);

        moveCursorBy(3 + boxCol, (after.length * -1) - 1 - rowCount - 1);
        
        let toWrite = dashedQuestion + reader.line;
        let startIndex = 0;
        for(let i = 0; i < rowCount; i++){
            let endIndex = startIndex + processedBox.largestStringLength;
            process.stdout.write(toWrite.substring(startIndex, endIndex));
            if(endIndex >= toWrite.length) break;
            moveCursorBy(processedBox.largestStringLength * -1, 1);
            startIndex = endIndex;
        }
        
        // moveCursorBy(0, 4);
        // console.log(questionBox);
        // return ;

        let positionsToMove = reader.line.length + dashedQuestion.length;
        let rowsToMove = 0;
        while(positionsToMove > processedBox.largestStringLength){
            positionsToMove -= processedBox.largestStringLength;
            rowsToMove -= 1;
        }
        moveCursorBy(positionsToMove * -1, rowsToMove);
        positionsToMove = cursor + dashedQuestion.length;
        rowsToMove = 0;
        while(positionsToMove >= processedBox.largestStringLength){
            positionsToMove = positionsToMove - processedBox.largestStringLength;
            rowsToMove += 1;
        }
        moveCursorBy(positionsToMove, rowsToMove);
        // process.stdout.write(positionsToMove + 'X' + rowsToMove + '-' + getCursorLinearPosition());
        // moveCursorBy(20, 3);
        // reader.line = '';
    };
    let upd = (length, cursor) => {
        if(length + dashedQuestion.length + 1 > preProcessedBox.largestStringLength){
            let rowsToMove = 0;
            let positionsToMove = cursor + dashedQuestion.length;
            while(positionsToMove >= preProcessedBox.largestStringLength){
                positionsToMove = positionsToMove - preProcessedBox.largestStringLength;
                rowsToMove += 1;
            }
            if(rowsToMove >= rowCount){
                db(cursor);
            }
        }
    }
    let db2 = () => {db(getCursorLinearPosition());}
    process.stdout.on('resize', db2);
    while(true){
        if(multichar == true){
            option = (await readString(after.length + 3, db, upd, defaultAnswer)).trim();
        }else{
            reader.line = '';
            db();
            option = (await readChar(after.length + 3)).trim();
        }
        if(await validate(option) == true) break;
        between = [
            '',
            'Invalid option, try again.',
            '',
            question,
        ]
    }
    process.stdout.off('resize', db2);
    return option;
}

function gropuOptions(options, colCount=5, extra = 6){
    let toReturn = [];
    while(colCount > 1){
        toReturn = [];
        let rows = [];
        let rowCount = Math.ceil(options.length / colCount);
        for(let i = 0; i < rowCount; i++){
            let toPush = [];
            for(let j = 0; j < colCount; j++){
                if(i + j * rowCount < options.length){
                    toPush.push(options[i + j * rowCount]);
                }
            }
            rows.push(toPush);
        }
        let longests = [];
        for(let i = 0; i < colCount; i++){
            longests.push(0);
        }
        for(let i = 0; i < colCount; i++){
            for(let j = 0; j < rows.length; j++){
                if(rows[j][i] != undefined && rows[j][i].length > longests[i]){
                    longests[i] = rows[j][i].length;
                }
            }
        }
        
        for(let i = 0; i < rows.length; i++){
            rows[i] = rows[i].map((row, index) => {
                return row + ' '.repeat(longests[index] - row.length)
            });
            toReturn.push(rows[i].join(' | '));
        }

        let rowLength = 0;
        for(let i = 0; i < toReturn.length; i++){
            rowLength = Math.max(rowLength, toReturn[i].length);
        }

        if(rowLength < process.stdout.getWindowSize()[0] - extra){
            return toReturn;
        }
        colCount--;
    }
    return options;
}

function parseCue(cueContent){
    try {
        let content = cueContent
            .split('\n').map(line => line.trim())
            .filter(line => line.trim().toLowerCase().startsWith('file'))
            .map(line => {
                let elements = line.split(' ');
                let firstElement = elements.findIndex(element => element.startsWith('"'));
                let lastElement = elements.findIndex(element => element.endsWith('"'));
                let toReturn = '';
                if(firstElement == -1 || lastElement == -1){
                    toReturn = elements.slice(1).find(element => element.trim() != '');
                }else{
                    toReturn = elements.slice(firstElement, lastElement + 1).join(' ');
                    toReturn = toReturn.substring(1, toReturn.length - 1);
                };
                return toReturn;
            })
            .filter(file => file != undefined);
        return content;
    } catch (error) {
        return [];
    }
}
async function createStub(){
    let coreList = (await fsp.readdir(`./cores`, {withFileTypes: true}))
        .filter(file => file.isDirectory())
        .map(file => file.name)
        .filter(file => file != 'config')
        .sort();
    let coreMenuOptions = [];
    for(let i = 0; i < coreList.length; i++){
        coreMenuOptions.push(`${i + 1}. ${coreList[i]}`);
    }

    let selectedCoreRaw = await boxedQuestion(
        () => {
            return [
                ['CREATE OR UPDATE STUBS'],
                Doubleline,
                '',
                'Select the source core:',
                '',
                ...gropuOptions(coreMenuOptions, 10),
            ]
        },
        'Option: ', [], (option) => {
            return (parseInt(option) >= 1 && parseInt(option) <= coreList.length) ||
            coreList.findIndex((core) => core.toLowerCase() == option.toLowerCase()) >= 0;
        }, true
    );

    let selectedCoreOpt = coreList.findIndex((core) => core.toLowerCase() == selectedCoreRaw.toLowerCase());
    if(selectedCoreOpt < 0) {
        selectedCoreOpt = parseInt(selectedCoreRaw) - 1;
    };
    let core = coreList[selectedCoreOpt];

    let destinationOptions = [
        ['1. Famicom (NES)', 'FC'],
        ['2. Super Famicom (SNES)', 'SFC'],
        ['3. Game Boy', 'GB'],
        ['4. Game Boy Color', 'GBC'],
        ['5. Game Boy Advance', 'GBA'],
        ['6. Mega Drive', 'MD'],
        ['7. Arcade', 'ARCADE'],
        ['8. User Roms', 'ROMS'],
    ]
    let selectedCoreIndex = parseInt(
        await boxedQuestion(
            [
                ['CREATE OR UPDATE STUBS'],
                Doubleline,
                'Selected core:',
                '   ' + coreMenuOptions[selectedCoreOpt],
                SingleLine,
                '',
                'Select a destination for the stubs:',
                '',
                ...destinationOptions.map((option) => option[0]),
            ],    
            'Option: ', [], (option) => {
                return parseInt(option) >= 1 && parseInt(option) <= destinationOptions.length
            }
        )
    ) - 1;

    if(selectedCoreIndex == -1) {
        console.log('');
        console.log('');
        console.log('');
        process.exit(0);
    };

    let selectedDestinationopt = destinationOptions[selectedCoreIndex];
    let destination = selectedDestinationopt[1];

    let coreDefaultExtensions = {
        a26: "a26 bin",
        a5200: "a52 bin",
        a78: "a78 bin cdf",
        a800: "xfd atr cdm cas bin atx car com xex m3u",
        amstradb: "dsk sna tap cdt voc m3u cpr",
        cavestory: "exe",
        cdg: "cdg",
        chip8: "ch8 sc8 xo8 hc8",
        col: "col cv bin rom",
        fcf: "bin rom chf",
        flashback: "map aba seq lev",
        gb: "gb dmg gbc cgb sgb",
        gba: "gba bin agb gbz u1",
        gbgb: "gb dmg gbc cgb sgb",
        gg: "sms gg sg mv bin rom",
        gme: "ay gbs gym hes kss nsf nsfe sap spc vgm vgz",
        gong: "game",
        gw: "mgw",
        int: "int rom bin",
        lnx: "lnx o",
        m2k: "zip",
        msx: "rom ri mx1 mx2 col dsk cas sg sc m3u",
        nes: "fds nes unf unif",
        nesq: "nes",
        ngpc: "ngp ngc ngpc npc",
        outrun: "game 88",
        pce: "pce cue ccd chd toc m3u",
        pokem: "min",
        prboom: "wad iwad pwad lmp",
        retro8: "p8 png",
        sega: "bin gen smd md 32x cue iso chd sms gg sg sc m3u 68k sgd pco",
        snes: "smc fig sfc gd3 gd7 dx2 bsx swc",
        snes02: "smc fig sfc gd3 gd7 dx2 bsx swc",
        spec: "tzx tap z80 rzx scl trd dsk",
        thom: "fd sap k7 m7 m5 rom",
        vapor: "vaporbin",
        vec: "bin vec",
        wolf3d: "wl6 n3d sod sdm wl1 pk3 exe",
        wsv: "bin sv",
        xrick: "zip"
    }
    let selectedCoreDefaultExt = coreDefaultExtensions[core] || '';

    let ext = await boxedQuestion(
        [
            ['CREATE OR UPDATE STUBS'],
            Doubleline,
            'Selected core:',
            '   ' + coreMenuOptions[selectedCoreOpt],
            '',
            'Selected destination:',
            '   ' + selectedDestinationopt[0],
            SingleLine,
            '',
            'Type the extensions, separated by spaces:                               '
        ],
        'Extensions: ',
        [],
        (option) => {return option.trim() != '';}, true, selectedCoreDefaultExt
    );
    // if(ext.trim() == '0'){
    //     console.log('');
    //     console.log('');
    //     process.exit(0);
    // }

    ext = ext.split(' ').map(ext => ext.trim().toLowerCase()).filter(ext => ext != '');
    
    let files = [];
    if(fs.existsSync(`./ROMS/${core}`) && (await fsp.stat(`./ROMS/${core}`)).isDirectory()){
        files = (await fsp.readdir(`./ROMS/${core}`, {withFileTypes: true}))
            .filter(file => file.isFile())
            .map(file => file.name)
            .filter(file => ext.includes(path.extname(file).slice(1).toLowerCase()));
    }
    
    let existingStubFiles = (await fsp.readdir(`./${destination}`, {withFileTypes: true}))
        .filter(file => file.isFile())
        .map(file => file.name)
        .filter(file => path.extname(file).slice(1) == 'gBa');

    let existingStubs = [];
    for(let i = 0; i < existingStubFiles.length; i++){
        let content = (await fsp.readFile(`./${destination}/${existingStubFiles[i]}`, 'utf8')).split(';');
        if(content.length != 2) continue;
        if(content[0] != core) continue;
        existingStubs.push(content[1]);
    }

    console.log('\n\n');
    drawLine();
    console.log('');
    
    let created = false;
    let skipped = false;
    
    let cueFiles = [];
    if(fs.existsSync(`./ROMS/${core}`) && (await fsp.stat(`./ROMS/${core}`)).isDirectory()){
        cueFiles = (await fsp.readdir(`./ROMS/${core}`, {withFileTypes: true}))
            .filter(file => file.isFile())
            .map(file => file.name)
            .filter(file => path.extname(file).slice(1).toLowerCase() == 'cue');
    }
    
    let inCue = [];
    for(let i = 0; i < cueFiles.length; i++){
        let content = parseCue(await fsp.readFile(`./ROMS/${core}/${cueFiles[i]}`, 'utf8'));
        content.forEach(file => inCue.push(file));
    }
    files = files.filter(file => !inCue.includes(file));

    for(let i = 0; i < files.length; i++){
        if(existingStubs.includes(files[i])){
            console.log(`Skipping existing stub: ${core};${files[i]}`);
            skipped = true;
            continue;
        }
        console.log(`Creating stub: ${core};${files[i]}`);
        let _fileWoExt = path.basename(files[i], path.extname(files[i]));
        let fileWoExt = _fileWoExt;
        if(fs.existsSync(`./${destination}/${fileWoExt}.gBa`)){
            fileWoExt = `${_fileWoExt}[${core}]`;
        }
        let counter = 2;
        while(fs.existsSync(`./${destination}/${fileWoExt}.gBa`)){
            fileWoExt = `${_fileWoExt}[${core}] (${counter})`;
            counter++;
        }
        await fsp.writeFile(`./${destination}/${fileWoExt}.gBa`, `${core};${files[i]}`);
        created = true;
    }

    if(created == false){
        if(skipped){
            console.log('');
        }
        console.log('No new stubs to create. Press any key to continue.');
    }else{
        console.log('');
        if(destination != 'ROMS'){
            console.log('Stubs generated. YOU MUST RUN TADPOLE TO UPDATE THE ROMS LIST.');
            console.log('Press any key to continue.');
        }else{
            console.log('Stubs generated. Press any key to continue.');
        }
    }
    
    await anyKey();
}
async function deleteStub(){
    let destinationOptions = [
        ['1. Famicom (NES)', 'FC'],
        ['2. Super Famicom (SNES)', 'SFC'],
        ['3. Game Boy', 'GB'],
        ['4. Game Boy Color', 'GBC'],
        ['5. Game Boy Advance', 'GBA'],
        ['6. Mega Drive', 'MD'],
        ['7. Arcade', 'ARCADE'],
        ['8. User Roms', 'ROMS'],
    ]
    let destinationIndex = parseInt(
        await boxedQuestion(
            [
                ['DELETE STUBS THAT POINTS TO INEXISTING ROMS'],
                Doubleline,
                '',
                'Select the stub directory:',
                '',
                ...destinationOptions.map((option) => option[0]),
            ],
            'Option: ', [], (option) => {
                return parseInt(option) >= 1 && parseInt(option) <= destinationOptions.length
            }
        )
    ) - 1;
    let selectedDestinationopt = destinationOptions[destinationIndex];
    if(destinationIndex == -1){
        console.log('');
        console.log('');
        console.log('');
        process.exit(0);
    }
    console.log('\n\n\n');
    drawLine();
    console.log('');
    let destination = selectedDestinationopt[1];
    let existingStubFiles = (await fsp.readdir(`./${destination}`, {withFileTypes: true}))
        .filter(file => file.isFile())
        .map(file => file.name)
        .filter(file => path.extname(file).slice(1) == 'gBa');

    let coreCueds = {};

    let deleted = false;
    for(let i = 0; i < existingStubFiles.length; i++){
        let currentFile = `./${destination}/${existingStubFiles[i]}`;
        let content = (await fsp.readFile(currentFile, 'utf8')).split(';');
        if(content.length != 2){
            console.log(`Deleting corrupted stub: ${currentFile}`);
            await fsp.unlink(currentFile);
            deleted = true;
            continue;
        }
        if(coreCueds[content[0]] == undefined){
            let toAdd = [];
            try {
                let cueFiles = (await fsp.readdir(`./ROMS/${content[0]}`, {withFileTypes: true}))
                    .filter(file => file.isFile())
                    .map(file => file.name)
                    .filter(file => path.extname(file).slice(1).toLowerCase() == 'cue');

                for(let i = 0; i < cueFiles.length; i++){
                    let cueContent = parseCue(await fsp.readFile(`./ROMS/${content[0]}/${cueFiles[i]}`, 'utf8'));
                    cueContent.forEach(file => toAdd.push(file));
                }
            } catch (error) {}
            coreCueds[content[0]] = toAdd;
        }
        // console.log(coreCueds);
        if(coreCueds[content[0]].includes(content[1])){
            console.log(`Deleting stub in cue: ${content[0]};${content[1]}`);
            await fsp.unlink(currentFile);
            deleted = true;
            continue;
        }
        if(!fs.existsSync(`./ROMS/${content[0]}/${content[1]}`)){
            console.log(`Deleting stub: ${content[0]};${content[1]}`);
            await fsp.unlink(currentFile);
            deleted = true;
        }
    }
    
    if(deleted == false){
        console.log('No stubs to delete. Press any key to continue.');
    }else{
        console.log('');
        if(destination != 'ROMS'){
            console.log('Stubs deleted. YOU MUST RUN TADPOLE TO UPDATE THE ROMS LIST.');
            console.log('Press any key to continue.');
        }else{
            console.log('Stubs deleted. Press any key to continue.');
        }
    }
    
    await anyKey();
}

async function firstFunction(){
    try {
        process.stdout.write('\x1b]2;' + 'MULTICORE STUB V2 GENERATOR' + '\x1b\x5c');
    } catch (error) {}
    try {
        if(!fs.existsSync('./bios/bisrv.asd')){
            await boxedQuestion(
                [
                    ['MULTICORE STUB V2 GENERATOR'],
                    Doubleline,
                    '',
                    "It seems that this stub generator is not running from the root of your SD card. ",
                    '',
                    'You must run this stub generator from the root of the sf2000\'s sd card.',
                ],
                'Press any key to exit.',[], (option) => {return true}
            )
            moveCursorBy(-1, 0);
            process.stdout.write(' ');
            console.log('');
            console.log('');
            console.log('');
            process.exit(72);
        }

        let option = ''
        while(option != '0'){
            option = await boxedQuestion(
                [
                    ['MULTICORE STUB V2 GENERATOR'],
                    Doubleline,
                    '',
                    "Select one of the following options:",
                    '',
                    "1. Create or update stubs",
                    "2. Delete stubs that points to inexisting roms",
                    '',
                    "0. Exit",
                ],
                'Option: ',[], (option) => {return option >= '0' && option <= '2'}
            );
            if(option == '1'){
                await createStub();
            }
            if(option == '2'){
                await deleteStub();
            }
            if(option == '0'){
                console.log('');
                console.log('');
                console.log('');
            }
        }
    } catch (error) {
        console.error('');
        console.error(error);
        console.error('');
        console.error('Something went wrong, press any key to exit. ');
        await anyKey();
    }
    reader.close();
}
firstFunction();