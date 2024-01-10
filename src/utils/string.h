#pragma once
#include <Arduino.h>
#include <StringUtils.h>

namespace gson {

using sutil::AnyText;
using sutil::AnyValue;

class string {
   public:
    // доступ к строке
    String s;

    // доступ к строке
    operator String&() {
        return s;
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

    // прибавить gson::string. Будет добавлена запятая
    string& add(string& str) {
        s += str.s;
        return comma();
    }

    // прибавить gson::string. Будет добавлена запятая
    void operator+=(string& str) {
        add(str);
    }

    // =============== KEY ===============

    // добавить ключ (строка любого типа)
    string& addKey(AnyText key) {
        if (key.valid()) {
            _text(key);
            s += ':';
        }
        return *this;
    }

    // добавить ключ (строка любого типа)
    string& operator[](AnyText key) {
        return addKey(key);
    }

    // =============== VALUE ===============

    // прибавить текст (строка любого типа)
    string& addRaw(AnyText str, bool esc = false) {
        _addText(str, esc);
        return *this;
    }

    // // прибавить текст (строка любого типа)
    // void operator+=(AnyText str) {
    //     _addText(str);
    // }

    // ======================
    // добавить строку (строка любого типа)
    string& __attribute__((deprecated)) addStr(AnyText key, AnyText value) {
        addKey(key);
        return addStr(value);
    }

    // добавить строку (строка любого типа)
    string& __attribute__((deprecated)) addStr(AnyText value) {
        _text(value, true);
        return comma();
    }
    // ======================

    // добавить строку (строка любого типа)
    string& addString(AnyText key, AnyText value) {
        addKey(key);
        return addString(value);
    }

    // добавить строку (строка любого типа)
    string& addString(AnyText value) {
        _text(value, true);
        return comma();
    }

    // добавить строку (строка любого типа)
    void operator=(const char* value) {
        addString(value);
    }
    void operator+=(const char* value) {
        addString(value);
    }

    void operator=(const __FlashStringHelper* value) {
        addString(value);
    }
    void operator+=(const __FlashStringHelper* value) {
        addString(value);
    }

    void operator=(const String& value) {
        addString(value);
    }
    void operator+=(const String& value) {
        addString(value);
    }

    void operator=(String& value) {
        addString(value);
    }
    void operator+=(String& value) {
        addString(value);
    }

    // добавить bool
    string& addBool(AnyText key, const bool& value) {
        addKey(key);
        return addBool(value);
    }

    // добавить bool
    string& addBool(const bool& value) {
        s += value ? F("true") : F("false");
        return comma();
    }

    // добавить bool
    void operator=(const bool& value) {
        addBool(value);
    }
    void operator+=(const bool& value) {
        addBool(value);
    }

    // добавить float
    string& addFloat(AnyText key, const double& value, uint8_t dec = 2) {
        addKey(key);
        return addFloat(value, dec);
    }

    // добавить float
    string& addFloat(const double& value, uint8_t dec = 2) {
        if (isnan(value)) s += '0';
        else {
            char buf[33];
            dtostrf(value, dec + 2, dec, buf);
            s += buf;
        }
        return comma();
    }

    // добавить float
    void operator=(const double& value) {
        addFloat(value);
    }
    void operator+=(const double& value) {
        addFloat(value);
    }

    // добавить int
    string& addInt(AnyText key, AnyValue value) {
        addKey(key);
        value.addString(s);
        return comma();
    }

    // добавить int
    string& addInt(AnyValue value) {
        value.addString(s);
        return comma();
    }

    // добавить int
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

    // =============== CONTAINER ===============

    // начать объект
    string& beginObj(AnyText key = AnyText()) {
        addKey(key);
        s += '{';
        return *this;
    }

    // завершить объект
    string& endObj() {
        _replaceComma('}');
        return comma();
    }

    // начать массив
    string& beginArr(AnyText key = AnyText()) {
        addKey(key);
        s += '[';
        return *this;
    }

    // завершить массив
    string& endArr() {
        _replaceComma(']');
        return comma();
    }

    // двойные кавычки
    string& dq() {
        s += '\"';
        return *this;
    }

    // запятая
    string& comma() {
        s += ',';
        return *this;
    }

    // двоеточие
    string& colon() {
        s += ':';
        return *this;
    }

    // =============== PRIVATE ===============
   private:
    void _replaceComma(char sym) {
        if (s[s.length() - 1] == ',') {
            if (!sym) {
                s.remove(s.length() - 1);
                return;
            }
            s[s.length() - 1] = sym;
        } else s += sym;
    }
    void _text(AnyText& value, bool esc = false) {
        dq();
        _addText(value, esc);
        dq();
    }
    void _addText(AnyText& value, bool esc = false) {
        if (esc) {
            s.reserve(s.length() + value.length());
            char p = 0;
            for (uint16_t i = 0; i < value.length(); i++) {
                char c = value.charAt(i);
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
        } else {
            value.addString(s);
        }
    }
};

}  // namespace gson