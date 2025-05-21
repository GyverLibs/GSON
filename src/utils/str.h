#pragma once
#include <Arduino.h>
#include <GTL.h>
#include <Print.h>
#include <StringUtils.h>

namespace gtl {
class string : protected gtl::stack<char> {
    typedef gtl::stack<char> ST;

   public:
    // ================= STR =================

    void concat(char sym) {
        ST::push(sym);
    }

    void concat(char* str) {
        ST::concat(str, strlen(str));
    }
    void concat(char* str, uint16_t len) {
        ST::concat(str, len);
    }

    void concat(const char* str) {
        ST::concat(str, strlen(str));
    }
    void concat(const char* str, uint16_t len) {
        ST::concat(str, len);
    }

    void concat(const __FlashStringHelper* fstr) {
        ST::concat((PGM_P)fstr, strlen_P((PGM_P)fstr), true);
    }
    void concat(const __FlashStringHelper* fstr, uint16_t len) {
        ST::concat((PGM_P)fstr, len, true);
    }

    void concat(const String& s) {
        ST::concat(s.c_str(), s.length());
    }
    void concat(const Text& txt) {
        ST::concat(txt.str(), txt.length(), txt.pgm());
    }

    // ================= NUM =================

    void concat(bool b) {
        b ? concat(F("true"), 4) : concat(F("false"), 5);
    }

    void concat(signed char val) {
        concat((long)val);
    }
    void concat(short val) {
        concat((long)val);
    }
    void concat(int val) {
        concat((long)val);
    }
    void concat(long val) {
        if (addCapacity(12)) _len += su::intToStr(val, buf() + _len);
    }
    void concat(long long val) {
        if (addCapacity(21)) _len += su::int64ToStr(val, buf() + _len);
    }

    void concat(unsigned char val) {
        concat((unsigned long)val);
    }
    void concat(unsigned short val) {
        concat((unsigned long)val);
    }
    void concat(unsigned int val) {
        concat((unsigned long)val);
    }
    void concat(unsigned long val) {
        if (addCapacity(12)) _len += su::uintToStr(val, buf() + _len);
    }
    void concat(unsigned long long val) {
        if (addCapacity(21)) _len += su::uint64ToStr(val, buf() + _len);
    }

    void concat(float val, uint8_t dec = 2) {
        if (isnan(val) || isinf(val)) {
            concat(F("null"), 4);
            return;
        }
        uint8_t len = su::floatLen(val, dec);
        if (addCapacity(len)) {
            dtostrf(val, dec ? dec + 2 : 1, dec, buf() + _len);
            _len += len;
        }
    }
    void concat(double val, uint8_t dec = 2) {
        concat(float(val), dec);
    }

    using ST::buf;
    using ST::clear;
    using ST::concat;
    using ST::length;
    using ST::reserve;
};
}  // namespace gtl

namespace gson {

class Str : public Printable, public gtl::string {
   public:
    Str() {}
    Str(uint16_t res) {
        reserve(res);
    }

    explicit operator bool() const {
        return true;
    }

    operator Text() const {
        return Text(buf(), length());
    }

    size_t printTo(Print& p) const {
        return p.write(buf(), length());
    }

    void operator=(const Str& str) {
        if (!str.length()) return;
        _checkNc();
        concat(str);
        _resetNc();
    }
    void operator+=(const Str& str) {
        if (!str.length()) return;
        _checkNc();
        concat(str);
        _resetNc();
    }

    // =================== VAL ===================
#define GS_STR_MAKE_VAL(T)               \
    void operator=(T val) { _val(val); } \
    void operator+=(T val) { _val(val); }

    GS_STR_MAKE_VAL(bool)
    GS_STR_MAKE_VAL(signed char)
    GS_STR_MAKE_VAL(unsigned char)
    GS_STR_MAKE_VAL(short)
    GS_STR_MAKE_VAL(unsigned short)
    GS_STR_MAKE_VAL(int)
    GS_STR_MAKE_VAL(unsigned int)
    GS_STR_MAKE_VAL(long)
    GS_STR_MAKE_VAL(unsigned long)
    GS_STR_MAKE_VAL(long long)
    GS_STR_MAKE_VAL(unsigned long long)
    GS_STR_MAKE_VAL(float)
    GS_STR_MAKE_VAL(double)

    // добавить float с кол-вом знаков
    void add(float val, uint8_t dec = 2) {
        _checkNc();
        concat(val, dec);
        _resetNc();
    }

    // =================== NULL ===================
    void operator=(nullptr_t) { _null(); }
    void operator+=(nullptr_t) { _null(); }

    // =================== STR ===================
    void operator=(char s) { _str(s); }
    void operator+=(char s) { _str(s); }
    void operator=(char* s) { _str(s); }
    void operator+=(char* s) { _str(s); }
    void operator=(const char* s) { _str(s); }
    void operator+=(const char* s) { _str(s); }
    void operator=(const __FlashStringHelper* s) { _str(s); }
    void operator+=(const __FlashStringHelper* s) { _str(s); }
    void operator=(const String& s) { _str(s); }
    void operator+=(const String& s) { _str(s); }
    void operator=(const Text& s) { _str(s); }
    void operator+=(const Text& s) { _str(s); }

    // прибавить строку с escape
    void escape(const Text& txt) {
        _checkNc();
        push('\"');
        uint16_t len = txt.length();
        reserve(len);
        char p = 0;
        for (uint16_t i = 0; i < len; i++) {
            char c = txt._charAt(i);
            switch (c) {
                case '\"':
                case '\\':
                    if (p != '\\') push('\\');
                    push(c);
                    break;
                case '\n':
                    push('\\');
                    push('n');
                    break;
                case '\r':
                    push('\\');
                    push('r');
                    break;
                case '\t':
                    push('\\');
                    push('t');
                    break;
                default:
                    push(c);
                    break;
            }
            p = c;
        }
        push('\"');
        _resetNc();
    }

    // =================== KEY ===================
    Str& operator[](const char* s) { return _key(s); }
    Str& operator[](const __FlashStringHelper* s) { return _key(s); }
    Str& operator[](const String& s) { return _key(s); }
    Str& operator[](const Text& s) { return _key(s); }

    // =================== CONT ===================
    Str& operator()(char cont) {
        switch (cont) {
            case '{':
            case '[':
                _checkNc();
                break;
            default:
                _resetNc();
                break;
        }
        push(cont);
        return *this;
    }

    using gtl::string::concat;

   private:
    bool _nc = false;

    Str& _key(const Text& t) {
        _checkNc();
        push('\"');
        concat(t);
        push('\"');
        push(':');
        return *this;
    }
    template <typename T>
    void _val(T val) {
        _checkNc();
        concat(val);
        _resetNc();
    }
    void _str(const Text& t) {
        _checkNc();
        push('\"');
        concat(t);
        push('\"');
        _resetNc();
    }
    void _str(char ch) {
        _checkNc();
        push('\"');
        push(ch);
        push('\"');
        _resetNc();
    }
    void _null() {
        _checkNc();
        concat(F("null"), 4);
        _resetNc();
    }
    inline void _checkNc() {
        if (_nc) _nc = false, push(',');
    }
    inline void _resetNc() {
        _nc = true;
    }
};

}  // namespace gson