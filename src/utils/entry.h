#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "stack.h"
#include "types.h"

namespace GSON {
using sutil::AnyText;

struct Entry_t {
    const char* key;
    const char* value;
    int8_t parent;
    GSON::Type type;
};

class Entry : public AnyText {
   public:
    Entry(const gsutil::Stack<Entry_t>& entries, int16_t idx) : AnyText(entries[idx].value, false), entries(entries), idx(idx) {}

    // получить элемент по ключу
    Entry get(const AnyText& key) const {
        if (valid() && entries[idx].type == GSON::Type::Object) {
            for (uint16_t i = 0; i < entries.length(); i++) {
                if (entries[i].parent == idx &&
                    entries[i].key &&
                    key.compare(entries[i].key)) return Entry(entries, i);
            }
        }
        return Entry(entries, -1);
    }

    // получить элемент по индексу
    Entry get(int index) const {
        if (valid() && _isContainer()) {
            for (uint16_t i = 0; i < entries.length(); i++) {
                if (entries[i].parent == idx) {
                    if (!index) return Entry(entries, i);
                    index--;
                }
            }
        }
        return Entry(entries, -1);
    }

    // доступ по ключу (контейнер - Object)
    Entry operator[](const char* key) const {
        return get(key);
    }
    Entry operator[](const __FlashStringHelper* key) const {
        return get(key);
    }
    Entry operator[](const String& key) const {
        return get(key);
    }
    Entry operator[](String& key) const {
        return get(key);
    }

    // доступ по индексу (контейнер - Array или Object)
    Entry operator[](int index) const {
        return get(index);
    }

    // проверка корректности (существования)
    bool valid() const {
        return idx >= 0;
    }

    // получить ключ
    AnyText key() const {
        return entries[idx].key ? entries[idx].key : "";
    }

    // получить значение
    AnyText value() const {
        return entries[idx].value ? entries[idx].value : "";
    }

    // получить размер (для объектов и массивов. Для остальных 0)
    uint16_t length() const {
        if (!valid() || !_isContainer()) return 0;
        uint16_t len = 0;
        for (uint16_t i = 0; i < entries.length(); i++) {
            if (entries[i].parent == idx) len++;
        }
        return len;
    }

    // получить тип
    GSON::Type type() const {
        if (!valid()) return GSON::Type::None;
        return entries[idx].type;
    }

    // void operator=(const Entry& entry) {
    //     idx = entry.idx;
    // }

   private:
    const gsutil::Stack<Entry_t>& entries;
    int16_t idx = -1;

    bool _isContainer() const {
        return entries[idx].type == GSON::Type::Array || entries[idx].type == GSON::Type::Object;
    }
    bool _hasVal() const {
        return valid() && !_isContainer();
    }
};

}  // namespace GSON