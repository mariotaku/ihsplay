import through2 from "through2";
import {BufferFile, StreamFile} from "vinyl";
import Fontmin from "fontmin";

type FontminConfigurator = Pick<Fontmin<any>, 'use'>;

export default function fontmin(config: (file: BufferFile, it: FontminConfigurator) => void) {
    return through2.obj((file: BufferFile | StreamFile, _, cb) => {
        if (!file.isBuffer()) {
            cb(new Error('Only buffer file is supported!'));
            return;
        }
        const fm = new Fontmin();
        fm.src((file as BufferFile).contents);
        config(file, fm);
        fm.run((err, files) => {
            if (err) {
                cb(err);
            } else {
                (file as BufferFile).contents = files.map(f => (f as any).contents)[0];
                cb(null, file);
            }
        });
    });
}
