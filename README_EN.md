This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gson
Parser and data collector in json format for Arduino
- 7 times faster and much lighter than Arduinojson
- JSON Parsing with error processing
- Linear assembly of a json package
- shielding "dangerous" symbols
- works on the basis of Anytext (Stringutils)
- works with 64 bit numbers
- *The library is not suitable for storing data!Only parsing and assembly from scratch*

## compatibility
Compatible with all arduino platforms (used arduino functions)

### Dependencies
- Library [Stringutils] (https://github.com/gyverlibs/stringutils)

## Content
- [documentation] (#docs)
- [use] (#usage)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id="docs"> </a>

## Documentation
### `gson :: doc`
`` `CPP
// Designer
Gson :: doc ();
GSON :: doc (size);
GSON :: doCstatic <size> ();

// Methods
uint16_t Length ();// get the number of elements
uint16_t size ();// get the size of the document in RAM (byte)
Void Hashkeys ();// Heshcht the keys of all elements (the operation is irreversible)
Bool Hashed ();// Checking whether the keys were drunken

Entry Get (Anytext Key);// Key access (main container - Object)
Entry Get (Size_t Hash);// Access to Hash Key (main container - Object)
Entry Get (Int Index);// Access by index (main container - Array or Object)

Anytext Key (Intx);// Read the key on the index
size_t keyhash (int IDX);// Read the Hesh key on the index
Anytext Value (Intx);// Read the value by index
int8_t part (int itx);// Read the parent on the index
Type Type (int IDX);// get the type of index

const __flashstringhelper* readtype (Uint16_T IDX);// Read the type of index

// Parish.True will return with successful parsing.You can specify Max.The level of investment
Bool Parse (String & Json, Uint8_t Maxdepth = 16);
Bool Parse (Const Char* Json, Uint8_t Maxdepth = 16);

// Bring out to Print with formatting
VOID stringify (print& p);

// error processing
Bool Haserror ();// there is a parsing error
Error Geterror ();// get an error
uint16_t errorindex ();// Index of the place of error in the line
const __flashstringhelper* readerror ();// Read the error
`` `

### limits
- After parsing, one element weighs 6 bytes on AVR and 12 bytes on ESP
- the maximum number of elements (meter restriction) - 255 on AVR and 65535 on ESP
- The maximum level of investment is set in the function of `PARSE ()`.Parsing is recursive, each level adds several bytes to the RAM

### tests
He tested on ESP8266, a package of messages from the bottle - 3500 characters, 8 messages, 180 "elements".He received the value of the most distant and invested element, in the gson - through the hash.Result:

|Libe |Flash |Sram |Freeheap |Parse |Hash |Get |
| ------------ | -------- | ------ | --------- | ------------| -------- | ------- |
|Arduinojson |285525 |28256 |45648 |9900 US |- |158 US |
|Gson |275193 |28076 |46664 |1400 US |324 US |156 US |

Thus, Gson ** is 7 times faster ** during parsing, the values of the elements are obtained withis the same speed.The library itself is easier to 10 kb in Flash and 1 KB in Heap.In other tests (AVR) to obtain the GSON value with a hasa, it worked on average 1.5 times faster.

### `GSON :: Type`
`` `CPP
None
Object
Array
String
Int
Float
Bool
`` `

### `gson :: error`
`` `CPP
None
Alloc
Toodeep
NopARENT
NotContainer
UNEXCOMMA
Unexcolon
UNEXTOKEN
Unexquotes
Unexopen
Unexclose
Unknowntoken
Brokentoken
Brokenstring
BrokenContainer
Emptyky
Indexoverflow
`` `

### `GSON :: Entry`
Also inherits everything from `Anytext`, documentation [here] (https://github.com/gyverlibs/stringutils?tab=Readme-ov-file#anytext)

`` `CPP
Entry Get (Anytext Key);// get an element by key
Bool Includes (Anytext Key);// contains an element with the specified key

Entry Get (Size_t Hash);// Get the element according to the Key Hash
Bool Includes (Size_t Hash);// contains an element with the indicated Key Hash

Entry Get (Int Index);// Get an index element

Bool Valid ();// Checking correctness (existence)
uint16_t Length ();// Get the size (for objects and arrays. For the rest 0)
Type type ();// get the type of element
Anytext Key ();// Get the key
size_t keyhash ();// Get a keys of the key
Anytext Value ();// get a value
`` `

### `GSON :: String`
`` `CPP
String S;// Access to the line
Void Clear ();// Clean the line
Bool Reserve (Uint16_T Res);// Reserve a line

// Add Gson :: String.A comma will be added
String & Add (Constation String & STR);

// Add the key (string of any type)
String & Addkey (Anytext Key);

// add text (string of any type)
String & Addraw (Anytext Str, Bool ESC = FALSE);

// Add a line (a string of any type)
String & Addstr (Anytext Key, Anytext Value);
String & Addstr (Anytext Value);

// Add bool
String & Addbool (Anytext Key, Const Bool & Value);
String & Addbool (Const Bool & Value);

// Add Float
String & Addfloat (Anytext Key, const Double & Value, Uint8_t Dec = 2);
String & Addfloat (Constance Double & Value, Uint8_t Dec = 2);

// Add int
String & Addint (Anytext Key, Const ANYVALUE & VALUE);
String & Addint (Const ANYVALUE & VALUE);

String & Beginobj (Anytext Key = "");// Start an object
String & Endobj ();// Complete the object

String & Beginarr (Anytext Key = "");// Start an array
String & Endarr ();// End the array

String & End ();// complete the package
`` `

<a id="usage"> </a>

## Usage
### Parsing
Peculiarities:
- The library ** does not duplicate the json line ** in memory, but works with the original line, preserving the signs on the values
- Parsing ** Changes the initial line **: some characters are replaced by `'\ 0'`

It follows that:
- the line must exist in memory throughout work with JSON document
- If the initial line - `string` - it should categorically not change the program until the end of work with the document

Creating a document:
`` `CPP
Gson :: doc doc;// Dynamic document
Gson :: doc doc (10);// Dynamic document reserved for 10 elements
GSON :: doCstatic <10> doc;// Static document reserved for 10 elements
`` `

The meaning is like `String`-string: the dynamic document will increase during parsing in the dynamic memory of MK, if the size of the document is unknown.You can reserve the size in advance so that Parsing occurs faster.Static - stands out statically using less memory on weak platforms.

`` `CPP
// received json
Char JSON [] = R "BIN (" Key ": Value," Int ": 12345," Obj ": {" Float ": 3.14," Bool ": FALSE}," arr ":" Hello ", true]}) BIN ";
String json = "\" key \ ": \" value \ ", \" int \ ": 12345, \" Obj \ ": {\" float \ ": 3.14, \" bool \ ": false}, \ \"Arr \": [\ "Hello \", true]}; ";

// Parish
doc.parse (json);

// error processing
if (doc.Haserror ()) {
    Serial.print (doc.earaderror ());
    Serial.print ("in");
    Serial.println (doc.errorindex ());
} Else serial.println ("Done");
`` `

After parsing, you can derive the entire package with types, keys, values in the form of text and parent:
```CPP
for (uint16_t i = 0; i <doC.length (); i ++) {
    // if (doc.type (i) == gson :: type :: Object || doc.type (i) == gson :: type :: array) Continue;// Skip containers
    Serial.print (i);
    Serial.print (".. [");
    Serial.print (doc.readtype (i));
    Serial.print ("]");
    Serial.print (doc.key (i));
    Serial.print (":");
    Serial.print (doc.value (i));
    Serial.print ("{");
    Serial.print (doc.part (i));
    Serial.println ("}");
}
`` `

> The weight of one "element" of any type (line, number, object, array) is 6 bytes on AVR and 10 bytes on ESP.Thus, after parsing, 9 elements (54 bytes) will additionally appear in the memory.

The values can be obtained in the type of `Anytext`, which can convert to other types and displayed into the port:
- the key can be a line in any form (`" line "`, `f (" line ")`)
- you can contact the nested objects on the key, and to the arrays on the index

`` `CPP
Serial.println (doc ["key"]);// Value
Serial.println (doc [f ("int")]);// 12345
int Val = doc ["int"]. toint16 ();// Conversion into the specified type
val = doc ["int"];// Auto Convertation
Float F = doc ["Obj"] ["Float"];// invested object
Serial.println (doc ["arr"] [0]);// Hello
Serial.println (doc ["arr"] [1]);// True

// Type check
doc ["arr"]. Type () == gson :: type :: array;

// Conclusion of the contents of the array
for (int i = 0; i <doc ["arr"]. Length (); i ++) {
    Serial.println (doc ["arr"] [i]);
}

// and better - so
Gson :: Entry Arr = doc ["arr"];
for (int i = 0; i <arr.length (); i ++) {
    Serial.println (arr [i]);
}

// Let json have the form [[123,456], ["ABC", "Def"]], then you can turn to the nested arrays:
Serial.println (doc [0] [0]);// 123
Serial.println (doc [0] [1]);// 456
Serial.println (doc [1] [0]);// ABC
Serial.println (doc [1] [1]);// Def
`` `

Each element can be brought to the type `gson :: Entry` by name (from the object) or index (from array) and use separately so as not to" look "it again:
`` `CPP
GSON :: Entry E = doc ["arr"];
Serial.println (E.LENGTH ());// The length of the array
Serial.println (E [0]);// Hello
Serial.println (E [1]);// True
`` `

### Hash
GSON is natively supported by Stringutils hash strokes, work with hashi significantly increases the speed of access to the JSON elements of the key document.For this you need:

1. Hashcht the keys.** This operation is irreversible **: The keys in the text form can no longer read from the document:
`` `CPP
doc.hashkeys ();
`` `

2. Turn to the elements according to the Hesh keys using the function `su :: Sh`:
`` `CPP
USing Sutil :: SH;

VOID FOO () {
    Serial.println (doc [SH ("int")]);
    Serial.println (doc [SH ("Obj")] [SH ("Float")]);
    Serial.println (doc [SH ("Array")] [0]);
}
`` `

The line transferred to `SH`, in general ** does not exist in the ** program and does not take up place: the hash is considered a compiler at the compilation stage, the number is substituted instead.And the comparison of numbers is performed faster than the comparison of the lines.

Notes:
- function `keyhash ()` will return:
    - Previously counted hash, if it was caused by `hashkeys ()`
    - if not caused, he will count and return the hash
- The function `key ()` will return:
    - a valid line if the hash `hashkeys ()` `was not calculated
    - empty line if it was calculated

3. For calculating and obtaining a hash, you can simply apply `hash ()` from `ANYTEXT`
`` `CPP
switch (doc [SH ("str")]. Hash ()) {
    Case SH ("SOME TEXT"):
    // ...
}
`` `

## assembly
Json line is going to ** linear ** in the usual `string`-string, which is very simple and pleasant for a microcontroller:
`` `CPP
Gson :: String GS;// Create a line
gs.beginobj ();// Start object 1
GS.Addstr ("str1", f ("value"));// Add a string value
GS ["str2"] = "value2";// So you can also
GS ["int"] = 12345;// integer
GS.Beginobj ("Obj");// invested object 2
GS.Addfloat (Float), 3.14);// Float
GS ["float2"] = 3.14;// or so
GS ["Bool"] = false;// Bool value
gs.ndobj ();// Complete object 2

GS.Beginarr ("Array");// Start an array
GS.Addfloat (3.14);// to array - without key
GS += "Text";
GS += 12345;
GS += True;
gs.ndarr ();// End the array

gs.ndobj ();// Complete object 1

gs.end ();// complete the package (necessarily called at the end)

Serial.println (GS);// Conclusion to the port
`` `

<a id="versions"> </a>

## versions
- V1.0
- V1.1 - Improved Parser, added keys and harsh codes.

<a id="install"> </a>

## Installation
- The library can be found by the name ** Gson ** and installed through the library manager in:
    - Arduino ide
    - Arduino ide v2
    - Platformio
- [download the library] (https://github.com/gyverlibs/gson/archive/refs/heads/main.zip). Zip archive for manual installation:
    - unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
    - unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
    - unpack and put in *documents/arduino/libraries/ *
    - (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%BD%D0%BE%BE%BE%BED0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!

<a id="feedback"> </a>
## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!

When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a canvas of a thousand lines, but a minimum code