#pragma once
#include <Arduino.h>
#include <StringUtils.h>
#include <limits.h>

#include "types.h"

namespace gson {
#ifdef GSON_NO_LIMITS
#define GSON_MAX_KEY_LEN 256
#if (UINT_MAX == UINT32_MAX)
#define GSON_PARENT_BIT 16
typedef uint16_t parent_t;
#else
#define GSON_PARENT_BIT 8
typedef uint8_t parent_t;
#endif

#else  // GSON_NO_LIMITS
#define GSON_MAX_KEY_LEN 32
#if (UINT_MAX == UINT32_MAX)
#define GSON_PARENT_BIT 9
typedef uint16_t parent_t;
#else
#define GSON_PARENT_BIT 8
typedef uint8_t parent_t;
#endif

#endif  // GSON_NO_LIMITS
}  // namespace gson

#define GSON_MAX_LEN 0xffff
#define GSON_MAX_INDEX ((1 << GSON_PARENT_BIT) - 1)

namespace gsutil {

struct Entry_t {
#ifdef GSON_NO_LIMITS
    gson::parent_t parent;  // 16/8 bit
    gson::Type type;        // 8 bit
    uint8_t key_len;        // 8 bit
    uint16_t val_len;       // 16 bit
#else                       // GSON_NO_LIMITS
    gson::parent_t parent : GSON_PARENT_BIT;  // 512/256
    gson::Type type : 3;                      // 8
    uint8_t key_len : 5;                      // 32
    uint16_t val_len : 15;                    // 32 768
#endif                      // GSON_NO_LIMITS

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

    inline su::Text keyText(const char* str) {
        return su::Text(key(str), key_len);
    }
    inline su::Text valueText(const char* str) {
        return su::Text(value(str), val_len);
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
            _eptr[i].key_hash = _eptr[i].keyText(str).hash();
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