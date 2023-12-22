#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entry.h"
#include "stack.h"
#include "types.h"

#define GSON_MAX_DEPTH 16

namespace gsutil {

using GSON::Entry;
using GSON::Entry_t;
using GSON::Error;
using GSON::Type;
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
    gsutil::Stack<Entry_t, capacity> entries;

    DocCore(uint16_t reserve = 0) : entries(reserve) {}

    // получить количество записей Entry
    uint16_t length() {
        return entries.length();
    }

    // получить размер Doc в оперативной памяти (байт)
    uint16_t size() {
        return entries.length() * sizeof(Entry_t);
    }

    // ============ READ ============
    // доступ по ключу (главный контейнер - Object)
    Entry get(const AnyText& key) {
        if (entries[0].type == Type::Object) {
            for (uint16_t i = 0; i < entries.length(); i++) {
                if (key.compare(entries[i].key)) return Entry(entries, i);
            }
        }
        return Entry(entries, -1);
    }
    Entry operator[](const AnyText& key) {
        return get(key);
    }

    // доступ по индексу (главный контейнер - Array или Object)
    Entry get(int index) {
        if (entries[0].type == Type::Object || entries[0].type == Type::Array) {
            for (uint16_t i = 0; i < entries.length(); i++) {
                if (entries[i].parent == 0) {
                    if (!index) return Entry(entries, i);
                    index--;
                }
            }
        }
        return Entry(entries, -1);
    }
    Entry operator[](int index) {
        return get(index);
    }

    // прочитать ключ по индексу
    AnyText key(uint16_t idx) {
        return (idx < length() && entries[idx].key) ? entries[idx].key : "";
    }

    // прочитать значение по индексу
    AnyText value(uint16_t idx) {
        return (idx < length() && entries[idx].value) ? entries[idx].value : "";
    }

    // получить тип по индексу
    GSON::Type type(uint16_t idx) {
        return (idx < length()) ? entries[idx].type : Type::None;
    }

    // прочитать тип по индексу
    const __FlashStringHelper* readType(uint16_t index) {
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
    // парсить. Вернёт true при успешном парсинге
    bool parse(String& json) {
        return parse(json.c_str());
    }

    // парсить. Вернёт true при успешном парсинге
    bool parse(const char* json) {
        return parseT<>(json);
    }

    // парсить. Вернёт true при успешном парсинге
    template <uint16_t max_depth = GSON_MAX_DEPTH>
    bool parseT(String& json) {
        return parseT<max_depth>(json.c_str());
    }

    // парсить. Вернёт true при успешном парсинге
    template <uint16_t max_depth = GSON_MAX_DEPTH>
    bool parseT(const char* json) {
        str = json;
        p = (char*)json;
        error = Error::None;
        entries.clear();

        gsutil::Stack<int8_t, max_depth> parents;
        State state = State::Idle;
        bool clearComma = 0;
        bool readStr = 0;
        Entry_t buf;

        if (p[0] == '{' || p[0] == '[') {
            Entry_t entry = {nullptr, nullptr, -1, (p[0] == '{') ? Type::Object : Type::Array};
            if (!entries.push(entry)) return error = Error::Alloc, 0;
            parents.push(0);
            p++;
        } else {
            return error = Error::NotContainer, 0;
        }

        while (*p) {
            switch (*p) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;

                case ',':
                    if (state != State::Idle) return error = Error::UnexComma, 0;
                    state = (entries[parents.peek()].type == Type::Array) ? State::WaitValue : State::WaitKey;
                    if (clearComma) {
                        clearComma = 0;
                        *p = 0;
                    }
                    break;

                case ':':
                    if (entries[parents.peek()].type == Type::Object && state == State::WaitColon) state = State::WaitValue;
                    else return error = Error::UnexColon, 0;
                    break;

                case '\"':
                    switch (entries[parents.peek()].type) {
                        case Type::Array:
                            switch (state) {
                                case State::Idle:
                                case State::WaitValue:
                                    readStr = 1;
                                    break;

                                default:
                                    return error = Error::UnexQuotes, 0;
                            }
                            break;

                        case Type::Object:
                            switch (state) {
                                case State::WaitKey:
                                case State::Idle:
                                    buf.key = p + 1;
                                    if (!skipString()) return error = Error::BrokenString, 0;
                                    *p = 0;
                                    state = State::WaitColon;
                                    break;

                                case State::WaitValue:
                                    readStr = 1;
                                    break;

                                default:
                                    return error = Error::UnexQuotes, 0;
                            }
                            break;

                        default:
                            return error = Error::UnexQuotes, 0;
                    }
                    break;

                case '{':
                case '[': {
                    if (!(entries[parents.peek()].type == Type::Array && (state == State::Idle || state == State::WaitValue)) &&
                        !(entries[parents.peek()].type == Type::Object && state == State::WaitValue)) return error = Error::UnexOpen, 0;
                    Entry_t entry = Entry_t{buf.key, nullptr, parents.peek(), (*p == '{') ? Type::Object : Type::Array};
                    if (!entries.push(entry)) return error = Error::Alloc, 0;
                    if (!parents.push(entries.length() - 1)) return error = Error::TooDeep, 0;  // idx of last in array
                    buf.key = buf.value = nullptr;
                    state = State::Idle;
                } break;

                case '}':
                case ']': {
                    if (state != State::Idle || entries[parents.peek()].type != ((*p == '}') ? Type::Object : Type::Array)) {
                        return error = Error::UnexClose, 0;
                    }
                    *p = 0;
                    if (!parents.pop()) return error = Error::NoParent, 0;
                    state = State::Idle;
                    if (!parents.length()) return 1;
                } break;

                default: {
                    if (
                        !(entries[parents.peek()].type == Type::Object && state == State::WaitValue) &&
                        !(entries[parents.peek()].type == Type::Array && (state == State::WaitValue || state == State::Idle))) return error = Error::UnexToken, 0;

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
                            return error = Error::UnknownToken, 0;
                    }
                    while (true) {
                        if (*p == '.') {
                            if (buf.type == Type::Int) buf.type = Type::Float;
                            else return error = Error::UnknownToken, 0;
                        }
                        if (!p[1]) return error = Error::BrokenToken, 0;
                        bool end = 0;
                        switch (p[1]) {
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
                        if (len == 4 && !strncmp_P(buf.value, PSTR("true"), 4))
                            ;
                        else if (len == 5 && !strncmp_P(buf.value, PSTR("false"), 5))
                            ;
                        else return error = Error::BrokenToken, 0;
                    }
                    Entry_t entry = Entry_t{buf.key, buf.value, parents.peek(), buf.type};
                    if (!entries.push(entry)) return error = Error::Alloc, 0;
                    buf.key = buf.value = nullptr;
                    state = State::Idle;
                } break;

            }  // switch

            if (readStr) {
                readStr = 0;
                p++;
                buf.value = p;
                if (!skipString()) return error = Error::BrokenString, 0;
                *p = 0;
                Entry_t entry = Entry_t{buf.key, buf.value, parents.peek(), Type::String};
                if (!entries.push(entry)) return error = Error::Alloc, 0;
                buf.key = buf.value = nullptr;
                state = State::Idle;
            }

            p++;
        }  // while

        if (parents.length()) return error = Error::BrokenContainer, 0;
        return 1;
    }

    // вывести в Print с форматированием
    void stringify(Print* p) {
        int16_t idx = 0;
        uint8_t dep = 0;
        _stringify(*p, idx, -1, dep);
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
            case Error::None:
                return F("None");
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
        }
    }

    // ============ PRIVATE ============
   private:
    const char* str = nullptr;
    char* p = nullptr;
    Error error = Error::None;

    bool skipString() {
        while (*p++) {
            if (*p == '\"' && p[-1] != '\\') return 1;
        }
        return 0;
    }

    void printT(Print& p, int16_t amount) {
        while (amount--) {
            p.print(' ');
            p.print(' ');
        }
    }

    void _stringify(Print& p, int16_t& idx, int16_t parent, uint8_t& dep) {
        bool first = true;
        while (idx < entries.length()) {
            Entry_t ent = entries[idx];
            if (ent.parent != parent) return;
            if (first) first = false;
            else p.print(",\n");

            if (ent.type == Type::Array || ent.type == Type::Object) {
                printT(p, dep);
                if (ent.key) {
                    p.print('\"');
                    p.print(ent.key);
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
                if (ent.key) {
                    p.print('\"');
                    p.print(ent.key);
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
                }
                if (ent.type == Type::String) p.print('\"');
                idx++;
            }
        }
    }
};

}  // namespace gsutil

namespace GSON {

// ================== DOC DYNAMIC ==================
class Doc : public gsutil::DocCore<-1> {
   public:
    Doc(uint16_t reserve = 0) : gsutil::DocCore<-1>(reserve) {}
};

// ================== DOC STATIC ==================
template <uint16_t capacity>
class DocStatic : public gsutil::DocCore<capacity> {};

}  // namespace GSON