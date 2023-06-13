import {dest, parallel, src} from "gulp";
import binheader from "./binheader";
import codepoints from "./codepoints";
import rename from "gulp-rename";
import subsetFont from "./gulp-subset-font";
import symheader from "./symheader";

const outDir = '../../app/lvgl/fonts/bootstrap-icons';

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
        .pipe(symheader({prefix: 'BS'}))
        .pipe(rename(file => {
            file.basename = 'symbols';
        }))
        .pipe(dest(outDir));
}

exports['iconfonts'] = parallel(iconfont, symlist);