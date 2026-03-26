import { Transform, TransformCallback, TransformOptions } from 'node:stream';
import { SaxPushParser } from './sax_parser';

export interface SaxTransformOptions extends TransformOptions {
  callbacks?: Record<string, (...args: any[]) => void>;
}

/**
 * A Transform stream that wraps SaxPushParser.
 * Write XML chunks to it; SAX events are emitted on the stream.
 *
 * Events: startDocument, endDocument, startElementNS, endElementNS,
 *         characters, cdata, comment, warning, saxError
 */
export class SaxTransform extends Transform {
  private _parser: any;

  constructor(opts?: SaxTransformOptions) {
    super({ ...opts, readableObjectMode: true });
    this._parser = SaxPushParser(opts?.callbacks);

    // Forward SAX events from the push parser to this stream.
    // The SAX 'error' event is re-emitted as 'saxError' to avoid
    // conflicting with Node.js stream's 'error' event semantics.
    const events = [
      'startDocument',
      'endDocument',
      'startElementNS',
      'endElementNS',
      'characters',
      'cdata',
      'comment',
      'warning',
    ];
    for (const event of events) {
      this._parser.on(event, (...args: any[]) => {
        this.emit(event, ...args);
      });
    }
    this._parser.on('error', (...args: any[]) => {
      this.emit('saxError', ...args);
    });
  }

  _transform(
    chunk: Buffer | string,
    _encoding: BufferEncoding,
    callback: TransformCallback
  ): void {
    try {
      const str = typeof chunk === 'string' ? chunk : chunk.toString('utf-8');
      this._parser.push(str, false);
      callback();
    } catch (err) {
      callback(err instanceof Error ? err : new Error(String(err)));
    }
  }

  _flush(callback: TransformCallback): void {
    try {
      this._parser.push('', true);
    } catch {
      // Ignore errors from signaling end-of-document — the document
      // may already be complete from a prior chunk.
    }
    callback();
  }
}
