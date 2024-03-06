#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entries.h"
#include "types.h"

namespace gson {

class Entry : public sutil::AnyText {
   public:
    Entry(const gsutil::Entries<>* ens, parent_t idx, const char* str) : ens(ens), idx(idx), str(str) {
        if (valid()) {
            _str = ens->get(idx).value(str);
            _len = ens->get(idx).val_len;
        }
    }

    // ===================== BY KEY =====================

    // получить элемент по ключу
    Entry get(const sutil::AnyText& key) const {
        if (valid() && ens->get(idx).isObject()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx &&
                    ens->get(i).key_offs &&
                    key.compare(ens->get(i).keyAT(str))) return Entry(ens, i, str);
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
#ifndef GSON_NO_HASH
        if (valid() && ens->get(idx).isObject() && ens->hashed()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx && ens->get(i).key_hash == hash) return Entry(ens, i, str);
            }
        }
#endif
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
    Entry get(uint16_t index) const {
        if (valid() && index < ens->length() && ens->get(idx).isContainer()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx) {
                    if (!index) return Entry(ens, i, str);
                    index--;
                }
            }
        }
        return Entry(nullptr, 0, str);
    }

    // доступ по индексу (контейнер - Array или Object)
    Entry operator[](int index) const {
        return get((uint16_t)index);
    }

    // ===================== MISC =====================

    // проверка корректности (существования)
    bool valid() const {
        return ens && str;
    }

    // получить ключ
    sutil::AnyText key() const {
        return (valid()) ? ens->get(idx).keyAT(str) : "";
    }

    // получить хэш ключа
    size_t keyHash() const {
#ifndef GSON_NO_HASH
        return valid() ? (ens->hashed() ? ens->get(idx).key_hash : ens->get(idx).keyAT(str).hash()) : 0;
#else
        return 0;
#endif
    }

    // получить значение
    sutil::AnyText value() const {
        return *this;
    }

    // получить размер для объектов и массивов
    uint16_t length() const {
        if (!valid() || !ens->get(idx).isContainer()) return 0;
        uint16_t len = 0;
        for (uint16_t i = 0; i < ens->length(); i++) {
            if (ens->get(i).parent == idx) len++;
        }
        return len;
    }

    // получить тип элемента
    gson::Type type() const {
        return valid() ? ens->get(idx).type : gson::Type::None;
    }

   private:
    const gsutil::Entries<>* ens;
    const parent_t idx;
    const char* str;
};

}  // namespace gson