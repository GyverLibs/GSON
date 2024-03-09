#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "types.h"

#define GSON_MAX_LEN 0xffff
#define GSON_MAX_KEY_LEN 32

namespace gsutil {

struct Entry_t {
#if (UINT_MAX == UINT32_MAX)
    gson::parent_t parent : 9;
#else
    gson::parent_t parent;
#endif
    gson::Type type : 3;
    uint16_t key_len : 5;
    uint16_t val_len : 15;

    uint16_t key_offs;
    uint16_t val_offs;
#ifndef GSON_NO_HASH
    size_t key_hash;
#endif

    inline bool is(gson::Type t) {
        return type == t;
    }
    void reset() {
        key_offs = val_offs = key_len = val_len = 0;
        type = gson::Type::None;
    }

    inline const char* key(const char* str) {
        return str + key_offs;
    }
    inline const char* value(const char* str) {
        return str + val_offs;
    }

    inline sutil::AnyText keyAT(const char* str) {
        return sutil::AnyText(key(str), key_len);
    }
    inline sutil::AnyText valueAT(const char* str) {
        return sutil::AnyText(value(str), val_len);
    }

    bool isContainer() {
        return is(gson::Type::Array) || is(gson::Type::Object);
    }
    inline bool isObject() {
        return is(gson::Type::Object);
    }
    inline bool isArray() {
        return is(gson::Type::Array);
    }
};

// capacity -1 == dynamic
template <int16_t capacity = -1>
class Entries {
   public:
    Entries(uint16_t size = 0) {
        if (capacity < 0) reserve(size);
        else _eptr = _arr;
    }
    ~Entries() {
        if (capacity < 0 && _eptr) free(_eptr);
    }

    void clear() {
        _len = 0;
        _hashed = 0;
    }

    bool push(const Entry_t& val) {
        if (capacity < 0) {
            if (_len >= _cap && !reserve(_cap + 1)) return 0;
        } else {
            if (_len >= _cap) return 0;
        }
        _eptr[_len] = val;
        _len++;
        return 1;
    }

    bool pop() {
        if (!_len) return 0;
        _len--;
        return 1;
    }

    Entry_t& last() const {
        return _eptr[_len ? (_len - 1) : 0];
    }

    bool reserve(uint16_t size) {
        if (capacity < 0) {
            Entry_t* narr = (Entry_t*)realloc(_eptr, size * sizeof(Entry_t));
            if (narr) {
                _eptr = narr;
                _cap = size;
                return 1;
            }
        }
        return 0;
    }

    uint16_t length() const {
        return _len;
    }

    Entry_t& get(uint16_t idx) const {
        return _eptr[idx < _len ? idx : 0];
    }

    Entry_t& operator[](int idx) const {
        return get((uint16_t)idx);
    }

    void hashKeys(const char* str) {
#ifndef GSON_NO_HASH
        for (uint16_t i = 0; i < _len; i++) {
            _eptr[i].key_hash = _eptr[i].keyAT(str).hash();
        }
        _hashed = 1;
#endif
    }

    bool hashed() const {
        return _hashed;
    }

   private:
    Entry_t* _eptr = nullptr;
    Entry_t _arr[capacity >= 0 ? capacity : 0];
    uint16_t _cap = (capacity >= 0 ? capacity : 0);
    uint16_t _len = 0;
    bool _hashed = 0;
};

}  // namespace gsutil