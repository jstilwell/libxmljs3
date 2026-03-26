var libxml = require('../');

var doc = libxml.Document();
var elem = doc.node('name1');
var firstChild = libxml.Element(doc, 'new-child');
elem.addChild(firstChild);

var _child1 = elem.node('child1');
var child2 = elem.node('child2', 'second');
var secondChild = libxml.Element(doc, 'new-child');
var name2 = elem.node('name2');
name2.addChild(secondChild);
child2.cdata('<h1>cdata test</h1>').cdata('<p>It\'s worked</p>').cdata('<hr/>All done');

console.log('Document with CDATA: %s', doc.toString());
