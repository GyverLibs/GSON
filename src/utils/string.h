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
    operator Text() {
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
    uint16_t length() const {
        return s.length();
    }

    // зарезервировать строку
    bool reserve(uint16_t res) {
        return s.reserve(res);
    }

    // завершить пакет
    string& end() {
        replaceComma('\0');
        return *this;
    }

    // =============== ADD ===============

    // прибавить gson::string. Будет добавлена запятая
    string& add(const string& str) {
        s += str.s;
        replaceComma(',');
        return *this;
    }

    // прибавить gson::string. Будет добавлена запятая
    void operator+=(const string& str) {
        add(str);
    }

    // =============== KEY ===============

    // добавить ключ (строка любого типа)
    string& addKey(const Text& key) {
        if (key.valid()) {
            _addRaw(key, true, false);
            colon();
        }
        return *this;
    }

    // добавить ключ (строка любого типа)
    string& operator[](const Text& key) {
        return addKey(key);
    }

    // =============== TEXT ===============

    // прибавить текст (строка любого типа) без запятой и кавычек
    string& addTextRaw(const Text& txt) {
        if (txt.valid()) _addRaw(txt, false, false);
        return *this;
    }

    // прибавить текст (строка любого типа) без запятой и кавычек с escape символов
    string& addTextRawEsc(const Text& txt) {
        if (txt.valid()) _addRaw(txt, false, true);
        return *this;
    }

    // прибавить текст (строка любого типа) без кавычек
    string& addText(const Text& txt) {
        if (txt.valid()) {
            addTextRaw(txt);
            comma();
        }
        return *this;
    }

    // прибавить текст (строка любого типа) без кавычек с escape символов
    string& addTextEsc(const Text& txt) {
        if (txt.valid()) {
            addTextRawEsc(txt);
            comma();
        }
        return *this;
    }

    // добавить текст (строка любого типа) без кавычек
    string& addText(const Text& key, const Text& txt) {
        if (key.valid() && txt.valid()) {
            addKey(key);
            addText(txt);
        }
        return *this;
    }

    // добавить текст (строка любого типа) без кавычек с escape символов
    string& addTextEsc(const Text& key, const Text& txt) {
        if (key.valid() && txt.valid()) {
            addKey(key);
            addTextEsc(txt);
        }
        return *this;
    }

    // =============== STRING BIN ===============

    // добавить строку (строка любого типа) с escape символов без запятой
    string& addStringRawEsc(const Text& value) {
        if (value.valid()) _addRaw(value, true, true);
        return *this;
    }

    // добавить строку (строка любого типа) без запятой
    string& addStringRaw(const Text& value) {
        if (value.valid()) _addRaw(value, true, false);
        return *this;
    }

    // =============== STRING ===============

    // добавить строку (строка любого типа) с escape символов
    string& addStringEsc(const Text& key, const Text& value) {
        if (key.valid() && value.valid()) {
            addKey(key);
            addStringEsc(value);
        }
        return *this;
    }

    // добавить строку (строка любого типа) с escape символов
    string& addStringEsc(const Text& value) {
        if (value.valid()) {
            _addRaw(value, true, true);
            comma();
        }
        return *this;
    }

    // добавить строку (строка любого типа)
    string& addString(const Text& key, const Text& value) {
        if (key.valid() && value.valid()) {
            addKey(key);
            addString(value);
        }
        return *this;
    }

    // добавить строку (строка любого типа)
    string& addString(const Text& value) {
        if (value.valid()) {
            _addRaw(value, true, false);
            comma();
        }
        return *this;
    }

    // добавить строку (строка любого типа)
    void operator=(const char* value) {
        _addString(value);
    }
    void operator+=(const char* value) {
        _addString(value);
    }

    void operator=(const __FlashStringHelper* value) {
        _addString(value);
    }
    void operator+=(const __FlashStringHelper* value) {
        _addString(value);
    }

    void operator=(const String& value) {
        _addString(value);
    }
    void operator+=(const String& value) {
        _addString(value);
    }

    void operator=(const Text& value) {
        _addString(value);
    }
    void operator+=(const Text& value) {
        _addString(value);
    }

    // =============== BOOL ===============

    // добавить bool
    string& addBool(const Text& key, const bool& value) {
        if (key.valid()) {
            addKey(key);
            addBool(value);
        }
        return *this;
    }

    // добавить bool
    string& addBool(const bool& value) {
        addBoolRaw(value);
        comma();
        return *this;
    }

    // добавить bool без запятой
    string& addBoolRaw(const bool& value) {
        s += value ? F("true") : F("false");
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
    string& addFloat(const Text& key, const double& value, uint8_t dec = 2) {
        if (key.valid()) {
            addKey(key);
            addFloat(value, dec);
        }
        return *this;
    }

    // добавить float
    string& addFloat(const double& value, uint8_t dec = 2) {
        addFloatRaw(value, dec);
        comma();
        return *this;
    }

    // добавить float без запятой
    string& addFloatRaw(const double& value, uint8_t dec = 2) {
        if (isnan(value)) s += '0';
        else {
            char buf[33];
            dtostrf(value, dec + 2, dec, buf);
            s += buf;
        }
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
    string& addInt(const Text& key, const Value& value) {
        if (key.valid() && value.valid()) {
            addKey(key);
            addInt(value);
        }
        return *this;
    }

    // добавить int
    string& addInt(const Value& value) {
        if (value.valid()) {
            value.addString(s);
            comma();
        }
        return *this;
    }

    // добавить int без запятой
    string& addIntRaw(const Value& value) {
        if (value.valid()) value.addString(s);
        return *this;
    }
#else
    // добавить int
    template <typename T>
    string& addInt(const Text& key, T value) {
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

    // добавить int без запятой
    template <typename T>
    string& addIntRaw(T value) {
        s += value;
        return *this;
    }
#endif

#define GSON_MAKE_ADD_INT(type)                   \
    void operator=(type value) { addInt(value); } \
    void operator+=(type value) { addInt(value); }

    GSON_MAKE_ADD_INT(Value)
    GSON_MAKE_ADD_INT(char)
    GSON_MAKE_ADD_INT(unsigned char)
    GSON_MAKE_ADD_INT(short)
    GSON_MAKE_ADD_INT(unsigned short)
    GSON_MAKE_ADD_INT(int)
    GSON_MAKE_ADD_INT(unsigned int)
    GSON_MAKE_ADD_INT(long)
    GSON_MAKE_ADD_INT(unsigned long)

#ifndef SUTIL_NO_VALUE
    GSON_MAKE_ADD_INT(long long)
    GSON_MAKE_ADD_INT(unsigned long long)
#endif
    // =============== CONTAINER ===============

    // начать объект
    string& beginObj(const Text& key = Text()) {
        addKey(key);
        s += '{';
        return *this;
    }

    // завершить объект. last - не добавлять запятую
    string& endObj(bool last = false) {
        replaceComma('}');
        if (!last) comma();
        return *this;
    }

    // начать массив
    string& beginArr(const Text& key = Text()) {
        addKey(key);
        s += '[';
        return *this;
    }

    // завершить массив. last - не добавлять запятую
    string& endArr(bool last = false) {
        replaceComma(']');
        if (!last) comma();
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

    // делать escape символов при прибавлении через оператор = (умолч. вкл, true)
    void escapeDefault(bool esc) {
        _esc = esc;
    }

    // =============== PRIVATE ===============
   protected:
    // вызывается перед запятой (после добавления значения)
    virtual void afterValue() {}

    // escape символов
    virtual void escape(const Text& text) {
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

    // заменить последнюю запятую символом. Если символ '\0' - удалить запятую. Если это не запятая - добавить символ
    void replaceComma(char sym) {
        int16_t len = s.length() - 1;
        if (s[len] == ',') {
            if (!sym) s.remove(len);
            else s[len] = sym;
        } else {
            if (sym) s += sym;
        }
    }

   private:
    bool _esc = true;

    void _addRaw(const Text& text, bool quot, bool esc) {
        if (quot) quotes();
        if (esc) {
            if (!s.reserve(s.length() + text.length())) return;
            escape(text);
        } else {
            text.addString(s);
        }
        if (quot) quotes();
    }
    void _addString(const Text& text) {
        _addRaw(text, true, _esc);
        comma();
    }
};

}  // namespace gson