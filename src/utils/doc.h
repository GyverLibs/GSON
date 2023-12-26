#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entries.h"
#include "entry.h"
#include "types.h"

namespace gsutil {

using gson::Entry;
using gson::Error;
using gson::parent_t;
using gson::Type;
using gsutil::Entry_t;
using sutil::AnyText;

// ================== DOC CORE ==================
template <int16_t capacity = -1>
class DocCore {
   private:
    enum class State : uint8_t {
        Idle,
        WaitKey,
        WaitValue,
        WaitColon,
    };

   public:
    DocCore(uint16_t reserve = 0) : entries(reserve) {}

    // получить количество элементов
    uint16_t length() {
        return entries.length();
    }

    // получить размер документа в оперативной памяти (байт)
    uint16_t size() {
        return entries.length() * sizeof(Entry_t);
    }

    // хешировать ключи всех элементов (операция необратима)
    void hashKeys() {
        entries.hashKeys();
    }

    // проверка были ли хешированы ключи
    bool hashed() {
        return entries.hashed();
    }

    // ===================== BY KEY =====================

    // доступ по ключу (главный контейнер - Object)
    Entry get(AnyText key) {
        if (entries[0].type == Type::Object && !entries.hashed()) {
            for (uint16_t i = 1; i < entries.length(); i++) {
                if (entries[i].parent == 0 && entries[i].key.str && key.compare(entries[i].key.str)) {
                    return Entry(&entries, i);
                }
            }
        }
        return Entry(nullptr, 0);
    }
    Entry operator[](AnyText key) {
        return get(key);
    }

    // ===================== BY HASH =====================

    // доступ по хэшу ключа (главный контейнер - Object)
    Entry get(const size_t& hash) {
        if (entries[0].type == Type::Object && entries.hashed()) {
            for (uint16_t i = 1; i < entries.length(); i++) {
                if (entries[i].parent == 0 && entries[i].key.hash == hash) {
                    return Entry(&entries, i);
                }
            }
        }
        return Entry(nullptr, 0);
    }
    Entry operator[](const size_t& hash) {
        return get(hash);
    }

    // ===================== BY INDEX =====================

    // доступ по индексу в главный контейнер - Array или Object
    Entry get(int index) {
        if (entries[0].type == Type::Object || entries[0].type == Type::Array) {
            for (uint16_t i = 1; i < entries.length(); i++) {
                if (entries[i].parent == 0) {
                    if (!index) return Entry(&entries, i);
                    index--;
                }
            }
        }
        return Entry(nullptr, 0);
    }
    Entry operator[](int index) {
        return get(index);
    }

    // ===================== MISC =====================

    // прочитать ключ по индексу
    AnyText key(uint16_t idx) {
        return (idx < length() && entries[idx].key.str && !entries.hashed()) ? entries[idx].key.str : "";
    }

    // прочитать хэш ключа по индексу
    size_t keyHash(uint16_t idx) {
        return (idx < length()) ? (entries.hashed() ? entries[idx].key.hash : sutil::hash(entries[idx].key.str)) : 0;
    }

    // прочитать значение по индексу
    AnyText value(uint16_t idx) {
        return (idx < length() && entries[idx].value) ? entries[idx].value : "";
    }

    // прочитать родителя по индексу
    parent_t parent(uint16_t idx) {
        return (idx < length()) ? entries[idx].parent : 0;
    }

    // получить тип по индексу
    gson::Type type(uint16_t idx) {
        return (idx < length()) ? entries[idx].type : Type::None;
    }

    // прочитать тип по индексу
    const __FlashStringHelper* readType(int index) {
        switch (type(index)) {
            case Type::Object:
                return F("Object");
            case Type::Array:
                return F("Array");
            case Type::String:
                return F("String");
            case Type::Int:
                return F("Int");
            case Type::Float:
                return F("Float");
            case Type::Bool:
                return F("Bool");
            default:
                return F("None");
        }
    }

    // ============ PARSE ============
    // парсить. Вернёт true при успешном парсинге. Можно указать макс. уровень вложенности
    bool parse(String& json, uint8_t maxdepth = 16) {
        return parse(json.c_str());
    }

    // парсить. Вернёт true при успешном парсинге. Можно указать макс. уровень вложенности
    bool parse(const char* json, uint8_t maxdepth = 16) {
        str = json;
        p = (char*)json;
        state = State::Idle;
        clearComma = 0;
        readStr = 0;
        buf = Entry_t();
        entries.clear();
        depth = maxdepth;

        if (p[0] == '{' || p[0] == '[') {
            error = _parse(0);
            entries[0].parent = GSON_MAX_INDEX;
        } else error = Error::NotContainer;
        return !hasError();
    }

    // вывести в Print с форматированием
    void stringify(Print* p) {
        parent_t idx = 0;
        uint8_t dep = 0;
        _stringify(*p, idx, GSON_MAX_INDEX, dep);
        p->println();
    }

    // ============ ERROR ============
    // есть ошибка парсинга
    bool hasError() {
        return error != Error::None;
    }

    // получить ошибку
    Error getError() {
        return error;
    }

    // индекс места ошибки в строке
    uint16_t errorIndex() {
        return p - str;
    }

    // прочитать ошибку
    const __FlashStringHelper* readError() {
        switch (error) {
            case Error::Alloc:
                return F("Alloc");
            case Error::TooDeep:
                return F("TooDeep");
            case Error::NoParent:
                return F("NoParent");
            case Error::NotContainer:
                return F("NotContainer");
            case Error::UnexComma:
                return F("UnexComma");
            case Error::UnexColon:
                return F("UnexColon");
            case Error::UnexToken:
                return F("UnexToken");
            case Error::UnexQuotes:
                return F("UnexQuotes");
            case Error::UnexOpen:
                return F("UnexOpen");
            case Error::UnexClose:
                return F("UnexClose");
            case Error::UnknownToken:
                return F("UnknownToken");
            case Error::BrokenToken:
                return F("BrokenToken");
            case Error::BrokenString:
                return F("BrokenString");
            case Error::BrokenContainer:
                return F("BrokenContainer");
            case Error::EmptyKey:
                return F("EmptyKey");
            case Error::IndexOverflow:
                return F("IndexOverflow");
            default:
                return F("None");
        }
    }

    // ============ PRIVATE ============
   private:
    gsutil::Entries<capacity> entries;
    const char* str = nullptr;
    char* p = nullptr;
    Error error = Error::None;

    State state = State::Idle;
    bool clearComma = 0;
    bool readStr = 0;
    Entry_t buf;
    uint8_t depth = 0;

    void printT(Print& p, int16_t amount) {
        while (amount--) {
            p.print(' ');
            p.print(' ');
        }
    }

    void _stringify(Print& p, parent_t& idx, parent_t parent, uint8_t& dep) {
        bool first = true;
        while (idx < entries.length()) {
            Entry_t ent = entries[idx];
            if (ent.parent != parent) return;
            if (first) first = false;
            else p.print(",\n");

            if (ent.type == Type::Array || ent.type == Type::Object) {
                printT(p, dep);
                if (ent.key.str) {
                    p.print('\"');
                    p.print(ent.key.str);
                    p.print("\": ");
                }
                p.print((ent.type == Type::Array) ? '[' : '{');
                p.print('\n');
                int16_t par = idx;
                idx++;
                dep++;
                _stringify(p, idx, par, dep);
                dep--;
                p.print('\n');
                printT(p, dep);
                p.print((ent.type == Type::Array) ? ']' : '}');
            } else {
                printT(p, dep);
                if (ent.key.str) {
                    p.print('\"');
                    p.print(ent.key.str);
                    p.print("\":");
                }
                if (ent.type == Type::String) p.print('\"');
                switch (ent.type) {
                    case Type::String:
                    case Type::Int:
                    case Type::Float:
                        p.print(ent.value);
                        break;
                    case Type::Bool:
                        p.print((ent.value[0] == 't') ? F("true") : F("false"));
                        break;
                    default:
                        break;
                }
                if (ent.type == Type::String) p.print('\"');
                idx++;
            }
        }
    }

    Error _parse(parent_t parent) {
        while (*p) {
            switch (*p) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;

                case ',':
                    if (state != State::Idle) return Error::UnexComma;
                    state = (entries[parent].type == Type::Array) ? State::WaitValue : State::WaitKey;
                    if (clearComma) {
                        clearComma = 0;
                        *p = 0;
                    }
                    break;

                case ':':
                    if (entries[parent].type == Type::Object && state == State::WaitColon) state = State::WaitValue;
                    else return Error::UnexColon;
                    break;

                case '\"':
                    switch (entries[parent].type) {
                        case Type::Array:
                            switch (state) {
                                case State::Idle:
                                case State::WaitValue:
                                    readStr = 1;
                                    break;

                                default:
                                    return Error::UnexQuotes;
                            }
                            break;

                        case Type::Object:
                            switch (state) {
                                case State::WaitKey:
                                case State::Idle:
                                    p++;
                                    if (*p == '\"') return Error::EmptyKey;
                                    buf.key.str = p;
                                    while (1) {
                                        p = strchr(p + 1, '\"');
                                        if (p[-1] != '\\') break;
                                        if (!p) return Error::BrokenString;
                                    }
                                    *p = 0;
                                    state = State::WaitColon;
                                    break;

                                case State::WaitValue:
                                    readStr = 1;
                                    break;

                                default:
                                    return Error::UnexQuotes;
                            }
                            break;

                        default:
                            return Error::UnexQuotes;
                    }
                    break;

                case '{':
                case '[': {
                    if (p != str) { // not first symb
                        if (!(entries[parent].type == Type::Array && (state == State::Idle || state == State::WaitValue)) &&
                            !(entries[parent].type == Type::Object && state == State::WaitValue)) {
                            return Error::UnexOpen;
                        }
                    }

                    Entry_t entry = Entry_t{buf.key.str, nullptr, parent, (*p == '{') ? Type::Object : Type::Array};
                    if (entries.length() == GSON_MAX_INDEX - 1) return Error::IndexOverflow;
                    if (!entries.push(entry)) return Error::Alloc;

                    buf.key.str = buf.value = nullptr;
                    state = State::Idle;
                    p++;

                    if (depth - 1 == 0) return Error::TooDeep;
                    depth--;
                    error = _parse(entries.length() - 1);
                    depth++;
                    if (hasError()) return error;
                } break;

                case '}':
                case ']': {
                    if (state != State::Idle || entries[parent].type != ((*p == '}') ? Type::Object : Type::Array)) {
                        return Error::UnexClose;
                    }
                    *p = 0;
                    return Error::None;
                } break;

                default: {
                    if (!(entries[parent].type == Type::Object && state == State::WaitValue) &&
                        !(entries[parent].type == Type::Array && (state == State::WaitValue || state == State::Idle))) {
                        return Error::UnexToken;
                    }

                    buf.value = p;
                    switch (*p) {
                        case 't':
                        case 'f':
                            buf.type = Type::Bool;
                            break;
                        case '-':
                        case '1' ... '9':
                            buf.type = Type::Int;
                            break;
                        default:
                            return Error::UnknownToken;
                    }
                    while (true) {
                        if (*p == '.') {
                            if (buf.type == Type::Int) buf.type = Type::Float;
                            else return Error::UnknownToken;
                        }
                        if (!p[1]) return Error::BrokenToken;

                        bool end = 0;
                        switch (p[1]) {  // next sym
                            case ' ':
                            case '\t':
                            case '\r':
                            case '\n':
                                *p = 0;
                            case ',':
                                clearComma = 1;
                            case '}':
                            case ']':
                                end = 1;
                        }
                        if (end) break;
                        p++;
                    }
                    if (buf.type == Type::Bool) {
                        uint16_t len = p - buf.value + 1;
                        if (!(len == 4 && !strncmp_P(buf.value, PSTR("true"), 4)) &&
                            !(len == 5 && !strncmp_P(buf.value, PSTR("false"), 5))) {
                            return Error::BrokenToken;
                        }
                    }
                    Entry_t entry = Entry_t{buf.key.str, buf.value, parent, buf.type};
                    if (entries.length() == GSON_MAX_INDEX - 1) return Error::IndexOverflow;
                    if (!entries.push(entry)) return Error::Alloc;
                    buf.key.str = buf.value = nullptr;
                    state = State::Idle;
                } break;

            }  // switch

            if (readStr) {
                readStr = 0;
                p++;
                if (*p == '\"') {
                    buf.value = nullptr;
                } else {
                    buf.value = p;
                    while (1) {
                        p = strchr(p + 1, '\"');
                        if (p[-1] != '\\') break;
                        if (!p) return Error::BrokenString;
                    }
                }
                *p = 0;
                Entry_t entry = Entry_t{buf.key.str, buf.value, parent, Type::String};
                if (entries.length() == GSON_MAX_INDEX - 1) return Error::IndexOverflow;
                if (!entries.push(entry)) return Error::Alloc;
                buf.key.str = buf.value = nullptr;
                state = State::Idle;
            }

            p++;
        }  // while

        return (parent == 0) ? Error::None : Error::BrokenContainer;
    }
};

}  // namespace gsutil

namespace gson {

// ================== DOC DYNAMIC ==================
class Doc : public gsutil::DocCore<-1> {
   public:
    Doc(uint16_t reserve = 0) : gsutil::DocCore<-1>(reserve) {}
};

// ================== DOC STATIC ==================
template <uint16_t capacity>
class DocStatic : public gsutil::DocCore<capacity> {};

}  // namespace gson