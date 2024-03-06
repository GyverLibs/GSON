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

    // получить количество элементов
    uint16_t length() {
        return str ? entries.length() : 0;
    }

    // получить размер документа в оперативной памяти (байт)
    uint16_t size() {
        return length() * sizeof(gsutil::Entry_t);
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
    gson::Entry get(const sutil::AnyText& key) {
        if (length() && entries[0].isObject()) {
            for (uint16_t i = 1; i < length(); i++) {
                if (entries[i].parent == 0 && entries[i].key_offs && key.compare(entries[i].keyAT(str))) {
                    return gson::Entry(&entries, i, str);
                }
            }
        }
        return gson::Entry(nullptr, 0, str);
    }
    gson::Entry operator[](const sutil::AnyText& key) {
        return get(key);
    }

    // ===================== BY HASH =====================

    // доступ по хэшу ключа (главный контейнер - Object)
    gson::Entry get(const size_t& hash) {
#ifndef GSON_NO_HASH
        if (length() && entries[0].type == gson::Type::Object && entries.hashed()) {
            for (uint16_t i = 1; i < length(); i++) {
                if (entries[i].parent == 0 && entries[i].key_hash == hash) {
                    return gson::Entry(&entries, i, str);
                }
            }
        }
#endif
        return gson::Entry(nullptr, 0, str);
    }
    gson::Entry operator[](const size_t& hash) {
        return get(hash);
    }

    // ===================== BY INDEX =====================

    // доступ по индексу в главный контейнер - Array или Object
    gson::Entry get(uint16_t index) {
        if (length() && index < length() && entries[0].isContainer()) {
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
        return get((uint16_t)index);
    }

    // ===================== MISC =====================

    // прочитать ключ по индексу
    sutil::AnyText key(uint16_t idx) {
        return (length() && idx < length()) ? entries[idx].keyAT(str) : "";
    }

    // прочитать хэш ключа по индексу
    size_t keyHash(uint16_t idx) {
#ifndef GSON_NO_HASH
        return (length() && idx < length()) ? (entries.hashed() ? entries[idx].key_hash : entries[idx].keyAT(str).hash()) : 0;
#else
        return 0;
#endif
    }

    // прочитать значение по индексу
    sutil::AnyText value(uint16_t idx) {
        return (length() && idx < length()) ? entries[idx].valueAT(str) : "";
    }

    // прочитать родителя по индексу
    gson::parent_t parent(uint16_t idx) {
        return (idx < length()) ? entries[idx].parent : 0;
    }

    // получить тип по индексу
    gson::Type type(uint16_t idx) {
        return (idx < length()) ? entries[idx].type : gson::Type::None;
    }

    // прочитать тип по индексу
    const __FlashStringHelper* readType(uint16_t index) {
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
    // парсить. Вернёт true при успешном парсинге. Можно указать макс. уровень вложенности
    bool parse(String& json, uint8_t maxdepth = 16) {
        return parse(json.c_str(), maxdepth);
    }

    // парсить. Вернёт true при успешном парсинге. Можно указать макс. уровень вложенности
    bool parse(const char* json, uint8_t maxdepth = 16) {
        if (!json || !json[0]) {
            error = gson::Error::EmptyString;
            return 0;
        }
        str = json;
        p = (char*)json;
        state = State::Idle;
        readStr = 0;
        buf = gsutil::Entry_t();
        entries.clear();
        depth = maxdepth;

        if (p[0] == '{' || p[0] == '[') {
            error = _parse(0);
            entries[0].parent = GSON_MAX_INDEX;
        } else {
            error = gson::Error::NotContainer;
        }
        return !hasError();
    }

    // вывести в Print с форматированием
    void stringify(Print* p) {
        if (!length()) return;
        gson::parent_t idx = 0;
        uint8_t dep = 0;
        _stringify(*p, idx, GSON_MAX_INDEX, dep);
        p->println();
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
        return str ? (p - str) : 0;
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
    uint8_t depth = 0;

    void printT(Print& p, int16_t amount) {
        while (amount--) {
            p.print(' ');
            p.print(' ');
        }
    }

    void _stringify(Print& p, gson::parent_t& idx, gson::parent_t parent, uint8_t& dep) {
        bool first = true;
        while (idx < length()) {
            gsutil::Entry_t ent = entries[idx];
            if (ent.parent != parent) return;
            if (first) first = false;
            else p.print(",\n");

            if (ent.isContainer()) {
                printT(p, dep);
                if (ent.key_offs) {
                    p.print('\"');
                    p.print(ent.keyAT(str));
                    p.print("\": ");
                }
                p.print((ent.isArray()) ? '[' : '{');
                p.print('\n');
                int16_t par = idx;
                idx++;
                dep++;
                _stringify(p, idx, par, dep);
                dep--;
                p.print('\n');
                printT(p, dep);
                p.print((ent.isArray()) ? ']' : '}');
            } else {
                printT(p, dep);
                if (ent.key_offs) {
                    p.print('\"');
                    p.print(ent.keyAT(str));
                    p.print("\":");
                }
                if (ent.type == gson::Type::String) p.print('\"');
                switch (ent.type) {
                    case gson::Type::String:
                    case gson::Type::Int:
                    case gson::Type::Float:
                        p.print(ent.valueAT(str));
                        break;
                    case gson::Type::Bool:
                        p.print((*(ent.value(str)) == 't') ? F("true") : F("false"));
                        break;
                    default:
                        break;
                }
                if (ent.type == gson::Type::String) p.print('\"');
                idx++;
            }
        }
    }

    gson::Error _parse(gson::parent_t parent) {
        while (*p) {
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
                    switch (entries[parent].type) {
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
                                    if (*p == '\"') return gson::Error::EmptyKey;
                                    buf.key_offs = p - str;
                                    while (1) {
                                        p = strchr(p + 1, '\"');
                                        if (p[-1] != '\\') break;
                                        if (!p) return gson::Error::BrokenString;
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
                            if (buf.type == gson::Type::Int) buf.type = gson::Type::Float;
                            else return gson::Error::UnknownToken;
                        }
                        if (!p[1]) return gson::Error::BrokenToken;

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
                    if (buf.type == gson::Type::Bool) {
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
                if (*p == '\"') {
                    buf.val_offs = 0;
                } else {
                    buf.val_offs = p - str;
                    while (1) {
                        p = strchr(p + 1, '\"');
                        if (p[-1] != '\\') break;
                        if (!p) return gson::Error::BrokenString;
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

            p++;
            if (p - str >= GSON_MAX_LEN) return gson::Error::LongPacket;
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