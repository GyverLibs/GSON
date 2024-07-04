#pragma once
#include <Arduino.h>
#include <GTL.h>
#include <StringUtils.h>

// бинарный JSON, может распаковываться в обычный. Структура данных:
// [BS_KEY_CODE][code]
// [BS_KEY_STR8][len] ...
// [BS_KEY_STR16][lenMSB][lenLSB] ...
// [BS_DATA_CODE][code]
// [BS_DATA_INTx][len] MSB...
// [BS_DATA_FLOAT][dec] MSB...
// [BS_DATA_STR8][len] ...
// [BS_DATA_STR16][lenMSB][lenLSB] ...

#define BS_KEY_CODE '$'
#define BS_KEY_STR8 'k'
#define BS_KEY_STR16 'K'

#define BS_DATA_CODE '#'
#define BS_DATA_FLOAT 'f'
#define BS_DATA_STR8 's'
#define BS_DATA_STR16 'S'

#define BS_DATA_INT8 '1'
#define BS_DATA_INT16 '2'
#define BS_DATA_INT32 '4'
#define BS_DATA_INT64 '8'

#define BS_DATA_UINT8 'b'
#define BS_DATA_UINT16 'c'
#define BS_DATA_UINT32 'e'
#define BS_DATA_UINT64 'i'

class BSON : public gtl::stack_uniq<uint8_t> {
   public:
    operator Text() {
        return Text(_buf, _len);
    }

    // data
    void addCode(uint8_t key, uint8_t val) {
        reserve(length() + 4);
        addKey(key);
        addCode(val);
    }
    void addCode(Text key, uint8_t val) {
        addKey(key);
        addCode(val);
    }
    void addCode(uint8_t val) {
        push(BS_DATA_CODE);
        push(val);
    }

    // bool
    void addBool(bool b) {
        addInt(b);
    }
    void addBool(uint8_t key, bool b) {
        reserve(length() + 4);
        addKey(key);
        addBool(b);
    }
    void addBool(Text key, bool b) {
        reserve(length() + 4);
        addKey(key);
        addBool(b);
    }

    // int
    template <typename T>
    void addInt(T val) {
        reserve(length() + sizeof(T) + 1);
        push(sizeof(T) + '0');
        _int(val);
    }
    template <typename T>
    void addInt(uint8_t key, T val) {
        addKey(key);
        addInt(val);
    }
    template <typename T>
    void addInt(Text key, T val) {
        addKey(key);
        addInt(val);
    }

    // uint
    template <typename T>
    void addUint(T val) {
        reserve(length() + sizeof(T) + 1);
        push(sizeof(T) + 'a');
        _int(val);
    }
    template <typename T>
    void addUint(uint8_t key, T val) {
        addKey(key);
        addUint(val);
    }
    template <typename T>
    void addUint(Text key, T val) {
        addKey(key);
        addUint(val);
    }

    // float
    void addFloat(float f, uint8_t dec) {
        reserve(length() + 6);
        push(BS_DATA_FLOAT);
        push(dec);
        _int(f);
    }
    void addFloat(uint8_t key, float f, uint8_t dec) {
        addKey(key);
        addFloat(f, dec);
    }
    void addFloat(Text key, float f, uint8_t dec) {
        addKey(key);
        addFloat(f, dec);
    }

    // text
    void addText(Text text) {
        reserve(length() + text.length() + 3);
        push(text.length() <= 255 ? BS_DATA_STR8 : BS_DATA_STR16);
        _text(text);
    }
    void addText(uint8_t key, Text text) {
        if (!text) return;
        reserve(length() + text.length() + 5);
        addKey(key);
        addText(text);
    }
    void addText(Text key, Text text) {
        if (!text) return;
        addKey(key);
        addText(text);
    }

    // key
    void addKey(uint8_t key) {
        push(BS_KEY_CODE);
        push(key);
    }
    void addKey(Text key) {
        reserve(length() + key.length() + 3);
        push(key.length() <= 255 ? BS_KEY_STR8 : BS_KEY_STR16);
        _text(key);
    }

    // object
    void beginObj() {
        push('{');
    }
    void beginObj(uint8_t key) {
        reserve(length() + 4);
        addKey(key);
        beginObj();
    }
    void beginObj(Text key) {
        addKey(key);
        beginObj();
    }
    void endObj() {
        push('}');
    }

    // array
    void beginArr() {
        push('[');
    }
    void beginArr(uint8_t key) {
        reserve(length() + 4);
        addKey(key);
        beginArr();
    }
    void beginArr(Text key) {
        addKey(key);
        beginArr();
    }
    void endArr() {
        push(']');
    }

   private:
    template <typename T>
    void _int(T val) {
        uint8_t b = sizeof(T);
        while (b--) push(((uint8_t*)&val)[b]);
    }

    void _text(Text text) {
        if (text.length() > 255) _int((uint16_t)text.length());
        else push(text.length());
        for (uint8_t i = 0; i < (uint8_t)text._len; i++) {
            push(text._charAt(i));
        }
    }
};

/*
String decodeBsonArduino(BSON& b) {
    String s;
    for (size_t i = 0; i < b.length(); i++) {
        char c = b[i];
        switch (c) {
            case ']':
            case '}':
                if (s[s.length() - 1] == ',') s[s.length() - 1] = c;
                else s += c;
                s += ',';
                break;

            case '[':
            case '{':
                s += c;
                break;

            case BS_DATA_CODE:
            case BS_KEY_CODE:
                s += (uint8_t)b[++i];
                s += (c == BS_KEY_CODE) ? ':' : ',';
                break;

            case BS_DATA_STR16:
            case BS_KEY_STR16:
            case BS_DATA_STR8:
            case BS_KEY_STR8: {
                bool key = (c == BS_KEY_STR8 || c == BS_KEY_STR16);
                uint16_t len = b[++i];
                if (c == BS_KEY_STR16 || c == BS_DATA_STR16) {
                    len <<= 8;
                    len |= b[++i];
                }
                s += '\"';
                while (len--) s += (char)b[++i];
                s += '\"';
                s += key ? ':' : ',';
            } break;

            case BS_DATA_INT8:
            case BS_DATA_INT16:
            case BS_DATA_INT32:
            case BS_DATA_INT64: {
                uint8_t size = c - '0';
                uint32_t v = 0;
                while (size--) {
                    v <<= 8;
                    v |= b[++i];
                }
                s += v;
                s += ',';
            } break;

            case BS_DATA_UINT8:
            case BS_DATA_UINT16:
            case BS_DATA_UINT32:
            case BS_DATA_UINT64: {
                uint8_t size = c - 'a';
                uint32_t v = 0;
                while (size--) {
                    v <<= 8;
                    v |= b[++i];
                }
                s += v;
                s += ',';
            } break;

            case BS_DATA_FLOAT: {
                uint32_t v = 0;
                uint8_t dec = b[++i];
                uint8_t size = 4;
                while (size--) {
                    v <<= 8;
                    v |= b[++i];
                }
                s += *((float*)&v); // todo dec
                s += ',';
            } break;
        }
    }
    if (s[s.length() - 1] == ',') s.remove(s.length() - 1);
    return s;
}
*/

/*
const codes = [
    'some',
    'string',
    'constants',
];

// @param b {Uint8Array}
export default function decodeBson(b) {
    if (!b || !(b instanceof Uint8Array) || !b.length) return null;

    const BS_KEY_CODE = '$';
    const BS_KEY_STR8 = 'k';
    const BS_KEY_STR16 = 'K';

    const BS_DATA_CODE = '#';
    const BS_DATA_FLOAT = 'f';
    const BS_DATA_STR8 = 's';
    const BS_DATA_STR16 = 'S';

    const BS_DATA_INT8 = '1';
    const BS_DATA_INT16 = '2';
    const BS_DATA_INT32 = '4';
    const BS_DATA_INT64 = '8';

    const BS_DATA_UINT8 = 'b';
    const BS_DATA_UINT16 = 'c';
    const BS_DATA_UINT32 = 'e';
    const BS_DATA_UINT64 = 'i';

    function ieee32ToFloat(intval) {
        var fval = 0.0;
        var x;
        var m;
        var s;
        s = (intval & 0x80000000) ? -1 : 1;
        x = ((intval >> 23) & 0xFF);
        m = (intval & 0x7FFFFF);
        switch (x) {
            case 0:
                break;
            case 0xFF:
                if (m) fval = NaN;
                else if (s > 0) fval = Number.POSITIVE_INFINITY;
                else fval = Number.NEGATIVE_INFINITY;
                break;
            default:
                x -= 127;
                m += 0x800000;
                fval = s * (m / 8388608.0) * Math.pow(2, x);
                break;
        }
        return fval;
    }

    let s = '';
    for (let i = 0; i < b.length; i++) {
        let c = String.fromCharCode(b[i]);
        switch (c) {
            case ']':
            case '}':
                if (s[s.length - 1] == ',') s = s.slice(0, -1);
                s += c;
                s += ',';
                break;

            case '[':
            case '{':
                s += c;
                break;

            case BS_DATA_CODE:
            case BS_KEY_CODE: {
                s += '"';
                s += codes[b[++i]];
                s += '"';
                s += (c == BS_KEY_CODE) ? ':' : ',';
            } break;

            case BS_DATA_STR16:
            case BS_KEY_STR16:
            case BS_DATA_STR8:
            case BS_KEY_STR8: {
                let key = (c == BS_KEY_STR8 || c == BS_KEY_STR16);
                let len = b[++i];
                if (c == BS_KEY_STR16 || c == BS_DATA_STR16) {
                    len <<= 8;
                    len |= b[++i];
                }
                s += '\"';
                while (len--) s += String.fromCharCode(b[++i]);
                s += '\"';
                s += key ? ':' : ',';
            } break;

            case BS_DATA_INT8:
            case BS_DATA_INT16:
            case BS_DATA_INT32:
            case BS_DATA_INT64: {
                let size = b[i] - '0'.charCodeAt(0);
                let v = 0;
                while (size--) {
                    v <<= 8;
                    v |= b[++i];
                }
                switch (c) {
                    case BS_DATA_INT8: s += (new Int8Array([v]))[0]; break;
                    case BS_DATA_INT16: s += (new Int16Array([v]))[0]; break;
                    case BS_DATA_INT32: s += (new Int32Array([v]))[0]; break;
                    case BS_DATA_UINT64: s += (new BigInt64Array([v]))[0]; break;
                }
                s += ',';
            } break;

            case BS_DATA_UINT8:
            case BS_DATA_UINT16:
            case BS_DATA_UINT32:
            case BS_DATA_UINT64: {
                let size = b[i] - 'a'.charCodeAt(0);
                let v = 0;
                while (size--) {
                    v <<= 8;
                    v |= b[++i];
                }
                switch (c) {
                    case BS_DATA_UINT8: s += (new Uint8Array([v]))[0]; break;
                    case BS_DATA_UINT16: s += (new Uint16Array([v]))[0]; break;
                    case BS_DATA_UINT32:
                    case BS_DATA_UINT64: s += (v >>> 0); break;
                }
                s += ',';
            } break;

            case BS_DATA_FLOAT: {
                let v = 0;
                let dec = b[++i];
                let size = 4;
                while (size--) {
                    v <<= 8;
                    v |= b[++i];
                }
                s += ieee32ToFloat(v).toFixed(dec);
                s += ',';
            } break;
        }
    }
    if (s[s.length - 1] == ',') s = s.slice(0, -1);

    try {
        return JSON.parse(s);
    } catch (e) {
        throw new Error("JSON error")
    }
}
*/