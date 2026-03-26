const libxml = require('../index');

describe('xml textwriter', () => {
  describe('error handling', () => {
    it('endElement should throw an error if underlying method returned -1', () => {
      const writer = new libxml.TextWriter();

      expect(() => writer.endElement()).toThrow('Failed to end element');
    });
  });
  it('should write an XML preamble to memory', () => {
    let writer;
     
    let _count;
    let output;

    writer = new libxml.TextWriter();
    _count += writer.startDocument();
    _count += writer.endDocument();
    output = writer.outputMemory();
    expect(output).toBe('<?xml version="1.0"?>\n\n');

    writer = new libxml.TextWriter();
    _count += writer.startDocument('1.0');
    _count += writer.endDocument();
    output = writer.outputMemory();
    expect(output).toBe('<?xml version="1.0"?>\n\n');

    writer = new libxml.TextWriter();
    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.endDocument();
    output = writer.outputMemory();
    expect(output).toBe('<?xml version="1.0" encoding="UTF-8"?>\n\n');

    writer = new libxml.TextWriter();
    _count += writer.startDocument('1.0', 'utf8', 'yes');
    _count += writer.endDocument();
    output = writer.outputMemory();
    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n\n'
    );
  });

  describe('standalone handling', () => {
    it('true === yes', () => {
      const writer = new libxml.TextWriter();

      writer.startDocument('1.0', 'utf8', true);
      writer.endDocument();
      expect(writer.outputMemory()).toBe(
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n\n'
      );
    });

    it('false === no', () => {
      const writer = new libxml.TextWriter();

      writer.startDocument('1.0', 'utf8', false);
      writer.endDocument();
      expect(writer.outputMemory()).toBe(
        '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n\n'
      );
    });

    it('default === no', () => {
      const writer = new libxml.TextWriter();

      writer.startDocument('1.0', 'utf8', false);
      writer.endDocument();
      expect(writer.outputMemory()).toBe(
        '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n\n'
      );
    });

    it('falsy === NULL', () => {
      const writer = new libxml.TextWriter();

      writer.startDocument('1.0', 'utf8', 0);
      writer.endDocument();
      expect(writer.outputMemory()).toBe(
        '<?xml version="1.0" encoding="UTF-8"?>\n\n'
      );
    });

    it('missing === NULL', () => {
      const writer = new libxml.TextWriter();

      writer.startDocument('1.0', 'utf8');
      writer.endDocument();
      expect(writer.outputMemory()).toBe(
        '<?xml version="1.0" encoding="UTF-8"?>\n\n'
      );
    });

    it('undefined === NULL', () => {
      const writer = new libxml.TextWriter();

      writer.startDocument('1.0', 'utf8', undefined);
      writer.endDocument();
      expect(writer.outputMemory()).toBe(
        '<?xml version="1.0" encoding="UTF-8"?>\n\n'
      );
    });
  });

  it('should write elements without namespace', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(undefined, 'root');
    _count += writer.startElementNS(undefined, 'child');
    _count += writer.endElement();
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<root><child/></root>\n'
    );
  });

  it('should write elements with default namespace', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(
      undefined,
      'html',
      'http://www.w3.org/1999/xhtml'
    );
    _count += writer.startElementNS(undefined, 'head');
    _count += writer.endElement();
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<html xmlns="http://www.w3.org/1999/xhtml"><head/></html>\n'
    );
  });

  it('should write elements with namespace prefix', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(
      'html',
      'html',
      'http://www.w3.org/1999/xhtml'
    );
    _count += writer.startElementNS('html', 'head');
    _count += writer.endElement();
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<html:html xmlns:html="http://www.w3.org/1999/xhtml"><html:head/></html:html>\n'
    );
  });

  it('should write attributes with default namespace', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(undefined, 'root', 'http://example.com');
    _count += writer.startAttributeNS(undefined, 'attr', 'http://example.com');
    _count += writer.writeString('value');
    _count += writer.endAttribute();
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<root attr="value" xmlns="http://example.com"/>\n'
    );
  });

  it('should write attributes with namespace prefix', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(undefined, 'root');
    _count += writer.startAttributeNS('pfx', 'attr', 'http://example.com');
    _count += writer.writeString('value');
    _count += writer.endAttribute();
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<root pfx:attr="value" xmlns:pfx="http://example.com"/>\n'
    );
  });

  it('should write text node', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(undefined, 'root');
    _count += writer.writeString('some text here');
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<root>some text here</root>\n'
    );
  });

  it('should write cdata section', () => {
    const writer = new libxml.TextWriter();
     
    let _count;

    _count += writer.startDocument('1.0', 'utf8');
    _count += writer.startElementNS(undefined, 'root');
    _count += writer.startCdata();
    _count += writer.writeString('some text here');
    _count += writer.endCdata();
    _count += writer.endElement();
    _count += writer.endDocument();
    const output = writer.outputMemory();

    expect(output).toBe(
      '<?xml version="1.0" encoding="UTF-8"?>\n<root><![CDATA[some text here]]></root>\n'
    );
  });

  it('should return the contents of the output buffer when told so', () => {
    const writer = new libxml.TextWriter();
     
    let _count;
    let output;

    _count += writer.startDocument();
    _count += writer.startElementNS(undefined, 'root');
    output = writer.outputMemory();

    expect(output).toBe('<?xml version="1.0"?>\n<root');

    output = writer.outputMemory();
    expect(output).toBe('');

    _count += writer.endElement();
    _count += writer.endDocument();

    output = writer.outputMemory();
    expect(output).toBe('/>\n');
  });

  it('should not flush the output buffer when told so', () => {
    const writer = new libxml.TextWriter();
     
    let _count;
    let output;

    _count += writer.startDocument();
    _count += writer.startElementNS(undefined, 'root');

    // flush buffer=false, ...
    output = writer.outputMemory(false);
    expect(output).toBe('<?xml version="1.0"?>\n<root');

    // content should be receivable here.
    output = writer.outputMemory(true);
    expect(output).toBe('<?xml version="1.0"?>\n<root');

    // but not here anymore because of recent flush.
    output = writer.outputMemory();
    expect(output).toBe('');
  });
});
