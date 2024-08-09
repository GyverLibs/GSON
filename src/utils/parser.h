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
        error = gson::Error::None;
    }

    // очистить для нового парсинга
    void clear() {
        ents.clear();
        strp = nullptr;
        error = gson::Error::None;
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

    // проверка были ли хешированы ключи
    bool hashed() const {
        return ents.hashed();
    }

    // получить количество элементов в главном контейнере
    uint16_t rootLength() const {
        return length() ? gson::Entry(&ents, 0).length() : 0;
    }

    // ===================== BY KEY =====================

    // доступ по ключу (главный контейнер - Object)
    gson::Entry get(const Text& key) const {
        return length() ? gson::Entry(&ents, 0).get(key) : gson::Entry();
    }
    gson::Entry operator[](const Text& key) const {
        return get(key);
    }

    // содержит элемент с указанным ключом
    bool has(const Text& key) const {
        return get(key).valid();
    }

    // ===================== BY HASH =====================

    // доступ по хэшу ключа (главный контейнер - Object)
    gson::Entry get(size_t hash) const {
        return length() ? gson::Entry(&ents, 0).get(hash) : gson::Entry();
    }
    gson::Entry operator[](size_t hash) const {
        return get(hash);
    }

    // содержит элемент с указанным хэшем ключа
    bool has(size_t hash) const {
        return get(hash).valid();
    }

    // ===================== BY INDEX =====================

    // доступ по индексу в главный контейнер - Array или Object
    gson::Entry get(int index) const {
        return length() ? gson::Entry(&ents, 0).get(index) : gson::Entry();
    }
    gson::Entry operator[](int index) const {
        return get(index);
    }

    // ===================== PARSE =====================

    // парсить в массив длины rootLength()
    template <typename T>
    bool parseTo(T& arr) const {
        return rootLength() ? gson::Entry(&ents, 0).parseTo(arr) : 0;
    }

    // ===================== BY INDEX =====================

    // получить элемент по индексу в общем массиве парсера
    gson::Entry getByIndex(parent_t index) const {
        return (length() && index < length()) ? gson::Entry(&ents, index) : gson::Entry();
    }

    // ===================== MISC =====================

    // прочитать ключ по индексу
    Text key(int idx) const {
        return (length() && (uint16_t)idx < length()) ? ents.keyText(idx) : "";
    }

    // прочитать хэш ключа по индексу
    size_t keyHash(int idx) const {
        return (length() && (uint16_t)idx < length()) ? (ents.hashed() ? ents.hash[idx] : ents.keyText(idx).hash()) : 0;
    }

    // прочитать значение по индексу
    Text value(int idx) const {
        return (length() && (uint16_t)idx < length()) ? ents.valueText(idx) : "";
    }

    // прочитать родителя по индексу
    gson::parent_t parent(int idx) const {
        return ((uint16_t)idx < length()) ? ents[idx].parent : 0;
    }

    // получить тип по индексу
    gson::Type type(int idx) const {
        return ((uint16_t)idx < length()) ? ents[idx].type : gson::Type::None;
    }

    // прочитать тип по индексу
    const __FlashStringHelper* readType(int index) const {
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
        if (length()) gson::Entry(&ents, 0).stringify(pr);
    }

    // ============ ERROR ============
    // есть ошибка парсинга
    bool hasError() const {
        return error != gson::Error::None;
    }

    // получить ошибку
    gson::Error getError() const {
        return error;
    }

    // индекс места ошибки в строке
    uint16_t errorIndex() const {
        return (ents && strp) ? (strp - ents.str) : 0;
    }

    // прочитать ошибку
    const __FlashStringHelper* readError() const {
        switch (error) {
            case gson::Error::Alloc: return F("Alloc");
            case gson::Error::TooDeep: return F("TooDeep");
            case gson::Error::NoParent: return F("NoParent");
            case gson::Error::NotContainer: return F("NotContainer");
            case gson::Error::UnexComma: return F("UnexComma");
            case gson::Error::UnexColon: return F("UnexColon");
            case gson::Error::UnexToken: return F("UnexToken");
            case gson::Error::UnexQuotes: return F("UnexQuotes");
            case gson::Error::UnexOpen: return F("UnexOpen");
            case gson::Error::UnexClose: return F("UnexClose");
            case gson::Error::UnknownToken: return F("UnknownToken");
            case gson::Error::BrokenToken: return F("BrokenToken");
            case gson::Error::BrokenString: return F("BrokenString");
            case gson::Error::BrokenContainer: return F("BrokenContainer");
            case gson::Error::EmptyKey: return F("EmptyKey");
            case gson::Error::IndexOverflow: return F("IndexOverflow");
            case gson::Error::LongPacket: return F("LongPacket");
            case gson::Error::LongKey: return F("LongKey");
            case gson::Error::EmptyString: return F("EmptyString");
            default: return F("None");
        }
    }

    Parser(Parser& p) {
        move(p);
    }
    Parser& operator=(Parser& p) {
        move(p);
        return *this;
    }

#if __cplusplus >= 201103L
    Parser(Parser&& p) noexcept {
        move(p);
    }
    Parser& operator=(Parser&& p) noexcept {
        move(p);
        return *this;
    }
#endif

    void move(Parser& p) {
        if (this == &p) return;
        ents.move(p.ents);
        strp = p.strp;
        error = p.error;
        state = p.state;
        strF = p.strF;
        ebuf = p.ebuf;
        depth = p.depth;
        endp = p.endp;
    }

    // ============ PRIVATE ============
   private:
    gsutil::EntryStack ents;
    char* strp = nullptr;
    gson::Error error = gson::Error::None;
    State state = State::Idle;
    bool strF = 0;
    gsutil::Entry_t ebuf;
    uint8_t depth = 16;
    const char* endp = 0;

    bool _startParse(const char* json, size_t length) {
        if (!length) {
            error = gson::Error::EmptyString;
            return 0;
        }
        if (length >= (uint32_t)GSON_MAX_LEN) {
            error = gson::Error::LongPacket;
            return 0;
        }

        ents.str = json;
        endp = ents.str + length;
        strp = (char*)ents.str;
        state = State::Idle;
        strF = 0;
        ebuf = gsutil::Entry_t();
        ents.clear();

        if (strp[0] == '{' || strp[0] == '[') {
            ents.reserve(_count(json, length));
            error = _parse(0);
            ents[0].parent = GSON_MAX_INDEX;
        } else {
            error = gson::Error::NotContainer;
        }
        return !hasError();
    }

    gson::Error _parse(gson::parent_t parent) {
        while (strp && strp < endp && *strp) {
            switch (*strp) {
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;

                case ',':
                    if (state != State::Idle) return gson::Error::UnexComma;
                    state = (ents[parent].isArray()) ? State::WaitValue : State::WaitKey;
                    break;

                case ':':
                    if (ents[parent].isObject() && state == State::WaitColon) state = State::WaitValue;
                    else return gson::Error::UnexColon;
                    break;

                case '\"':
                    switch ((gson::Type)ents[parent].type) {
                        case gson::Type::Array:
                            switch (state) {
                                case State::Idle:
                                case State::WaitValue:
                                    strF = 1;
                                    break;

                                default:
                                    return gson::Error::UnexQuotes;
                            }
                            break;

                        case gson::Type::Object:
                            switch (state) {
                                case State::WaitKey:
                                case State::Idle:
                                    strp++;
                                    if (*strp == '\"' || strp >= endp) return gson::Error::EmptyKey;
                                    ebuf.key_offs = strp - ents.str;
                                    while (1) {
                                        strp = (char*)memchr((void*)(strp + 1), '\"', endp - strp - 1);
                                        if (!strp) return gson::Error::BrokenString;
                                        if (strp[-1] != '\\') break;
                                    }
                                    if (strp - ebuf.key(ents.str) > GSON_MAX_KEY_LEN) return gson::Error::LongKey;
                                    ebuf.key_len = strp - ebuf.key(ents.str);
                                    state = State::WaitColon;
                                    break;

                                case State::WaitValue:
                                    strF = 1;
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
                    if (strp != ents.str) {  // not first symb
                        if (!(ents[parent].isArray() && (state == State::Idle || state == State::WaitValue)) &&
                            !(ents[parent].isObject() && state == State::WaitValue)) {
                            return gson::Error::UnexOpen;
                        }
                    }
                    if (length() == GSON_MAX_INDEX - 1) return gson::Error::IndexOverflow;

                    ebuf.type = (*strp == '{') ? gson::Type::Object : gson::Type::Array;
                    ebuf.parent = parent;
                    if (!ents.push(ebuf)) return gson::Error::Alloc;
                    ebuf.reset();
                    state = State::Idle;
                    strp++;
                    if (strp >= endp) return gson::Error::BrokenContainer;
                    if (depth - 1 == 0) return gson::Error::TooDeep;

                    depth--;
                    error = _parse(length() - 1);  // RECURSIVE
                    depth++;
                    if (hasError()) return error;
                } break;

                case '}':
                case ']': {
                    if (state != State::Idle || ents[parent].type != ((*strp == '}') ? gson::Type::Object : gson::Type::Array)) {
                        return gson::Error::UnexClose;
                    }
                    return gson::Error::None;
                } break;

                default: {
                    if (!(ents[parent].isObject() && state == State::WaitValue) &&
                        !(ents[parent].isArray() && (state == State::WaitValue || state == State::Idle))) {
                        return gson::Error::UnexToken;
                    }

                    ebuf.val_offs = strp - ents.str;
                    switch (*strp) {
                        case 't':
                        case 'f':
                            ebuf.type = gson::Type::Bool;
                            break;
                        case '-':
                        case '0' ... '9':
                            ebuf.type = gson::Type::Int;
                            break;
                        default:
                            return gson::Error::UnknownToken;
                    }
                    while (true) {
                        if (*strp == '.') {
                            if (ebuf.is(gson::Type::Int)) ebuf.type = gson::Type::Float;
                            else return gson::Error::UnknownToken;
                        }
                        if (strp + 1 >= endp || !strp[1]) return gson::Error::BrokenToken;

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
                        strp++;
                    }
                    if (ebuf.is(gson::Type::Bool)) {
                        if (!(ebuf.val_len == 4 && !strncmp_P(ebuf.value(ents.str), PSTR("true"), 4)) &&
                            !(ebuf.val_len == 5 && !strncmp_P(ebuf.value(ents.str), PSTR("false"), 5))) {
                            return gson::Error::BrokenToken;
                        }
                    }
                    if (length() == GSON_MAX_INDEX - 1) return gson::Error::IndexOverflow;
                    ebuf.parent = parent;
                    if (!ents.push(ebuf)) return gson::Error::Alloc;
                    ebuf.reset();
                    state = State::Idle;
                } break;

            }  // switch

            if (strF) {
                strF = 0;
                strp++;
                if (strp >= endp) return gson::Error::BrokenString;
                ebuf.val_offs = strp - ents.str;
                if (*strp != '\"') {
                    while (1) {
                        strp = (char*)memchr((void*)(strp + 1), '\"', endp - strp - 1);
                        if (!strp) return gson::Error::BrokenString;
                        if (strp[-1] != '\\') break;
                    }
                }
                if (length() == GSON_MAX_INDEX - 1) return gson::Error::IndexOverflow;
                ebuf.val_len = strp - ebuf.value(ents.str);
                ebuf.parent = parent;
                ebuf.type = gson::Type::String;
                if (!ents.push(ebuf)) return gson::Error::Alloc;
                ebuf.reset();
                state = State::Idle;
            }

            if (strp) strp++;
        }  // while

        return (parent == 0) ? gson::Error::None : gson::Error::BrokenContainer;
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
                    if (!inStr) count++;
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