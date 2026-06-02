This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/GSON.svg?color=brightgreen)](https://github.com/GyverLibs/GSON/releases/latest/download/GSON.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GSON.svg)](https://registry.platformio.org/libraries/gyverlibs/GSON)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GSON?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GSON
Parser and data collector in JSON format for Arduino
- 6 times faster and much lighter than ArduinoJSON
- JSON parsing with error handling
- Linear Assembly of JSON Package
- Screening of "dangerous" symbols
- It's based on Text.[StringUtils](https://github.com/GyverLibs/StringUtils))
- Works with 64 bit numbers
- Built-in key hashing mechanism
- The library is not suitable for storing and modifying data. Only parsing and packet assembly *

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)

### Dependencies
- [StringUtils](https://github.com/GyverLibs/StringUtils)
- [GTL](https://github.com/GyverLibs/GTL) v1.0.7+

## Contents
- [Documentation.](#docs)
- [Use of use](#usage)
- [Versions](#versions)
- [Installation](#install)
- [Bugs and feedback](#feedback)

<a id="docs"></a>

## Documentation.
### `Text`
Under type.`Text`A line is understood in any format:
- `"const char"`- lines
- `char[]`- lines
- `F("f-строки")`
- `String`- lines

### `gson::Parser`
```cpp
// designer
gson::Parser;

// method
bool reserve(uint16_t cap);     // reserve memory to speed up parsing
uint16_t length();              // number
uint16_t size();                // Get the document size in RAM (byte)
void hashKeys();                // hash the keys of all elements (the operation is irreversible)
bool hashed();                  // check if the keys have been hashed
void reset();                   // memory
uint16_t rootLength();          // Get the number of items in the main container

// set the maximum depth of parsing nesting (failure 16)
void setMaxDepth(uint8_t maxdepth);

Entry get(Text key);            // Key access (main container - Object)
Entry get(size_t hash);         // access by key hash (main container - Object)
Entry get(int index);           // index access (main container - Array or Object) (rootLength())

Entry getByIndex(parent_t index);   // obtain an index element in the total parser array (length())

Text key(int idx);              // index
size_t keyHash(int idx);        // index
Text value(int idx);            // index
int8_t parent(int idx);         // index
Type type(int idx);             // index

const __FlashStringHelper* readType(uint16_t idx);  // index

// scratch. Return true with successful parsing
bool parse(const Text& json);
bool parse(const char* json, uint16_t len);

// output to Print with formatting n
void stringify(Print& p);

// strip
template <typename T>
bool parseTo(T& arr);

// error-handling
bool hasError();                        // there's a parsing error
Error getError();                       // mistake
uint16_t errorIndex();                  // line-code
const __FlashStringHelper* readError(); // mistake
```

ParserStream - parser from Stream Stream
```cpp
gson::ParserStream;

// Read from the stream and save yourself
bool parse(Stream* stream, size_t length);

// Read from the line and save yourself
bool parse(const char* str, size_t length);

// memory
void reset();

// Get a downloaded json package as Text
Text getRaw();

void move(ParserStream& ps);
```

#### Settings
Announced before connecting the library
```cpp
// Increase storage limits (described below)
#define GSON_NO_LIMITS
```

#### Limits and memory
- After parsing, one element weighs 8 bytes.
- Maximum number of elements: 255 on AVR and 512 on ESP
- Maximum length of json lines: 65,536 characters
- Maximum key length: 31 characters
- Maximum length of value: 32,767 characters
- The maximum nesting level of the elements 16 is configured. Parsing is recursive, each level adds several bytes to the RAM

When announcing`#define GSON_NO_LIMITS`Before connecting the library, the limits are increased:
- One element weighs 12 bytes.
- Maximum number of elements: 65,355 per ESP
- Maximum key length: 255 characters
- Maximum length of value: 65,355 characters

### Tests
Tested on ESP8266, a packet of messages from bot telegrams - 3600 characters, 147 "elements". Received the value of the most distant and nested element, in GSON - through a hash. Result:

Liba | Flash | SRAM | FreeHeap | Parse | Get |
| ------------ | ------- | ------- | --------- | ------- | ------ | |
| ArduinoJson | 297433 | 31628 | 41104 | 10686 us | 299 us |
| GSON | 279349 | 31400 | 43224 | 1744 us | 296 us |

So GSON **6 times faster** when parsing, the values of the elements get at the same rate. The library itself is lighter by 18kB in Flash and 2.1kB in Heap. In other tests (AVR) to obtain the value of GSON hash worked on average 1.5 times faster.

### `gson::Type`
```cpp
None
Object
Array
String
Int
Float
Bool
Null
```

### `gson::Error`
```cpp
None
Alloc
TooDeep
NoParent
NotContainer
UnexComma
UnexColon
UnexToken
UnexQuotes
UnexOpen
UnexClose
UnknownToken
BrokenToken
BrokenString
BrokenContainer
EmptyKey
IndexOverflow
LongPacket
LongKey
EmptyString
```

### `gson::Entry`
Also inherits everything from`Text`, documentation[here](https://github.com/GyverLibs/StringUtils?tab=readme-ov-file#anytext)

```cpp
Entry get(Text key);        // key
bool has(Text key);         // contains an element with the specified key

Entry get(size_t hash);     // hash
bool has(size_t hash);      // contains an element with the specified key hash

Entry get(int index);       // index

bool valid();               // verification of correctness (existence)
uint16_t length();          // Get the size (for objects and arrays). For the others 0)
Text key();                 // key
size_t keyHash();           // hash
Text value();               // value
void stringify(Print& p);   // output to Print with formatting n

Type type();                // type
bool is(gson::Type type);   // compare
bool isContainer();         // Array or Object
bool isObject();            // Object
bool isArray();             // Array
parent_t index();           // element index

// strip
template <typename T>
bool parseTo(T& arr);
```

### `gson::Str`
A new tool in return`gson::string`- 2 times faster and easier.

```cpp
// buffering
char* buf();

// cleanse
void clear();

// Add data of any type directly
void concat(T val);

// length
uint16_t length();

// reserve
bool reserve(uint16_t len);

// Add any data as JSON value
// (whole, float, char, nullptr, any strings)
operator=(T val);
operator+=(T val);

// float
void add(float val, uint8_t dec = 2);

// add a line with escape
void escape(const Text& txt);

// object key, string of any type
Str& operator[](T str);

// The container will always return true. Type {, }, [, ]
Str& operator()(char type);
```

<a id="usage"></a>

## Use of use
### parsing
The library does not duplicate a line in memory and works with the original line: it remembers the positions of the text, the source line does not change. It follows that:
- The string must exist in memory throughout the entire json document.
- If the reference line -`String`It should not be changed by the program until the end of the work with the document.
- `ParserStream`"downloads" a line from stream to memory

Creation of the document:
```cpp
gson::Parser p;
```

```cpp
// got json
char json[] = R"raw({"key":"value","int":12345,"obj":{"float":3.14,"bool":false},"arr":["hello",true]})raw";
String json = "{\"key\":\"value\",\"int\":12345,\"obj\":{\"float\":3.14,\"bool\":false},\"arr\":[\"hello\",true]};";

// scrape
p.parse(json);

// error-handling
if (p.hasError()) {
    Serial.print(p.readError());
    Serial.print(" in ");
    Serial.println(p.errorIndex());
} else Serial.println("done");
```

After parsing, you can output the entire package with types, keys, values in the form of text and the parent:
```cpp
for (uint16_t i = 0; i < p.length(); i++) {
    // if (p.type(i) == gson:::Type:::Object| | p.type(i) == gson:::Type:::Array) continue; // skip containers
    Serial.print(i);
    Serial.print(". [");
    Serial.print(p.readType(i));
    Serial.print("] ");
    Serial.print(p.key(i));
    Serial.print(":");
    Serial.print(p.value(i));
    Serial.print(" {");
    Serial.print(p.parent(i));
    Serial.println("}");
}
```

Values can be obtained in type`Text`, which can be converted to other types and output to the port:
- The key can be a string in any form (`"строка"`, `F("строка")`)
- You can refer to nested objects by key, and to arrays by index
- Documentation for`Text`look[here](https://github.com/GyverLibs/StringUtils)

```cpp
Serial.println(p["key"]);      // value-print
Serial.println(p[F("int")]);   // print 12345
int val = p["int"].toInt16();  // conversion
val = p["int"];                // converter
float f = p["obj"]["float"];   // nested
bool b = p["flag"].toBool();   // bool
Serial.println(p["arr"][0]);   // hello
Serial.println(p["arr"][1]);   // true

char buf[10];
p["key"].toStr(buf, 10);    // discharge

// test
p["arr"].type() == gson::Type::Array;

// array
for (int i = 0; i < p["arr"].length(); i++) {
    Serial.println(p["arr"][i]);
}

// Or better yet.
gson::Entry arr = p["arr"];
for (int i = 0; i < arr.length(); i++) {
    Serial.println(arr[i]);
}

// Let json have the form [[123,456], ["abc", "def"], then the nested arrays can be addressed:
Serial.println(p[0][0]);  // 123
Serial.println(p[0][1]);  // 456
Serial.println(p[1][0]);  // abc
Serial.println(p[1][1]);  // def
```

### bool
`Text`It is automatically converted to all types except`bool`Use it.`toBool()`. The conversion to bool shows the existence of an element, which can be used instead.`has()`:

```cpp
if (p["foo"]) {}            // if
if (p["foo"].toBool()) {}   // if there is a true
```

Comparison with`bool`If an element does not exist, the result will always be`false`:

```cpp
if (p["foo"] == true) {}    // if there is a true
if (p["foo"] == false) {}   // If there is a false
```

### Withdrawal to Entry
Each element can be typed`gson::Entry`by name (from the object) or index (from the array) and use separately, so as not to "seek" it again:
```cpp
gson::Entry e = p["arr"];
Serial.println(e.length());  // massif
Serial.println(e[0]);        // hello
Serial.println(e[1]);        // true
```

### Hashing
GSON natively supports hash strings from StringUtils, working with hashes significantly increases the speed of access to JSON elements of the document by key. Line transferred to function`SH`** does not exist in the program and does not occupy space: the hash is considered a compiler at the compilation stage, a number is substituted instead. Comparison of numbers is faster than comparison of lines. This requires:

1. Hash the keys after parsing:
```cpp
p.parse(json);
p.hashKeys();
```

> Note: hashing does not cancel line access, as was the case in the first versions of the library! It can be used as`p["key"]`well`p[su::SH("key")]`

2. Address elements by key hashes using the function`su::SH`:
```cpp
using su::SH;

void foo() {
    Serial.println(p[SH("int")]);
    Serial.println(p[SH("obj")][SH("float")]);
    Serial.println(p[SH("array")][0]);
}
```

> Note: overload is used to access the hash`[size_t]`, and to access the array element -`[int]`. Therefore, for correct access to the elements of the array, it is necessary to use`signed int`rather than unsigned (`uint8`and`uint16`) Otherwise, the compiler can cause access by hash instead of accessing the array.

```cpp
gson::Entry arr = p["arr"];
for (int i = 0; i < arr.length(); i++) {    // Int counter!
    Serial.println(arr[i]);
}
```

> Hashing creates an array of size in memory`колво_элементов * 4`

### Assembly
JSON string is collected **linear**, which is very simple and pleasant for MK:

```cpp
gson::Str j;

j('{'); // launch

if (j["obj"]('{')) {    // key object. if for visual separation of the container
    j["int"] = 123;
    j["float"] = 3.1415;
    j["float2"].add(3.1415, 4);
    j["null"] = nullptr;
    j["cstr"] = "cstr";
    j[F("fstr")] = F("fstr");
    j[String("str")] = String("str");
    j["esc"].escape(R"("he	ll\o world")");

    j('}');
}
if (j["arr"]('[')) {
    j += true;
    j += NAN;
    j += INFINITY;
    j += nullptr;

    if (j('[')) {
        j += 12345;
        j += 1234567L;
        j += 123456787654LL;
        j(']');
    }

    j(']');
}
j('}');

Serial.println(j);
```

```json
{"obj":{"int":123,"float":3.14,"float2":3.1415,"null":null,"cstr":"cstr","fstr":"fstr","str":"str","esc":"\"he\tll\\o world\""},"arr":[true,null,null,null,[12345,1234567,123456787654]]}
```

<a id="versions"></a>

## Versions
- v1.0
- v1.1 - improved parser, added key hashing and hashing
- v1.2 - Optimization for StringUtils 1. 3
- v1.3 - parser optimization, speed up reading values from Parser
- v1.4 - parser optimization, read acceleration, original line no longer changes with parser
- v1.4.1 is support for the kernel esp8266 v2. xx
- v1.4.2 - Raw methods added to string
- v1.4.3 - Upgrade to current StringUtils, parsing from Text
- v1.4.6 adds stringify to Entry
- v1.4.9 - Add more addText features to gson:string
- v1.5.0 
  - Accelerate parsing
  - Reduced disparsed weight in the RAM
  - Move semantics for parser transfer between objects as uniq object
  - Added parser from Stream
  - Abolished Static Parser
  - Small improvements
- v1.5.1 - added picker json (bson)
- v1.5.2 - BSON collector improved, JS example corrected
- v1.5.7 - Fixed critical bug with empty string parsing
- v1.5.9 – BSON adds binary data support. Incompatible with the previous version!

<a id="install"></a>

## Installation
- The library can be found by the name **GSON** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/GSON/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="feedback"></a>
## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.
