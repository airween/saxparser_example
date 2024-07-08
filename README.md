# LibXML2 SAX parser test

Welcome to the `saxparsertest` documentation.

## Installation

The parser relies on *libxml2* as its underlying XML parsing library.

Therefore, to compile and run it you will need:

* a **C** interpreter
* **autoconf** - GNU Autoconf
* **libxml2** development and runtime library

### Debian install

You can install these packages on Debian with this command:

```bash
sudo apt install gcc autoconf libxml2 libxml2-dev libxml2-utils
```

Run `autogen.sh` - this will generate `configure` script, then run `./configure`. If your **libxml2** path is not the default, pass it with `--with-xml-prefix` option, eg:

```bash
./configure --with-xml-prefix=/usr/local
```

After that, you can type `make` - this will compile the binary tool under the `src/` directory.

## Usage

Try to parse an XML file:

```bash
src/saxparser data/test1.xml 
key: 'root.a', val: '1'
key: 'root.b.element.a1', val: 'a1val'
key: 'root.b.element.a1', val: 'a2val'
```

The tool will parse the XML file, concatenates the node names into a key, and print the key:value pairs. The original XML content above is this:

```bash
cat data/test1.xml 
<?xml version="1.0" encoding="UTF-8"?>
<root>
    <a>1</a>
    <b>
        <element>
            <a1>a1val</a1>
        </element>
        <element>
            <a1>a2val</a1>
        </element>
    </b>
</root>
```

You can see that the first node which has a text value is `root.a` because the XML path is `root/a`, the second is `root.b.element.a1` because the path is `root/b/element/a1`.

### Options

You can get help with `-h`:

```bash
OPTIONS:
	-h	This help
	-H	Hide key:value pairs, only print time
	-T	Hide time results, only print key:value pairs
	-d	Set the max depth of XML tree; when the parser exceeds that, terminate
	-n	Set the max number of nodes; when the parser exceeds that, terminate
```

* `-H` - hide the key:value pairs during process; can be usefull if the XML contains many nodes
* `-T` - hide the time results; `saxparser` prints out of total processing time at the end, if you don't need that, use this option
* `-d` - set the maximum depth until the parser processes the file; when it reaches this value, terminates
* `-n` - set the maximum number of nodes; when it reaches this value, terminates

## Know issues

The max file size is fixed in code (10MB), see:
```C
#define BUFFLEN 1024*1024*10
```
If the maximum values (depth, number of nodes - pass them by option, see [Options](#options)) reach the value, allocated pointers are not freed.
