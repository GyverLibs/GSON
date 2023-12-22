This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gson
Parser and data collector in json format for Arduino
- Twice faster and much lighter than Arduinojson
- JSON Parsing with error processing
- Linear assembly of a json package
- shielding "dangerous" symbols
- works on the basis of Anytext (Stringutils)

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
uint16_t Length ();// Get the number of Entry records
uint16_t size ();// Get DOC size in RAM (byte)
Entry Get (Constext & Key);// Key access (main container - Object)
Entry Get (Int Index);// Access by index (main container - Array or Object)
Anytext Key (Uint16_T IDX);// Read the key on the index
Anytext Value (Uint16_T IDX);// Read the value by index
Gson :: Type Type ();// get the type of index
const __flashstringhelper* readtype (Uint16_T IDX);// Read the type of index

// Parish.Will return True with successful parsing
Bool Parse (String & Json);
Bool Parse (Const Char* Json);

TEMPLATE <UINT16_T MAX_DEPTH = 16>
Bool Parset (String & Json);
TEMPLATE <UINT16_T MAX_DEPTH = 16>
Bool Parset (Const Char* Json);

// Bring out to Print with formatting
VOID stringify (print* p);

// error processing
Bool Haserror ();// there is a parsing error
Error Geterror ();// get an error
uint16_t errorindex ();// Index of the place of error in the line
const __flashstringhelper* readerror ();// Read the error
`` `

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
`` `

### `GSON :: Entry`
Also inherits everything from `Anytext`, documentation [here] (https://github.com/gyverlibs/stringutils?tab=Readme-ov-file#anytext)

`` `CPP
Entry Get (Constext & Key);// get an element by key
Entry Get (Int Index);// Get an index element
Bool Valid ();// Checking correctness (existence)
Anytext Key ();// Get the key
Anytext Value ();// get a value
uint16_t Length ();// Get the size (for objects and arrays. For the rest 0)
Type type ();// get a type
`` `

### `GSON :: String`
`` `CPP
String S;// Access to the line
Void Clear ();// Clean the line
Bool Reserve (Uint16_T Res);// Reserve a line

// Add Gson :: String.A comma will be added
String & Add (Constation String & STR);

// Add the key (string of any type)
String & Addkey (Constext & Key);

// add text (string of any type)
String & Addraw (Constext & Str, Bool ESC = FALSE);

// Add a line (a string of any type)
String & aDDSTR (COST ANYTEXT & KEY, COST ANYTEXT & VALUE);
String & Addstr (Constext & Value);

// Add bool
String & Addbool (Constext & Key, Const Bool & Value);
String & Addbool (Const Bool & Value);

// Add Float
String & Addfloat (Constext & Key, const Double & Value, Uint8_t Dec = 2);
String & Addfloat (Constance Double & Value, Uint8_t Dec = 2);

// Add int
String & Addint (Constext & Key, const ANYVALUE & VALUE);
String & Addint (Const ANYVALUE & VALUE);

String & Beginobj (Constext & Key = "");// Start an object
String & Endobj ();// Complete the object

String & Beginarr (Constext & Key = "");// Start an array
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
Char JSON [] = R "RAW (" Key ": Value," Int ": 12345," Obj ": {" Float ": 3.14," Bool ": FALSE}," arr ":" Hello ", true]}) RAW ";
String json = "\" key \ ": \" value \ ", \" int \ ": 12345, \" Obj \ ": {\" float \ ": 3.14, \" bool \ ": false}, \ \"Arr \": [\ "Hello \", true]}; ";

// Parish
doc.parse (json);
`` `

Parser has ** fixed ** level of investment of objects and arrays, when it is overflowing, there will be a parsing error `toodeep`.By default, the value is 16 (*one level of investment occupies one byte of memory*).You can change it using the template function of `PARSET <LARS>`.

`` `CPP
// Bring the entire package with types, keys, values in the form of text and parent
for (int i = 0; i <doc.entries.length (); i ++) {
    // if (doc.entries [i] .type == gson :: type :: Object || doc.entries [i] .type == gson :: Type :: Array) Continue;// Skip containers
    Serial.print (i);
    Serial.print (".. [");
    Serial.print (doc.readtype (i));
    Serial.print ("]");
    Serial.print (doc.entries [i] .key);
    Serial.print (":");
    Serial.print (doc.entries [i] .value);
    Serial.print ("{");
    Serial.print (doc.entries [i] .part);
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

// Let json have the form [[123,456], ["ABC", "Def"]], then you can turn to the nested arrays:
Serial.println (doc [0] [0]);// 123
Serial.println (doc [0] [1]);// 456
Serial.println (doc [1] [0]);// ABC
Serial.println (doc [1] [1]);// Def
`` `

Each element can be brought to the type `gson :: Entry` by name (andto object) or index (from array) and use separately so as not to "look" it again:
`` `CPP
GSON :: Entry E = doc ["arr"];
Serial.println (E.LENGTH ());// The length of the array
Serial.println (E [0]);// Hello
Serial.println (E [1]);// True
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
gs.ndobj ();// Complete object 1

gs.end ();// complete the package (necessarily called at the end)

Serial.println (GS);// Conclusion to the port
`` `

<a id="versions"> </a>

## versions
- V1.0

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