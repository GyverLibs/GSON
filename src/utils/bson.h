#pragma once
#include <Arduino.h>
#include <GTL.h>
#include <StringUtils.h>
#include <limits.h>

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
        uint8_t len = _intSize(val);
        reserve(length() + len + 1);
        push(len + '0');
        _writeMSB(&val, len);
    }
    void addInt(int64_t val) {
        uint8_t len = _int64Size(val);
        reserve(length() + len + 1);
        push(len + '0');
        _writeMSB(&val, len);
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
        uint8_t len = _uintSize(val);
        reserve(length() + len + 1);
        push(len + 'a');
        _writeMSB(&val, len);
    }
    void addUint(uint64_t val) {
        uint8_t len = _uint64Size(val);
        reserve(length() + len + 1);
        push(len + 'a');
        _writeMSB(&val, len);
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
    template <typename T>
    void addFloat(T value, uint8_t dec) {
        reserve(length() + 6);
        push(BS_DATA_FLOAT);
        push(dec);
        float f = value;
        _writeMSB(&f, 4);
    }
    template <typename T>
    void addFloat(uint8_t key, T value, uint8_t dec) {
        addKey(key);
        addFloat(value, dec);
    }
    template <typename T>
    void addFloat(Text key, T value, uint8_t dec) {
        addKey(key);
        addFloat(value, dec);
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
    void _text(Text text) {
        _writeMSB(&text._len, text._len > 255 ? 2 : 1);
        for (uint8_t i = 0; i < (uint8_t)text._len; i++) {
            push(text._charAt(i));
        }
    }

    void _writeMSB(void* val, uint8_t size) {
        while (size--) push(((uint8_t*)val)[size]);
    }
    uint8_t _uintSize(uint32_t val) {
        switch (val) {
            case 0 ... UCHAR_MAX:
                return 1;
            case UCHAR_MAX + 1 ... USHRT_MAX:
                return 2;
            case USHRT_MAX + 1 ... ULONG_MAX:
                return 4;
        }
        return 0;
    }
    uint8_t _uint64Size(uint64_t& val) {
        return (val > ULONG_MAX) ? 8 : _uintSize(val);
    }
    uint8_t _intSize(int32_t val) {
        switch (val >= 0 ? val : -val) {
            case 0 ... CHAR_MAX:
                return 1;
            case CHAR_MAX + 1 ... SHRT_MAX:
                return 2;
            case SHRT_MAX + 1 ... LONG_MAX:
                return 4;
        }
        return 0;
    }
    uint8_t _int64Size(int64_t& val) {
        return ((val >= 0 ? val : -val) > ULONG_MAX) ? 8 : _intSize(val);
    }
};