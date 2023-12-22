#pragma once
#include <Arduino.h>
#include <StringUtils.h>

namespace GSON {

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

    // зарезервировать строку
    bool reserve(uint16_t res) {
        return s.reserve(res);
    }

    // завершить пакет
    string& end() {
        _replaceComma('\0');
        return *this;
    }

    // прибавить GSON::string. Будет добавлена запятая
    string& add(const string& str) {
        s += str.s;
        return _com();
    }

    // прибавить GSON::string. Будет добавлена запятая
    void operator+=(const string& str) {
        add(str);
    }

    // =============== KEY ===============

    // добавить ключ (строка любого типа)
    string& addKey(const AnyText& key) {
        if (key.valid()) {
            _text(key);
            s += ':';
        }
        return *this;
    }

    // добавить ключ (строка любого типа)
    string& operator[](const AnyText& key) {
        return addKey(key);
    }

    // =============== VALUE ===============

    // прибавить текст (строка любого типа)
    string& addRaw(const AnyText& str, bool esc = false) {
        _addText(str, esc);
        return *this;
    }

    // прибавить текст (строка любого типа)
    void operator+=(const AnyText& str) {
        _addText(str);
    }

    // добавить строку (строка любого типа)
    string& addStr(const AnyText& key, const AnyText& value) {
        addKey(key);
        return addStr(value);
    }

    // добавить строку (строка любого типа)
    string& addStr(const AnyText& value) {
        _text(value, true);
        return _com();
    }

    // добавить строку (строка любого типа)
    void operator=(const AnyText& value) {
        addStr(value);
    }

    // добавить bool
    string& addBool(const AnyText& key, const bool& value) {
        addKey(key);
        return addBool(value);
    }

    // добавить bool
    string& addBool(const bool& value) {
        s += value ? F("true") : F("false");
        return _com();
    }

    // добавить bool
    void operator=(const bool& value) {
        addBool(value);
    }

    // добавить float
    string& addFloat(const AnyText& key, const double& value, uint8_t dec = 2) {
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
        return _com();
    }

    // добавить float
    void operator=(const double& value) {
        addFloat(value);
    }

    // добавить int
    string& addInt(const AnyText& key, const AnyValue& value) {
        addKey(key);
        return addInt(value);
    }

    // добавить int
    string& addInt(const AnyValue& value) {
        value.toString(s);
        return _com();
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
    string& beginObj(const AnyText& key = AnyText()) {
        addKey(key);
        s += '{';
        return *this;
    }

    // завершить объект
    string& endObj() {
        _replaceComma('}');
        return _com();
    }

    // начать массив
    string& beginArr(const AnyText& key = AnyText()) {
        addKey(key);
        s += '[';
        return *this;
    }

    // завершить массив
    string& endArr() {
        _replaceComma(']');
        return _com();
    }

    // =============== PRIVATE ===============
   private:
    void _dq() {
        s += '\"';
    }
    string& _com() {
        s += ',';
        return *this;
    }
    void _replaceComma(char sym) {
        if (s[s.length() - 1] == ',') {
            if (!sym) {
                s.remove(s.length() - 1);
                return;
            }
            s[s.length() - 1] = sym;
        } else s += sym;
    }
    void _text(const AnyText& value, bool esc = false) {
        _dq();
        _addText(value, esc);
        _dq();
    }
    void _addText(const AnyText& value, bool esc = false) {
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

}  // namespace GSON