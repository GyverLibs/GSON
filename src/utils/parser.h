#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entries.h"
#include "entry.h"
#include "types.h"

namespace gsutil {

// ================== DOC CORE ==================
template <int16_t capacity = -1>
class ParserCore {
   private:
    enum class State : uint8_t {
        Idle,
        WaitKey,
        WaitValue,
        WaitColon,
    };

   public:
    ParserCore(uint16_t reserve = 0) : entries(reserve) {}

    // зарезервировать память для ускорения парсинга
    bool reserve(uint16_t cap) {
        return entries.reserve(cap);
    }

    // освободить память
    void destroy() {
        entries.destroy();
        str = nullptr;
        p = nullptr;
        error = gson::Error::None;
    }

    // получить количество элементов
    uint16_t length() {
        return str ? entries.length() : 0;
    }

    // получить размер документа в оперативной памяти (байт)
    uint16_t size() {
        return length() * sizeof(gsutil::Entry_t);
    }

    // установить максимальную глубину вложенности парсинга (умолч. 16)
    void setMaxDepth(uint8_t maxdepth) {
        depth = maxdepth;
    }

    // хешировать ключи всех элементов (операция необратима)
    void hashKeys() {
        if (str) entries.hashKeys(str);
    }

    // проверка были ли хешированы ключи
    bool hashed() {
        return entries.hashed();
    }

    // ===================== BY KEY =====================

    // доступ по ключу (главный контейнер - Object)
    gson::Entry get(const su::Text& key) {
        if (length() && entries[0].isObject()) {
            for (uint16_t i = 1; i < length(); i++) {
                if (entries[i].parent == 0 && entries[i].key_offs && key.compare(entries[i].keyText(str))) {
                    return gson::Entry(&entries, i, str);
                }
            }
        }
        return gson::Entry(nullptr, 0, str);
    }
    gson::Entry operator[](const su::Text& key) {
        return get(key);
    }

    // ===================== BY HASH =====================

    // доступ по хэшу ключа (главный контейнер - Object)
    gson::Entry get(size_t hash) {
#ifndef GSON_NO_HASH
        if (length() && entries[0].is(gson::Type::Object) && entries.hashed()) {
            for (uint16_t i = 1; i < length(); i++) {
                if (entries[i].parent == 0 && entries[i].key_hash == hash) {
                    return gson::Entry(&entries, i, str);
                }
            }
        }
#endif
        return gson::Entry(nullptr, 0, str);
    }
    gson::Entry operator[](size_t hash) {
        return get(hash);
    }

    // ===================== BY INDEX =====================

    // доступ по индексу в главный контейнер - Array или Object
    gson::Entry get(int index) {
        if (length() && (uint16_t)index < length() && entries[0].isContainer()) {
            for (uint16_t i = 1; i < length(); i++) {
                if (entries[i].parent == 0) {
                    if (!index) return gson::Entry(&entries, i, str);
                    index--;
                }
            }
        }
        return gson::Entry(nullptr, 0, str);
    }
    gson::Entry operator[](int index) {
        return get(index);
    }

    // ===================== MISC =====================

    // прочитать ключ по индексу
    su::Text key(int idx) {
        return (length() && (uint16_t)idx < length()) ? entries[idx].keyText(str) : "";
    }

    // прочитать хэш ключа по индексу
    size_t keyHash(int idx) {
#ifndef GSON_NO_HASH
        return (length() && (uint16_t)idx < length()) ? (entries.hashed() ? entries[idx].key_hash : entries[idx].keyText(str).hash()) : 0;
#else
        return 0;
#endif
    }

    // прочитать значение по индексу
    su::Text value(int idx) {
        return (length() && (uint16_t)idx < length()) ? entries[idx].valueText(str) : "";
    }

    // прочитать родителя по индексу
    gson::parent_t parent(int idx) {
        return ((uint16_t)idx < length()) ? entries[idx].parent : 0;
    }

    // получить тип по индексу
    gson::Type type(int idx) {
        return ((uint16_t)idx < length()) ? entries[idx].type : gson::Type::None;
    }

    // прочитать тип по индексу
    const __FlashStringHelper* readType(int index) {
        switch (type(index)) {
            case gson::Type::Object:
                return F("Object");
            case gson::Type::Array:
                return F("Array");
            case gson::Type::String:
                return F("String");
            case gson::Type::Int:
                return F("Int");
            case gson::Type::Float:
                return F("Float");
            case gson::Type::Bool:
                return F("Bool");
            default:
                return F("None");
        }
    }

    // ============ PARSE ============
    // парсить. Вернёт true при успешном парсинге
    bool parse(const su::Text& json) {
        if (json.pgm()) return 0;
        return _startParse(json);
    }
    bool parse(const char* json, uint16_t len) {
        return _startParse(su::Text(json, len));
    }

    // вывести в Print с форматированием
    void stringify(Print& pr) {
        if (length()) gson::Entry(&entries, 0, str).stringify(pr);
    }

    // ============ ERROR ============
    // есть ошибка парсинга
    bool hasError() {
        return error != gson::Error::None;
    }

    // получить ошибку
    gson::Error getError() {
        return error;
    }

    // индекс места ошибки в строке
    uint16_t errorIndex() {
        return (str && p) ? (p - str) : 0;
    }

    // прочитать ошибку
    const __FlashStringHelper* readError() {
        switch (error) {
            case gson::Error::Alloc:
                return F("Alloc");
            case gson::Error::TooDeep:
                return F("TooDeep");
            case gson::Error::NoParent:
                return F("NoParent");
            case gson::Error::NotContainer:
                return F("NotContainer");
            case gson::Error::UnexComma:
                return F("UnexComma");
            case gson::Error::UnexColon:
                return F("UnexColon");
            case gson::Error::UnexToken:
                return F("UnexToken");
            case gson::Error::UnexQuotes:
                return F("UnexQuotes");
            case gson::Error::UnexOpen:
                return F("UnexOpen");
            case gson::Error::UnexClose:
                return F("UnexClose");
            case gson::Error::UnknownToken:
                return F("UnknownToken");
            case gson::Error::BrokenToken:
                return F("BrokenToken");
            case gson::Error::BrokenString:
                return F("BrokenString");
            case gson::Error::BrokenContainer:
                return F("BrokenContainer");
            case gson::Error::EmptyKey:
                return F("EmptyKey");
            case gson::Error::IndexOverflow:
                return F("IndexOverflow");
            case gson::Error::LongPacket:
                return F("LongPacket");
            case gson::Error::LongKey:
                return F("LongKey");
            case gson::Error::EmptyString:
                return F("EmptyString");
            default:
                return F("None");
        }
    }

    // ============ PRIVATE ============
   private:
    gsutil::Entries<capacity> entries;
    const char* str = nullptr;
    char* p = nullptr;
    gson::Error error = gson::Error::None;

    State state = State::Idle;
    bool readStr = 0;
    gsutil::Entry_t buf;
    uint8_t depth = 16;
    const char* end = 0;

    bool _startParse(const su::Text& json) {
        if (!json.length()) {
            error = gson::Error::EmptyString;
            return 0;
        }
        if (json.length() >= (uint32_t)GSON_MAX_LEN) {
            error = gson::Error::LongPacket;
            return 0;
        }

        str = json.str();
        end = str + json.length();
        p = (char*)str;
        state = State::Idle;
        readStr = 0;
        buf = gsutil::Entry_t();
        entries.clear();

        if (p[0] == '{' || p[0] == '[') {
            error = _parse(0);
            entries[0].parent = GSON_MAX_INDEX;
        } else {
            error = gson::Error::NotContainer;
        }
        return !hasError();
    }

    gson::Error _parse(gson::parent_t parent) {
        while (p && p < end && *p) {
            switch (*p) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;

                case ',':
                    if (state != State::Idle) return gson::Error::UnexComma;
                    state = (entries[parent].isArray()) ? State::WaitValue : State::WaitKey;
                    break;

                case ':':
                    if (entries[parent].isObject() && state == State::WaitColon) state = State::WaitValue;
                    else return gson::Error::UnexColon;
                    break;

                case '\"':
                    switch ((gson::Type)entries[parent].type) {
                        case gson::Type::Array:
                            switch (state) {
                                case State::Idle:
                                case State::WaitValue:
                                    readStr = 1;
                                    break;

                                default:
                                    return gson::Error::UnexQuotes;
                            }
                            break;

                        case gson::Type::Object:
                            switch (state) {
                                case State::WaitKey:
                                case State::Idle:
                                    p++;
                                    if (*p == '\"' || p >= end) return gson::Error::EmptyKey;
                                    buf.key_offs = p - str;
                                    while (1) {
                                        p = (char*)memchr((void*)(p + 1), '\"', end - p - 1);
                                        if (!p) return gson::Error::BrokenString;
                                        if (p[-1] != '\\') break;
                                    }
                                    if (p - buf.key(str) > GSON_MAX_KEY_LEN) return gson::Error::LongKey;
                                    buf.key_len = p - buf.key(str);
                                    state = State::WaitColon;
                                    break;

                                case State::WaitValue:
                                    readStr = 1;
                                    break;

                                default:
                                    return gson::Error::UnexQuotes;
                            }
                            break;

                        default:
                            return gson::Error::UnexQuotes;
                    }
                    break;

                case '{':
                case '[': {
                    if (p != str) {  // not first symb
                        if (!(entries[parent].isArray() && (state == State::Idle || state == State::WaitValue)) &&
                            !(entries[parent].isObject() && state == State::WaitValue)) {
                            return gson::Error::UnexOpen;
                        }
                    }
                    if (length() == GSON_MAX_INDEX - 1) return gson::Error::IndexOverflow;

                    buf.type = (*p == '{') ? gson::Type::Object : gson::Type::Array;
                    buf.parent = parent;
                    if (!entries.push(buf)) return gson::Error::Alloc;
                    buf.reset();
                    state = State::Idle;
                    p++;
                    if (p >= end) return gson::Error::BrokenContainer;
                    if (depth - 1 == 0) return gson::Error::TooDeep;

                    depth--;
                    error = _parse(length() - 1);  // RECURSIVE
                    depth++;
                    if (hasError()) return error;
                } break;

                case '}':
                case ']': {
                    if (state != State::Idle || entries[parent].type != ((*p == '}') ? gson::Type::Object : gson::Type::Array)) {
                        return gson::Error::UnexClose;
                    }
                    return gson::Error::None;
                } break;

                default: {
                    if (!(entries[parent].isObject() && state == State::WaitValue) &&
                        !(entries[parent].isArray() && (state == State::WaitValue || state == State::Idle))) {
                        return gson::Error::UnexToken;
                    }

                    buf.val_offs = p - str;
                    switch (*p) {
                        case 't':
                        case 'f':
                            buf.type = gson::Type::Bool;
                            break;
                        case '-':
                        case '0' ... '9':
                            buf.type = gson::Type::Int;
                            break;
                        default:
                            return gson::Error::UnknownToken;
                    }
                    while (true) {
                        if (*p == '.') {
                            if (buf.is(gson::Type::Int)) buf.type = gson::Type::Float;
                            else return gson::Error::UnknownToken;
                        }
                        if (p + 1 >= end || !p[1]) return gson::Error::BrokenToken;

                        bool end = 0;
                        switch (p[1]) {  // next sym
                            case ' ':
                            case '\t':
                            case '\r':
                            case '\n':
                            case ',':
                            case '}':
                            case ']':
                                end = 1;
                        }
                        if (!buf.val_len && (end || !*p)) {
                            buf.val_len = p + 1 - buf.value(str);
                        }
                        if (end) break;
                        p++;
                    }
                    if (buf.is(gson::Type::Bool)) {
                        if (!(buf.val_len == 4 && !strncmp_P(buf.value(str), PSTR("true"), 4)) &&
                            !(buf.val_len == 5 && !strncmp_P(buf.value(str), PSTR("false"), 5))) {
                            return gson::Error::BrokenToken;
                        }
                    }
                    if (length() == GSON_MAX_INDEX - 1) return gson::Error::IndexOverflow;
                    buf.parent = parent;
                    if (!entries.push(buf)) return gson::Error::Alloc;
                    buf.reset();
                    state = State::Idle;
                } break;

            }  // switch

            if (readStr) {
                readStr = 0;
                p++;
                if (p >= end) return gson::Error::BrokenString;
                if (*p == '\"') {
                    buf.val_offs = 0;
                } else {
                    buf.val_offs = p - str;
                    while (1) {
                        p = (char*)memchr((void*)(p + 1), '\"', end - p - 1);
                        if (!p) return gson::Error::BrokenString;
                        if (p[-1] != '\\') break;
                    }
                }
                if (length() == GSON_MAX_INDEX - 1) return gson::Error::IndexOverflow;
                buf.val_len = p - buf.value(str);
                buf.parent = parent;
                buf.type = gson::Type::String;
                if (!entries.push(buf)) return gson::Error::Alloc;
                buf.reset();
                state = State::Idle;
            }

            if (p) p++;
        }  // while

        return (parent == 0) ? gson::Error::None : gson::Error::BrokenContainer;
    }
};

}  // namespace gsutil

namespace gson {

// ================== PARSER DYNAMIC ==================
class Parser : public gsutil::ParserCore<-1> {
   public:
    Parser(uint16_t reserve = 0) : gsutil::ParserCore<-1>(reserve) {}
};

// ================== PARSER STATIC ==================
template <uint16_t capacity>
class ParserStatic : public gsutil::ParserCore<capacity> {};

// ================== DEPRECATED ==================
class Doc : public gsutil::ParserCore<-1> {
   public:
    Doc(uint16_t reserve = 0) : gsutil::ParserCore<-1>(reserve) {}
};

template <uint16_t capacity>
class DocStatic : public gsutil::ParserCore<capacity> {};

}  // namespace gson