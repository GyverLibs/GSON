#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entries.h"
#include "types.h"

namespace gson {

class Entry : public sutil::AnyText {
   public:
#ifndef GSON_NO_LEN
    Entry(const gsutil::Entries<>* entries, parent_t idx, const char* str) : sutil::AnyText((entries ? (str + entries->get(idx).value) : nullptr), entries->get(idx).len), entries(entries), idx(idx), str(str) {}
#else
    Entry(const gsutil::Entries<>* entries, parent_t idx, const char* str) : sutil::AnyText(entries ? (str + entries->get(idx).value) : nullptr), entries(entries), idx(idx), str(str) {}
#endif

    // ===================== BY KEY =====================

    // получить элемент по ключу
    Entry get(const sutil::AnyText& key) const {
        if (valid() && entries->get(idx).type == gson::Type::Object && !entries->hashed()) {
            for (uint16_t i = idx + 1; i < entries->length(); i++) {
                if (entries->get(i).parent == idx &&
                    entries->get(i).key.str &&
                    key.compare(entries->get(i).key.str)) return Entry(entries, i, str);
            }
        }
        return Entry(nullptr, 0, str);
    }

    // содержит элемент с указанным ключом
    bool includes(const sutil::AnyText& key) const {
        return get(key).valid();
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

    // ===================== BY HASH =====================

    // получить элемент по хэшу ключа
    Entry get(const size_t& hash) const {
        if (valid() && entries->get(idx).type == gson::Type::Object && entries->hashed()) {
            for (uint16_t i = idx + 1; i < entries->length(); i++) {
                if (entries->get(i).parent == idx && entries->get(i).key.hash == hash) return Entry(entries, i, str);
            }
        }
        return Entry(nullptr, 0, str);
    }

    // содержит элемент с указанным хэшем ключа
    bool includes(const size_t& hash) const {
        return get(hash).valid();
    }

    // доступ по хэшу ключа (контейнер - Object)
    Entry operator[](const size_t& hash) const {
        return get(hash);
    }

    // ===================== BY INDEX =====================

    // получить элемент по индексу
    Entry get(int index) const {
        if (_isContainer()) {
            for (uint16_t i = idx + 1; i < entries->length(); i++) {
                if (entries->get(i).parent == idx) {
                    if (!index) return Entry(entries, i, str);
                    index--;
                }
            }
        }
        return Entry(nullptr, 0, str);
    }

    // доступ по индексу (контейнер - Array или Object)
    Entry operator[](int index) const {
        return get(index);
    }

    // ===================== MISC =====================

    // проверка корректности (существования)
    bool valid() const {
        return entries;
    }

    // получить ключ
    sutil::AnyText key() const {
        return (valid() && entries->get(idx).key.str && !entries->hashed()) ? entries->get(idx).key.str : "";
    }

    // получить хэш ключа
    size_t keyHash() const {
        return valid() ? (entries->hashed() ? entries->get(idx).key.hash : sutil::hash(entries->get(idx).key.str)) : 0;
    }

    // получить значение
    sutil::AnyText value() const {
        return *this;
    }

    // получить размер для объектов и массивов
    uint16_t length() const {
        if (!_isContainer()) return 0;
        uint16_t len = 0;
        for (uint16_t i = 0; i < entries->length(); i++) {
            if (entries->get(i).parent == idx) len++;
        }
        return len;
    }

    // получить тип элемента
    gson::Type type() const {
        return valid() ? entries->get(idx).type : gson::Type::None;
    }

   private:
    const gsutil::Entries<>* entries = nullptr;
    parent_t idx = 0;
    const char* str;

    bool _isContainer() const {
        return valid() && (entries->get(idx).type == gson::Type::Array || entries->get(idx).type == gson::Type::Object);
    }
};

}  // namespace gson