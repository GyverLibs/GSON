#pragma once
#include <Arduino.h>
#include <GTL.h>
#include <StringUtils.h>
#include <limits.h>

// бинарный JSON, может распаковываться в обычный. Структура данных:
/*
0 key code: [code msb:5] + [code:8]
1 key str: [len msb:5] + [len:8] + [...]
2 val code: [code msb:5] + [code:8]
3 val str: [len msb:5] + [len:8] + [...]
4 val int: [sign:1 + len:4]
5 val float: [dec:5]
6 cont: [obj:1 / arr:0] [open:1 / close:0]
7 bin: [len msb:5] + [len:8] + [...]
*/

#define BS_MAX_LEN ((size_t)0b0001111111111111)

#define BS_KEY_CODE (0 << 5)
#define BS_KEY_STR (1 << 5)
#define BS_VAL_CODE (2 << 5)
#define BS_VAL_STR (3 << 5)
#define BS_VAL_INT (4 << 5)
#define BS_VAL_FLOAT (5 << 5)
#define BS_CONTAINER (6 << 5)
#define BS_BINARY (7 << 5)

#define BS_CONT_OBJ (1 << 4)
#define BS_CONT_ARR (0 << 4)
#define BS_CONT_OPEN (1 << 3)
#define BS_CONT_CLOSE (0 << 3)

#define BS_NEGATIVE (0b00010000)
#define BS_MSB5(x) ((x >> 8) & 0b00011111)
#define BS_LSB5(x) (x & 0b00011111)
#define BS_LSB(x) (x & 0xff)

class BSON : private gtl::stack_uniq<uint8_t> {
   public:
    using gtl::stack_uniq<uint8_t>::write;
    using gtl::stack_uniq<uint8_t>::reserve;
    using gtl::stack_uniq<uint8_t>::length;
    using gtl::stack_uniq<uint8_t>::buf;
    using gtl::stack_uniq<uint8_t>::clear;
    using gtl::stack_uniq<uint8_t>::move;

    BSON() {}
    BSON(BSON& b) {
        move(b);
    }
    BSON(BSON&& b) {
        move(b);
    }
    BSON& operator=(BSON& b) {
        move(b);
        return *this;
    }
    BSON& operator=(BSON&& b) {
        move(b);
        return *this;
    }

    operator Text() {
        return toText();
    }

    Text toText() {
        return Text(buf(), length());
    }

    // bson
    void add(const BSON& bson) {
        concat(bson);
    }
    void operator+=(const BSON& bson) {
        concat(bson);
    }

    // key
    void addKey(uint16_t key) {
        push(BS_KEY_CODE | (key & 0b000));
        push(BS_LSB(key));
    }
    void addKey(const Text& key) {
        _text(key, BS_KEY_STR);
    }

    BSON& operator[](uint16_t key) {
        addKey(key);
        return *this;
    }
    BSON& operator[](Text key) {
        addKey(key);
        return *this;
    }

    // code
    void addCode(uint16_t key, uint16_t val) {
        reserve(length() + 5);
        addKey(key);
        addCode(val);
    }
    void addCode(const Text& key, uint16_t val) {
        addKey(key);
        addCode(val);
    }
    void addCode(uint16_t val) {
        push(BS_VAL_CODE | BS_MSB5(val));
        push(BS_LSB(val));
    }

    // bool
    void addBool(bool b) {
        addUint(b);
    }
    void addBool(uint16_t key, bool b) {
        addKey(key);
        addUint(b);
    }
    void addBool(const Text& key, bool b) {
        addKey(key);
        addUint(b);
    }

    // uint
    template <typename T>
    void addUint(T val) {
        uint8_t len = _uintSize(val);
        push(BS_VAL_INT | len);
        concat((uint8_t*)&val, len);
    }
    void addUint(unsigned long long val) {
        uint8_t len = _uint64Size(val);
        push(BS_VAL_INT | len);
        concat((uint8_t*)&val, len);
    }
    template <typename T>
    void addUint(uint16_t key, T val) {
        addKey(key);
        addUint(val);
    }
    template <typename T>
    void addUint(const Text& key, T val) {
        addKey(key);
        addUint(val);
    }

    // int
    template <typename T>
    void addInt(T val) {
        uint8_t neg = (val < 0) ? BS_NEGATIVE : 0;
        if (neg) val = -val;
        uint8_t len = _uintSize(val);
        push(BS_VAL_INT | neg | len);
        concat((uint8_t*)&val, len);
    }
    void addInt(long long val) {
        uint8_t neg = (val < 0) ? BS_NEGATIVE : 0;
        if (neg) val = -val;
        uint8_t len = _uint64Size(val);
        push(BS_VAL_INT | neg | len);
        concat((uint8_t*)&val, len);
    }
    template <typename T>
    void addInt(uint16_t key, T val) {
        addKey(key);
        addInt(val);
    }
    template <typename T>
    void addInt(const Text& key, T val) {
        addKey(key);
        addInt(val);
    }

    // float
    template <typename T>
    void addFloat(T value, uint8_t dec) {
        push(BS_VAL_FLOAT | BS_LSB5(dec));
        float f = value;
        concat((uint8_t*)&f, 4);
    }
    template <typename T>
    void addFloat(uint16_t key, T value, uint8_t dec) {
        addKey(key);
        addFloat(value, dec);
    }
    template <typename T>
    void addFloat(const Text& key, T value, uint8_t dec) {
        addKey(key);
        addFloat(value, dec);
    }

#define BSON_MAKE_ADD_STR(type)                \
    void operator=(type val) { addText(val); } \
    void operator+=(type val) { addText(val); }

#define BSON_MAKE_ADD_INT(type)               \
    void operator=(type val) { addInt(val); } \
    void operator+=(type val) { addInt(val); }

#define BSON_MAKE_ADD_UINT(type)               \
    void operator=(type val) { addUint(val); } \
    void operator+=(type val) { addUint(val); }

#define BSON_MAKE_ADD_FLOAT(type)                  \
    void operator=(type val) { addFloat(val, 4); } \
    void operator+=(type val) { addFloat(val, 4); }

    BSON_MAKE_ADD_STR(const char*)
    BSON_MAKE_ADD_STR(const __FlashStringHelper*)
    BSON_MAKE_ADD_STR(const String&)
    BSON_MAKE_ADD_STR(const Text&)

    BSON_MAKE_ADD_UINT(bool)
    BSON_MAKE_ADD_UINT(char)
    BSON_MAKE_ADD_UINT(unsigned char)
    BSON_MAKE_ADD_UINT(unsigned short)
    BSON_MAKE_ADD_UINT(unsigned int)
    BSON_MAKE_ADD_UINT(unsigned long)
    BSON_MAKE_ADD_UINT(unsigned long long)

    BSON_MAKE_ADD_INT(signed char)
    BSON_MAKE_ADD_INT(short)
    BSON_MAKE_ADD_INT(int)
    BSON_MAKE_ADD_INT(long)
    BSON_MAKE_ADD_INT(long long)

    BSON_MAKE_ADD_FLOAT(float)
    BSON_MAKE_ADD_FLOAT(double)

    // text
    void addText(const Text& text) {
        _text(text, BS_VAL_STR);
    }
    void addText(uint16_t key, const Text& text) {
        reserve(length() + text.length() + 5);
        addKey(key);
        _text(text, BS_VAL_STR);
    }
    void addText(const Text& key, const Text& text) {
        addKey(key);
        _text(text, BS_VAL_STR);
    }

    // bin
    void addBin(const void* data, size_t size) {
        if (size > BS_MAX_LEN) return;
        beginBin(size);
        write((const uint8_t*)data, size);
    }
    void addBin(const Text& key, const void* data, size_t size) {
        if (size > BS_MAX_LEN) return;
        addKey(key);
        addBin(data, size);
    }
    void addBin(uint16_t key, const void* data, size_t size) {
        if (size > BS_MAX_LEN) return;
        addKey(key);
        addBin(data, size);
    }
    bool beginBin(uint16_t size) {
        if (size > BS_MAX_LEN) return false;
        push(BS_BINARY | BS_MSB5(size));
        push(BS_LSB(size));
        return true;
    }

    // object
    void beginObj() {
        push(BS_CONTAINER | BS_CONT_OBJ | BS_CONT_OPEN);
    }
    void beginObj(uint16_t key) {
        reserve(length() + 4);
        addKey(key);
        beginObj();
    }
    void beginObj(const Text& key) {
        addKey(key);
        beginObj();
    }
    void endObj() {
        push(BS_CONTAINER | BS_CONT_OBJ | BS_CONT_CLOSE);
    }

    // array
    void beginArr() {
        push(BS_CONTAINER | BS_CONT_ARR | BS_CONT_OPEN);
    }
    void beginArr(uint16_t key) {
        reserve(length() + 4);
        addKey(key);
        beginArr();
    }
    void beginArr(const Text& key) {
        addKey(key);
        beginArr();
    }
    void endArr() {
        push(BS_CONTAINER | BS_CONT_ARR | BS_CONT_CLOSE);
    }

   private:
    void _text(const Text& text, uint8_t type) {
        uint16_t len = min((size_t)text.length(), BS_MAX_LEN);
        reserve(length() + len + 3);
        push(type | BS_MSB5(len));
        push(BS_LSB(len));
        concat((const uint8_t*)text.str(), len, text.pgm());
    }
    uint8_t _uintSize(uint32_t val) {
        switch (val) {
            case 0ul ... 0xfful:
                return 1;
            case 0xfful + 1 ... 0xfffful:
                return 2;
            case 0xfffful + 1ul ... 0xfffffful:
                return 3;
            case 0xfffffful + 1ul ... 0xfffffffful:
                return 4;
        }
        return 0;
    }
    uint8_t _uint64Size(uint64_t val) {
        return (val > ULONG_MAX) ? 8 : _uintSize(val);
    }
};