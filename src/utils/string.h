#pragma once
#include <Arduino.h>
#include <Print.h>
#include <StringUtils.h>

namespace gson {

class string : public Printable {
   public:
    string(uint16_t res = 0) {
        if (res) reserve(res);
    }

    // доступ к строке
    String s;

    // доступ к строке
    operator String&() {
        return s;
    }

    // доступ к строке
    operator sutil::AnyText() {
        return s;
    }

    // Напечатать в Print
    size_t printTo(Print& p) const {
        return p.print(s);
    }

    // очистить строку
    void clear() {
        s = "";
    }

    // длина строки
    uint16_t length() {
        return s.length();
    }

    // зарезервировать строку
    bool reserve(uint16_t res) {
        return s.reserve(res);
    }

    // завершить пакет
    string& end() {
        _replaceComma('\0');
        return *this;
    }

    // =============== ADD ===============

    // прибавить gson::string. Будет добавлена запятая
    string& add(const string& str) {
        s += str.s;
        comma();
        return *this;
    }

    // прибавить gson::string. Будет добавлена запятая
    void operator+=(const string& str) {
        add(str);
    }

    // прибавить gson::string. Будет добавлена запятая
    void operator=(const string& str) {
        add(str);
    }

    // =============== KEY ===============

    // добавить ключ (строка любого типа)
    string& addKey(const sutil::AnyText& key) {
        if (key.valid()) {
            _addRaw(key, true, false);
            colon();
        }
        return *this;
    }

    // добавить ключ (строка любого типа)
    string& operator[](const sutil::AnyText& key) {
        return addKey(key);
    }

    // =============== TEXT ===============

    // прибавить текст (строка любого типа) без запятой и кавычек
    string& addText(const sutil::AnyText& str) {
        if (str.valid()) _addRaw(str, false, false);
        return *this;
    }

    // прибавить текст (строка любого типа) без запятой и кавычек с escape символов
    string& addTextEsc(const sutil::AnyText& str) {
        if (str.valid()) _addRaw(str, false, true);
        return *this;
    }

    // =============== STRING ===============

    // добавить строку (строка любого типа) с escape символов
    string& addStringEsc(const sutil::AnyText& key, const sutil::AnyText& value) {
        if (key.valid() && value.valid()) {
            addKey(key);
            addStringEsc(value);
        }
        return *this;
    }

    // добавить строку (строка любого типа) с escape символов
    string& addStringEsc(const sutil::AnyText& value) {
        if (value.valid()) {
            _addRaw(value, true, true);
            comma();
        }
        return *this;
    }

    // добавить строку (строка любого типа)
    string& addString(const sutil::AnyText& key, const sutil::AnyText& value) {
        if (key.valid() && value.valid()) {
            addKey(key);
            addString(value);
        }
        return *this;
    }

    // добавить строку (строка любого типа)
    string& addString(const sutil::AnyText& value) {
        if (value.valid()) {
            _addRaw(value, true, false);
            comma();
        }
        return *this;
    }

    // добавить строку (строка любого типа)
    void operator=(const char* value) {
        addStringEsc(value);
    }
    void operator+=(const char* value) {
        addStringEsc(value);
    }

    void operator=(const __FlashStringHelper* value) {
        addStringEsc(value);
    }
    void operator+=(const __FlashStringHelper* value) {
        addStringEsc(value);
    }

    void operator=(const String& value) {
        addStringEsc(value);
    }
    void operator+=(const String& value) {
        addStringEsc(value);
    }

    void operator=(String& value) {
        addStringEsc(value);
    }
    void operator+=(String& value) {
        addStringEsc(value);
    }

    // =============== BOOL ===============

    // добавить bool
    string& addBool(const sutil::AnyText& key, const bool& value) {
        if (key.valid()) {
            addKey(key);
            addBool(value);
        }
        return *this;
    }

    // добавить bool
    string& addBool(const bool& value) {
        s += value ? F("true") : F("false");
        comma();
        return *this;
    }

    // добавить bool
    void operator=(const bool& value) {
        addBool(value);
    }
    void operator+=(const bool& value) {
        addBool(value);
    }

    // =============== FLOAT ===============

    // добавить float
    string& addFloat(const sutil::AnyText& key, const double& value, uint8_t dec = 2) {
        if (key.valid()) {
            addKey(key);
            addFloat(value, dec);
        }
        return *this;
    }

    // добавить float
    string& addFloat(const double& value, uint8_t dec = 2) {
        if (isnan(value)) s += '0';
        else {
            char buf[33];
            dtostrf(value, dec + 2, dec, buf);
            s += buf;
        }
        comma();
        return *this;
    }

    // добавить float
    void operator=(const double& value) {
        addFloat(value);
    }
    void operator+=(const double& value) {
        addFloat(value);
    }

    // =============== INT ===============

#ifndef SUTIL_NO_VALUE
    // добавить int
    string& addInt(const sutil::AnyText& key, const sutil::AnyValue& value) {
        if (key.valid() && value.valid()) {
            addKey(key);
            addInt(value);
        }
        return *this;
    }

    // добавить int
    string& addInt(const sutil::AnyValue& value) {
        if (value.valid()) {
            value.addString(s);
            comma();
        }
        return *this;
    }
#else
    // добавить int
    template <typename T>
    string& addInt(const sutil::AnyText& key, T value) {
        if (key.valid()) {
            addKey(key);
            addInt(value);
        }
        return *this;
    }

    // добавить int
    template <typename T>
    string& addInt(T value) {
        s += value;
        comma();
        return *this;
    }
#endif

    void operator=(const char& value) {
        addInt(value);
    }
    void operator+=(const char& value) {
        addInt(value);
    }

    void operator=(const unsigned char& value) {
        addInt(value);
    }
    void operator+=(const unsigned char& value) {
        addInt(value);
    }

    void operator=(const short& value) {
        addInt(value);
    }
    void operator+=(const short& value) {
        addInt(value);
    }

    void operator=(const unsigned short& value) {
        addInt(value);
    }
    void operator+=(const unsigned short& value) {
        addInt(value);
    }

    void operator=(const int& value) {
        addInt(value);
    }
    void operator+=(const int& value) {
        addInt(value);
    }

    void operator=(const unsigned int& value) {
        addInt(value);
    }
    void operator+=(const unsigned int& value) {
        addInt(value);
    }

    void operator=(const long& value) {
        addInt(value);
    }
    void operator+=(const long& value) {
        addInt(value);
    }

    void operator=(const unsigned long& value) {
        addInt(value);
    }
    void operator+=(const unsigned long& value) {
        addInt(value);
    }

#ifndef SUTIL_NO_VALUE
    void operator=(const long long& value) {
        addInt(value);
    }
    void operator+=(const long long& value) {
        addInt(value);
    }

    void operator=(const unsigned long long& value) {
        addInt(value);
    }
    void operator+=(const unsigned long long& value) {
        addInt(value);
    }
#endif
    // =============== CONTAINER ===============

    // начать объект
    string& beginObj(const sutil::AnyText& key = sutil::AnyText()) {
        addKey(key);
        s += '{';
        return *this;
    }

    // завершить объект
    string& endObj() {
        _replaceComma('}');
        comma();
        return *this;
    }

    // начать массив
    string& beginArr(const sutil::AnyText& key = sutil::AnyText()) {
        addKey(key);
        s += '[';
        return *this;
    }

    // завершить массив
    string& endArr() {
        _replaceComma(']');
        comma();
        return *this;
    }

    // запятая
    void comma() {
        afterValue();
        s += ',';
    }

    // двойные кавычки
    void quotes() {
        s += '\"';
    }

    // двоеточие
    void colon() {
        s += ':';
    }

    // =============== PRIVATE ===============
   protected:
    // вызывается перед запятой (после добавления значения)
    virtual void afterValue() {}

    // escape символов
    virtual void escape(const sutil::AnyText& text) {
        uint16_t len = text.length();
        char p = 0;
        for (uint16_t i = 0; i < len; i++) {
            char c = text.charAt(i);
            switch (c) {
                case '\"':
                case '\\':
                    if (p != '\\') s += '\\';
                    s += c;
                    break;
                case '\n':
                    s += '\\';
                    s += 'n';
                    break;
                case '\r':
                    s += '\\';
                    s += 'r';
                    break;
                case '\t':
                    s += '\\';
                    s += 't';
                    break;
                default:
                    s += c;
                    break;
            }
            p = c;
        }
    }

   private:
    void _addRaw(const sutil::AnyText& text, bool quot, bool esc) {
        if (quot) quotes();
        if (esc) {
            if (!s.reserve(s.length() + text.length())) return;
            escape(text);
        } else {
            text.addString(s);
        }
        if (quot) quotes();
    }
    void _replaceComma(const char& sym) {
        int16_t len = s.length() - 1;
        if (s[len] == ',') {
            if (!sym) s.remove(len);
            else s[len] = sym;
        } else {
            if (sym) s += sym;
        }
    }
};

}  // namespace gson