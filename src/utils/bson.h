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

class BSON : public gtl::stack_uniq<uint8_t> {
   public:
    operator Text() {
        return Text(_buf, _len);
    }

    // code
    void addCode(uint16_t key, uint16_t val) {
        reserve(length() + 5);
        addKey(key);
        addCode(val);
    }
    void addCode(Text key, uint16_t val) {
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
    void addBool(Text key, bool b) {
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
    void addUint(uint64_t val) {
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
    void addUint(Text key, T val) {
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
    void addInt(int64_t val) {
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
    void addInt(Text key, T val) {
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
    void addFloat(Text key, T value, uint8_t dec) {
        addKey(key);
        addFloat(value, dec);
    }

    // text
    void addText(Text text) {
        _text(text, BS_VAL_STR);
    }
    void addText(uint16_t key, Text text) {
        reserve(length() + text.length() + 5);
        addKey(key);
        _text(text, BS_VAL_STR);
    }
    void addText(Text key, Text text) {
        addKey(key);
        _text(text, BS_VAL_STR);
    }

    // key
    void addKey(uint16_t key) {
        push(BS_KEY_CODE | (key & 0b000));
        push(BS_LSB(key));
    }
    void addKey(Text key) {
        _text(key, BS_KEY_STR);
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
    void beginObj(Text key) {
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
    void beginArr(Text key) {
        addKey(key);
        beginArr();
    }
    void endArr() {
        push(BS_CONT_CLOSE | BS_CONT_ARR);
    }

   private:
    void _text(Text& text, uint8_t type) {
        reserve(length() + text.length() + 3);
        push(type | BS_MSB5(text.length()));
        push(BS_LSB(text.length()));
        concat((uint8_t*)text.str(), text.length(), text.pgm());
    }
    uint8_t _uintSize(uint32_t val) {
        switch (val) {
            case 0 ... 0xff:
                return 1;
            case 0xff + 1 ... 0xffff:
                return 2;
            case 0xffff + 1 ... 0xffffff:
                return 3;
            case 0xffffff + 1 ... 0xffffffff:
                return 4;
        }
        return 0;
    }
    uint8_t _uint64Size(uint64_t val) {
        return (val > ULONG_MAX) ? 8 : _uintSize(val);
    }
};