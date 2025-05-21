#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entry.h"
#include "entry_stack.h"
#include "types.h"

namespace gson {

// ================== PARSER ==================
class Parser {
   private:
    enum class State : uint8_t {
        Idle,
        WaitKey,
        WaitValue,
        WaitColon,
    };

   public:
    Parser(size_t size = 0) {
        reserve(size);
    }

    // зарезервировать память для ускорения парсинга
    bool reserve(size_t size) {
        return ents.reserve(size);
    }

    // освободить память
    void reset() {
        ents.reset();
        strp = nullptr;
        error = Error::None;
    }

    // очистить для нового парсинга
    void clear() {
        ents.clear();
        strp = nullptr;
        error = Error::None;
    }

    // получить количество элементов
    uint16_t length() const {
        return ents.length();
    }

    // получить размер документа в оперативной памяти (байт)
    uint16_t size() const {
        return length() * sizeof(gsutil::Entry_t);
    }

    // установить максимальную глубину вложенности парсинга (умолч. 16)
    void setMaxDepth(uint8_t maxdepth) {
        depth = maxdepth;
    }

    // хешировать ключи всех элементов (операция необратима)
    void hashKeys() {
        ents.hashKeys();
    }

    // проверить коллизии хэшей в объектах
    bool checkCollisions(bool recursive = true) const {
        return length() ? Entry(&ents, 0).checkCollisions(recursive) : false;
    }

    // проверка были ли хешированы ключи
    bool hashed() const {
        return ents.hashed();
    }

    // получить количество элементов в главном контейнере
    uint16_t rootLength() const {
        return length() ? Entry(&ents, 0).length() : 0;
    }

    // ===================== BY KEY =====================

    // доступ по ключу (главный контейнер - Object)
    Entry get(const Text& key) const {
        return length() ? Entry(&ents, 0).get(key) : Entry();
    }
    Entry operator[](const Text& key) const {
        return get(key);
    }

    // содержит элемент с указанным ключом
    bool has(const Text& key) const {
        return get(key).valid();
    }

    // ===================== BY HASH =====================

    // доступ по хэшу ключа (главный контейнер - Object)
    Entry get(size_t hash) const {
        return length() ? Entry(&ents, 0).get(hash) : Entry();
    }
    Entry operator[](size_t hash) const {
        return get(hash);
    }

    // содержит элемент с указанным хэшем ключа
    bool has(size_t hash) const {
        return get(hash).valid();
    }

    // ===================== BY INDEX =====================

    // доступ по индексу в главный контейнер - Array или Object
    Entry get(int index) const {
        return length() ? Entry(&ents, 0).get(index) : Entry();
    }
    Entry operator[](int index) const {
        return get(index);
    }

    // итерация по вложенным
    void loopAll(void (*cb)(Entry e)) {
        for (uint16_t i = 0; i < ents.length(); i++) cb(Entry(&ents, i));
    }

    // ===================== PARSE =====================

    // парсить в массив длины rootLength()
    template <typename T>
    bool parseTo(T& arr) const {
        return rootLength() ? Entry(&ents, 0).parseTo(arr) : 0;
    }

    // ===================== BY INDEX =====================

    // получить элемент по индексу в общем массиве парсера
    Entry getByIndex(parent_t index) const {
        return index < length() ? Entry(&ents, index) : Entry();
    }
    Entry _getByIndex(parent_t index) const {
        return Entry(&ents, index);
    }

    // ===================== MISC =====================

    // прочитать ключ по индексу
    Text key(int idx) const {
        return (length() && (uint16_t)idx < length()) ? ents.keyText(idx) : "";
    }
    // без проверок
    Text _key(int idx) const {
        return ents.keyText(idx);
    }

    // прочитать хэш ключа по индексу
    size_t keyHash(int idx) const {
        return (length() && (uint16_t)idx < length()) ? (ents.hashed() ? ents.hash[idx] : ents.keyText(idx).hash()) : 0;
    }
    // без проверок
    size_t _keyHash(int idx) const {
        return ents.hash[idx];
    }

    // прочитать значение по индексу
    Text value(int idx) const {
        return (length() && (uint16_t)idx < length()) ? ents.valueText(idx) : "";
    }
    // без проверок
    Text _value(int idx) const {
        return ents.valueText(idx);
    }

    // прочитать родителя по индексу
    parent_t parent(int idx) const {
        return ((uint16_t)idx < length()) ? ents[idx].parent : 0;
    }

    // получить тип по индексу
    Type type(int idx) const {
        return ((uint16_t)idx < length()) ? ents[idx].type : Type::None;
    }

    // прочитать тип по индексу
    const __FlashStringHelper* readType(int index) const {
        return gson::readType(type(index));
    }

    // ============ PARSE ============
    // парсить. Вернёт true при успешном парсинге
    bool parse(const Text& json) {
        return json.pgm() ? 0 : _startParse(json.str(), json.length());
    }
    bool parse(const char* json, uint16_t len) {
        return _startParse(json, len);
    }
    bool parse(const uint8_t* json, uint16_t len) {
        return _startParse((const char*)json, len);
    }

    // вывести в Print с форматированием
    void stringify(Print& pr) const {
        if (length()) Entry(&ents, 0).stringify(pr);
    }

    // ============ ERROR ============
    // есть ошибка парсинга
    bool hasError() const {
        return error != Error::None;
    }

    // получить ошибку
    Error getError() const {
        return error;
    }

    // индекс места ошибки в строке
    uint16_t errorIndex() const {
        return (ents && strp) ? (strp - ents.str) : 0;
    }

    // прочитать ошибку
    const __FlashStringHelper* readError() const {
        return gson::readError(error);
    }

    // ============ PRIVATE ============
   private:
    gsutil::EntryStack ents;
    char* strp = nullptr;
    Error error = Error::None;
    State state = State::Idle;
    bool strF = 0;
    gsutil::Entry_t ebuf;
    uint8_t depth = 16;
    const char* endp = 0;

    bool _startParse(const char* json, size_t length) {
        if (!length) {
            error = Error::EmptyString;
            return 0;
        }
        if (length >= (uint32_t)GSON_MAX_LEN) {
            error = Error::LongPacket;
            return 0;
        }

        ents.str = json;
        endp = ents.str + length;
        strp = (char*)ents.str;
        state = State::Idle;
        strF = 0;
        ebuf = gsutil::Entry_t();
        ents.clear();

        if ((strp[0] == '{' && strp[length - 1] == '}') || (strp[0] == '[' && strp[length - 1] == ']')) {
            ents.reserve(_count(json, length));
            error = _parse(0);
            ents[0].parent = GSON_MAX_INDEX;
        } else {
            error = Error::NotContainer;
        }
        return !hasError();
    }

    Error _parse(parent_t parent) {
        while (strp && strp < endp && *strp) {
            switch (*strp) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;

                case ',':
                    if (state != State::Idle) return Error::UnexComma;
                    state = (ents[parent].isArray()) ? State::WaitValue : State::WaitKey;
                    break;

                case ':':
                    if (ents[parent].isObject() && state == State::WaitColon) state = State::WaitValue;
                    else return Error::UnexColon;
                    break;

                case '\"':
                    switch ((Type)ents[parent].type) {
                        case Type::Array:
                            switch (state) {
                                case State::Idle:
                                case State::WaitValue:
                                    strF = 1;
                                    break;

                                default:
                                    return Error::UnexQuotes;
                            }
                            break;

                        case Type::Object:
                            switch (state) {
                                case State::WaitKey:
                                case State::Idle:
                                    ++strp;
                                    if (*strp == '\"' || strp >= endp) return Error::EmptyKey;
                                    ebuf.key_offs = strp - ents.str;
                                    while (1) {
                                        strp = (char*)memchr((void*)(strp + 1), '\"', endp - strp - 1);
                                        if (!strp) return Error::BrokenString;
                                        if (strp[-1] != '\\') break;
                                    }
                                    if (strp - ebuf.key(ents.str) > GSON_MAX_KEY_LEN) return Error::LongKey;
                                    ebuf.key_len = strp - ebuf.key(ents.str);
                                    state = State::WaitColon;
                                    break;

                                case State::WaitValue:
                                    strF = 1;
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
                    if (strp != ents.str) {  // not first symb
                        if (!(ents[parent].isArray() && (state == State::Idle || state == State::WaitValue)) &&
                            !(ents[parent].isObject() && state == State::WaitValue)) {
                            return Error::UnexOpen;
                        }
                    }
                    if (length() == GSON_MAX_INDEX - 1) return Error::IndexOverflow;

                    ebuf.type = (*strp == '{') ? Type::Object : Type::Array;
                    ebuf.parent = parent;
                    if (!ents.push(ebuf)) return Error::Alloc;
                    ebuf.reset();
                    state = State::Idle;
                    ++strp;
                    if (strp >= endp) return Error::BrokenContainer;
                    if (depth - 1 == 0) return Error::TooDeep;

                    --depth;
                    error = _parse(length() - 1);  // RECURSIVE
                    ++depth;
                    if (hasError()) return error;
                } break;

                case '}':
                case ']': {
                    if (state != State::Idle || ents[parent].type != ((*strp == '}') ? Type::Object : Type::Array)) {
                        return Error::UnexClose;
                    }
                    return Error::None;
                } break;

                default: {
                    if (!(ents[parent].isObject() && state == State::WaitValue) &&
                        !(ents[parent].isArray() && (state == State::WaitValue || state == State::Idle))) {
                        return Error::UnexToken;
                    }

                    ebuf.val_offs = strp - ents.str;
                    switch (*strp) {
                        case 't':
                        case 'f':
                            ebuf.type = Type::Bool;
                            break;
                        case '-':
                        case '0' ... '9':
                            ebuf.type = Type::Int;
                            break;
                        case 'n':
                            ebuf.type = Type::Null;
                            break;
                        default:
                            return Error::UnknownToken;
                    }
                    while (true) {
                        if (*strp == '.') {
                            if (ebuf.is(Type::Int)) ebuf.type = Type::Float;
                            else return Error::UnknownToken;
                        }
                        if (strp + 1 >= endp || !strp[1]) return Error::BrokenToken;

                        bool endF = 0;
                        switch (strp[1]) {  // next sym
                            case ' ':
                            case '\t':
                            case '\r':
                            case '\n':
                            case ',':
                            case '}':
                            case ']':
                                endF = 1;
                        }
                        if (!ebuf.val_len && (endF || !*strp)) {
                            ebuf.val_len = strp + 1 - ebuf.value(ents.str);
                        }
                        if (endF) break;
                        ++strp;
                    }
                    if (ebuf.is(Type::Bool)) {
                        if (!(ebuf.val_len == 4 && !strncmp_P(ebuf.value(ents.str), PSTR("true"), 4)) &&
                            !(ebuf.val_len == 5 && !strncmp_P(ebuf.value(ents.str), PSTR("false"), 5))) {
                            return Error::BrokenToken;
                        }
                    }
                    if (ebuf.is(Type::Null)) {
                        if (!(ebuf.val_len == 4 && !strncmp_P(ebuf.value(ents.str), PSTR("null"), 4))) {
                            return Error::BrokenToken;
                        }
                    }
                    if (length() == GSON_MAX_INDEX - 1) return Error::IndexOverflow;
                    ebuf.parent = parent;
                    if (!ents.push(ebuf)) return Error::Alloc;
                    ebuf.reset();
                    state = State::Idle;
                } break;

            }  // switch

            if (strF) {
                strF = 0;
                ++strp;
                if (strp >= endp) return Error::BrokenString;
                ebuf.val_offs = strp - ents.str;
                if (*strp != '\"') {
                    while (1) {
                        strp = (char*)memchr((void*)(strp + 1), '\"', endp - strp - 1);
                        if (!strp) return Error::BrokenString;
                        if (strp[-1] != '\\') break;
                    }
                }
                if (length() == GSON_MAX_INDEX - 1) return Error::IndexOverflow;
                ebuf.val_len = strp - ebuf.value(ents.str);
                ebuf.parent = parent;
                ebuf.type = Type::String;
                if (!ents.push(ebuf)) return Error::Alloc;
                ebuf.reset();
                state = State::Idle;
            }

            if (strp) ++strp;
        }  // while

        return (parent == 0) ? Error::None : Error::BrokenContainer;
    }

    // посчитать приблизительное количество элементов
    uint16_t _count(const char* str, uint16_t len) {
        if (!len) return 0;
        uint16_t count = 0;
        bool inStr = false;
        while (--len) {  // len-1.. 1
            switch (str[len]) {
                case '\"':
                    if (str[len - 1] != '\\') inStr = !inStr;
                    break;

                case '{':
                case '[':
                    if (!inStr) count += 2;
                    break;

                case ',':
                    if (!inStr) ++count;
                    break;
            }
        }
        return count;
    }
};

// ================== DEPRECATED ==================
template <size_t capacity>
class ParserStatic : public Parser {};

class Doc : public Parser {
   public:
    using Parser::Parser;
};

template <size_t capacity>
class DocStatic : public Parser {};

}  // namespace gson