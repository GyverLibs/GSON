#pragma once
#include <Arduino.h>
#include <GTL.h>

#include "entry_t.h"

namespace gsutil {
class EntryStack : public gtl::stack<Entry_t> {
    typedef gtl::stack<Entry_t> ST;

   public:
    void hashKeys() {
        if (valid() && hash.resize(length())) {
            for (uint16_t i = 0; i < length(); i++) {
                hash[i] = keyText(i).hash();
            }
        }
    }

    // ключи хешированы
    bool hashed() const {
        return hash.size() == length();
    }

    // освободить память
    void reset() {
        hash.reset();
        ST::reset();
    }

    // очистить буфер для следующего парсинга
    void clear() {
        hash.reset();
        ST::clear();
    }

    inline const char* key(int idx) const {
        return _get(idx).key(str);
    }
    inline const char* value(int idx) const {
        return _get(idx).value(str);
    }

    inline Text keyText(int idx) const {
        return _get(idx).keyText(str);
    }
    inline Text valueText(int idx) const {
        return _get(idx).valueText(str);
    }

    size_t getHash(int idx) {
        return hash ? hash[idx] : 0;
    }

    // буфер и строка существуют
    bool valid() const {
        return ST::valid() && str;
    }

    const char* str = nullptr;
    gtl::array<size_t> hash;
};

}  // namespace gsutil