#pragma once
#include <Arduino.h>
#include <StringUtils.h>

namespace gson {

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
        comma();
        return *this;
    }

    // прибавить gson::string. Будет добавлена запятая
    void operator+=(string& str) {
        add(str);
    }

    // =============== KEY ===============

    // добавить ключ (строка любого типа)
    string& addKey(const sutil::AnyText& key) {
        if (key.valid()) {
            _addStr(key, false);
            colon();
        }
        return *this;
    }

    // добавить ключ (строка любого типа)
    string& operator[](const sutil::AnyText& key) {
        return addKey(key);
    }

    // =============== VALUE ===============

    // прибавить текст (строка любого типа) без запятой и кавычек
    string& addRaw(const sutil::AnyText& str, bool escape = true) {
        _addRaw(str, escape);
        return *this;
    }

    // добавить строку (строка любого типа)
    string& addString(const sutil::AnyText& key, const sutil::AnyText& value, bool escape = true) {
        addKey(key);
        return addString(value, escape);
    }

    // добавить строку (строка любого типа)
    string& addString(const sutil::AnyText& value, bool escape = true) {
        _addStr(value, escape);
        comma();
        return *this;
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
    string& addBool(const sutil::AnyText& key, const bool& value) {
        addKey(key);
        return addBool(value);
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

    // добавить float
    string& addFloat(const sutil::AnyText& key, const double& value, uint8_t dec = 2) {
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

    // добавить int
    string& addInt(const sutil::AnyText& key, const sutil::AnyValue& value) {
        addKey(key);
        value.addString(s);
        comma();
        return *this;
    }

    // добавить int
    string& addInt(const sutil::AnyValue& value) {
        value.addString(s);
        comma();
        return *this;
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
    // вызывается после добавления значения, но перед запятой
    virtual void afterValue() {}

   private:
    void _replaceComma(const char& sym) {
        int16_t len = s.length() - 1;
        if (s[len] == ',') {
            if (!sym) {
                s.remove(len);
                return;
            }
            s[len] = sym;
        } else {
            s += sym;
        }
    }
    void _addStr(const sutil::AnyText& value, const bool& esc) {
        quotes();
        _addRaw(value, esc);
        quotes();
    }
    void _addRaw(const sutil::AnyText& value, const bool& esc) {
        if (!esc) {
            value.addString(s);
        } else {
            uint16_t len = value.length();
            if (!s.reserve(s.length() + len)) return;
            char p = 0;
            for (uint16_t i = 0; i < len; i++) {
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
        }
    }
};

}  // namespace gson