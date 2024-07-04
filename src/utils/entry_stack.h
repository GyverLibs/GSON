#pragma once
#include <Arduino.h>
#include <GTL.h>

#include "entry_t.h"

namespace gsutil {
class EntryStack : public gtl::stack_uniq<Entry_t> {
   public:
    EntryStack() : gtl::stack_uniq<Entry_t>() {}

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
        gtl::stack_uniq<Entry_t>::reset();
    }

    // очистить буфер для следующего парсинга
    void clear() {
        hash.reset();
        gtl::stack_uniq<Entry_t>::clear();
    }

    inline Text keyText(int idx) const {
        return get(idx).keyText(str);
    }
    inline Text valueText(int idx) const {
        return get(idx).valueText(str);
    }
    size_t getHash(int idx) {
        return hash ? hash[idx] : 0;
    }

    // буфер и строка существуют
    bool valid() const {
        return gtl::stack_uniq<Entry_t>::valid() && str;
    }

    void move(EntryStack& es) noexcept {
        if (this == &es) return;
        gtl::stack_uniq<Entry_t>::move(es);
        hash.move(es.hash);
        str = es.str;
    }

    const char* str = nullptr;
    gtl::array_uniq<size_t> hash;

   private:
};

}  // namespace gsutil