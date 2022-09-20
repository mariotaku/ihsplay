import {dest, parallel, src} from "gulp";
import binheader from "./binheader";
import Fontmin from "fontmin";
import codepoints from "./codepoints";
import fontmin from "./gulp-fontmin";
import asyncTransform from "./async-transform";
import rename from "gulp-rename";

const outDir = '../app/lvgl/fonts/material-icons'

async function iconfont() {
    return src('res/MaterialIcons-Regular.ttf')
        .pipe(codepoints())
        .pipe(fontmin((file, it) => {
            const codepoints: Map<string, number> = file.codepoints;
            it.use(Fontmin.glyph({text: String.fromCodePoint(...codepoints.values())}));
        }))
        .pipe(binheader({naming: 'snake_case', prefix: 'ttf'}))
        .pipe(rename(file => {
            file.basename = 'font';
        }))
        .pipe(dest(outDir));
}

async function symlist() {
    return src('res/MaterialIcons-Regular.ttf')
        .pipe(codepoints())
        .pipe(asyncTransform(async file => {
            const codepoints: Map<string, number> = file.codepoints;
            let content = '#pragma once\n\n';
            const encoder = new TextEncoder();
            codepoints.forEach((cp, key) => {
                const value = Array.from(encoder.encode(String.fromCodePoint(cp)))
                    .map(v => `\\x${v.toString(16)}`).join('');
                content += `#define MAT_SYMBOL_${key.toUpperCase()} "${value}"\n`;
            });
            file.contents = Buffer.from(content);
            file.basename = 'symbols.h';
        }))
        .pipe(dest(outDir));
}

exports['iconfonts'] = parallel(iconfont, symlist);