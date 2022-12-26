import {dest, parallel, src} from "gulp";
import binheader from "./binheader";
import codepoints from "./codepoints";
import asyncTransform from "./async-transform";
import rename from "gulp-rename";
import subsetFont from "./gulp-subset-font";

const outDir = '../app/lvgl/fonts/bootstrap-icons';

function codepointsMetadata(file: any): Record<string, number> {
    return file.codepoints;
}

async function iconfont() {
    return src('res/bootstrap-icons.woff2')
        .pipe(codepoints())
        .pipe(subsetFont(file => String.fromCodePoint(...Object.values(codepointsMetadata(file)))))
        .pipe(binheader({naming: 'snake_case', prefix: 'ttf'}))
        .pipe(rename(file => {
            file.basename = 'regular';
        }))
        .pipe(dest(outDir));
}

async function symlist() {
    return src('res/bootstrap-icons.woff2')
        .pipe(codepoints())
        .pipe(asyncTransform(async file => {
            const codepoints: Record<string, number> = codepointsMetadata(file);
            let content = '#pragma once\n\n';
            const encoder = new TextEncoder();
            for (let key in codepoints) {
                const cp: number = codepoints[key];
                const value = Array.from(encoder.encode(String.fromCodePoint(cp)))
                    .map(v => `\\x${v.toString(16)}`).join('');
                const name = key.toUpperCase().replace(/[^0-9a-z_]/ig, '_');
                content += `#define BS_SYMBOL_${name} "${value}"\n`;
            }
            file.contents = Buffer.from(content);
            file.basename = 'symbols.h';
        }))
        .pipe(dest(outDir));
}

exports['iconfonts'] = parallel(iconfont, symlist);