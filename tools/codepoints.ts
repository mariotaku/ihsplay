import fs from "fs/promises";
import asyncTransform from "./async-transform";

async function codepointsList(path: string, filter: ((name: string) => boolean)): Promise<Map<string, number>> {
    const lines: string[] = (await fs.readFile(path, {encoding: 'utf-8'})).split('\n');
    return new Map<string, number>(lines.map((line): [string, number] => {
        const split = line.trim().split(' ');
        if (split.length !== 2 || !filter(split[0])) return null;
        return [split[0], Number.parseInt(split[1], 16)];
    }).filter(v => v));
}

async function selectedList(path: string): Promise<string[]> {
    const lines = (await fs.readFile(path, {encoding: 'utf-8'})).split('\n');
    return lines.map(line => line.trim()).filter(v => v);
}

export default function codepoints() {
    return asyncTransform(async file => {
        if (file.extname != '.ttf') {
            return file;
        }
        const cpFile = file.clone();
        cpFile.extname = '.codepoints';
        const lstFile = file.clone();
        lstFile.extname = '.list';
        const list = await selectedList(lstFile.path);
        file.codepoints = await codepointsList(cpFile.path, cp => list.includes(cp));
        file.list = list;
        return file;
    });
}