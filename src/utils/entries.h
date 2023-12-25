#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "types.h"

namespace gsutil {

struct Entry_t {
    union {
        const char* str;
        size_t hash;
    } key;
    const char* value;
    gson::parent_t parent;
    gson::Type type;
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

    Entry_t& get(int idx) const {
        return _eptr[idx >= 0 ? idx : 0];
    }

    Entry_t& operator[](int idx) const {
        return get(idx);
    }

    void hashKeys() {
        for (uint16_t i = 0; i < _len; i++) {
            _eptr[i].key.hash = sutil::hash(_eptr[i].key.str);
        }
        _hashed = 1;
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