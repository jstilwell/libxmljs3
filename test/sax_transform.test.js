const { SaxTransform } = require('../index');
const { Readable } = require('node:stream');

describe('SaxTransform', () => {
  it('emits startDocument and endDocument', (done) => {
    const transform = new SaxTransform();
    const events = [];

    transform.on('startDocument', () => events.push('startDocument'));
    transform.on('endDocument', () => {
      events.push('endDocument');
      expect(events).toEqual(['startDocument', 'endDocument']);
      done();
    });

    transform.end('<?xml version="1.0"?><root/>');
  });

  it('emits startElementNS and endElementNS', (done) => {
    const transform = new SaxTransform();
    const elements = [];

    transform.on('startElementNS', (name) => elements.push(`start:${name}`));
    transform.on('endElementNS', (name) => elements.push(`end:${name}`));
    transform.on('endDocument', () => {
      expect(elements).toEqual([
        'start:root',
        'start:child',
        'end:child',
        'end:root',
      ]);
      done();
    });

    transform.end('<root><child/></root>');
  });

  it('emits characters', (done) => {
    const transform = new SaxTransform();
    let text = '';

    transform.on('characters', (chars) => {
      text += chars;
    });
    transform.on('endDocument', () => {
      expect(text).toBe('hello');
      done();
    });

    transform.end('<root>hello</root>');
  });

  it('works with piped streams', (done) => {
    const transform = new SaxTransform();
    const elements = [];

    transform.on('startElementNS', (name) => elements.push(name));
    transform.on('endDocument', () => {
      expect(elements).toEqual(['root', 'item', 'item']);
      done();
    });

    const source = new Readable({
      read() {
        this.push('<root>');
        this.push('<item/>');
        this.push('<item/>');
        this.push('</root>');
        this.push(null);
      },
    });

    source.pipe(transform);
  });

  it('handles multiple chunks', (done) => {
    const transform = new SaxTransform();
    const events = [];

    transform.on('startElementNS', (name) => events.push(name));
    transform.on('endDocument', () => {
      expect(events).toEqual(['root', 'a', 'b']);
      done();
    });

    transform.write('<root><a/>');
    transform.end('<b/></root>');
  });
});
