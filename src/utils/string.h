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
    string& add(const string& str) {
        s += str.s;
        return comma();
    }

    // прибавить gson::string. Будет добавлена запятая
    void operator+=(const string& str) {
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

    // прибавить текст (строка любого типа)
    void operator+=(AnyText str) {
        _addText(str);
    }

    // добавить строку (строка любого типа)
    string& addStr(AnyText key, AnyText value) {
        addKey(key);
        return addStr(value);
    }

    // добавить строку (строка любого типа)
    string& addStr(AnyText value) {
        _text(value, true);
        return comma();
    }

    // добавить строку (строка любого типа)
    void operator=(const char* value) {
        addStr(value);
    }
    void operator=(const __FlashStringHelper* value) {
        addStr(value);
    }
    void operator=(const String& value) {
        addStr(value);
    }
    void operator=(String& value) {
        addStr(value);
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

    // добавить int
    string& addInt(AnyText key, const AnyValue& value) {
        addKey(key);
        return addInt(value);
    }

    // добавить int
    string& addInt(AnyValue value) {
        value.toString(s);
        return comma();
    }

    // добавить int
    void operator=(const int8_t& value) {
        addInt(value);
    }
    void operator=(const uint8_t& value) {
        addInt(value);
    }
    void operator=(const int16_t& value) {
        addInt(value);
    }
    void operator=(const uint16_t& value) {
        addInt(value);
    }
    void operator=(const int32_t& value) {
        addInt(value);
    }
    void operator=(const uint32_t& value) {
        addInt(value);
    }
    void operator=(const int64_t& value) {
        addInt(value);
    }
    void operator=(const uint64_t& value) {
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
            value.toString(s);
        }
    }
};

}  // namespace gson