#pragma once
#include <Arduino.h>
#include <GTL.h>
#include <StringUtils.h>
#include <limits.h>

// бинарный JSON, может распаковываться в обычный. Структура данных:
/*
0 key code: [code msb5] + [code8]
1 key str: [len msb5] + [len8] + [...]
2 val code: [code msb5] + [code8]
3 val str: [len msb5] + [len8] + [...]
4 val int: [sign1 + len4]
5 val float: [dec5]
6 open: [1obj / 0arr]
7 close: [1obj / 0arr]
*/

#define BS_KEY_CODE (0 << 5)
#define BS_KEY_STR (1 << 5)
#define BS_VAL_CODE (2 << 5)
#define BS_VAL_STR (3 << 5)
#define BS_VAL_INT (4 << 5)
#define BS_VAL_FLOAT (5 << 5)
#define BS_CONT_OPEN (6 << 5)
#define BS_CONT_CLOSE (7 << 5)

#define BS_NEGATIVE (0b00010000)
#define BS_CONT_OBJ (1)
#define BS_CONT_ARR (0)
#define BS_MSB5(x) ((x >> 8) & 0b00011111)
#define BS_LSB5(x) (x & 0b00011111)
#define BS_LSB(x) (x & 0xff)

class BSON : private gtl::stack_uniq<uint8_t> {
   public:
    operator Text() {
        return toText();
    }

    Text toText() {
        return Text(buffer(), length());
    }

    const uint8_t* buffer() {
        return gtl::stack_uniq<uint8_t>::buf();
    }

    size_t length() {
        return gtl::stack_uniq<uint8_t>::length();
    }

    // bson
    void add(const BSON& bson) {
        concat(bson);
    }
    // void operator=(const BSON& bson) {
    //     concat(bson);
    // }
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

    // object
    void beginObj() {
        push(BS_CONT_OPEN | BS_CONT_OBJ);
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
        push(BS_CONT_CLOSE | BS_CONT_OBJ);
    }

    // array
    void beginArr() {
        push(BS_CONT_OPEN | BS_CONT_ARR);
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
        push(BS_CONT_CLOSE | BS_CONT_ARR);
    }

   private:
    void _text(const Text& text, uint8_t type) {
        reserve(length() + text.length() + 3);
        push(type | BS_MSB5(text.length()));
        push(BS_LSB(text.length()));
        concat((uint8_t*)text.str(), text.length(), text.pgm());
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