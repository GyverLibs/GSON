#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entries.h"
#include "types.h"

namespace gson {

class Entry : public su::Text {
   public:
    Entry(const gsutil::Entries<>* ens, parent_t idx, const char* str) : ens(ens), idx(idx), str(str) {
        if (valid()) {
            _str = ens->get(idx).value(str);
            _len = ens->get(idx).val_len;
        }
    }

    // ===================== BY KEY =====================

    // получить элемент по ключу
    Entry get(const su::Text& key) const {
        if (valid() && ens->get(idx).isObject()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx &&
                    ens->get(i).key_offs &&
                    key.compare(ens->get(i).keyText(str))) return Entry(ens, i, str);
            }
        }
        return Entry(nullptr, 0, str);
    }

    // содержит элемент с указанным ключом
    bool includes(const su::Text& key) const {
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
    Entry get(size_t hash) const {
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
    bool includes(size_t hash) const {
        return get(hash).valid();
    }

    // доступ по хэшу ключа (контейнер - Object)
    Entry operator[](size_t hash) const {
        return get(hash);
    }

    // ===================== BY INDEX =====================

    // получить элемент по индексу
    Entry get(int index) const {
        if (valid() && (uint16_t)index < ens->length() && ens->get(idx).isContainer()) {
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
        return get(index);
    }

    // ===================== MISC =====================

    // проверка корректности (существования)
    inline bool valid() const {
        return ens && str;
    }

    explicit inline operator bool() const {
        return valid();
    };

    // получить ключ
    su::Text key() const {
        return (valid()) ? ens->get(idx).keyText(str) : "";
    }

    // получить хэш ключа
    size_t keyHash() const {
#ifndef GSON_NO_HASH
        return valid() ? (ens->hashed() ? ens->get(idx).key_hash : ens->get(idx).keyText(str).hash()) : 0;
#else
        return 0;
#endif
    }

    // получить значение
    su::Text value() const {
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

    // вывести в Print с форматированием
    void stringify(Print& pr) {
        if (!valid()) return;
        if (ens->get(idx).isContainer()) {
            uint8_t dep = 1;
            parent_t index = idx + 1;
            pr.println(ens->get(idx).isObject() ? '{' : '[');
            _stringify(pr, index, ens->get(index).parent, dep);
            pr.println();
            pr.print(ens->get(idx).isObject() ? '}' : ']');
        } else {
            _print(pr, ens->get(idx));
        }
        pr.println();
    }

   private:
    const gsutil::Entries<>* ens;
    const parent_t idx;
    const char* str;

    void _printTab(Print& p, uint8_t amount) {
        while (amount--) {
            p.print(' ');
            p.print(' ');
        }
    }

    void _print(Print& p, gsutil::Entry_t& ent) {
        if (ent.key_offs) {
            p.print('\"');
            p.print(ent.keyText(str));
            p.print("\":");
        }
        if (ent.is(gson::Type::String)) p.print('\"');
        switch ((gson::Type)ent.type) {
            case gson::Type::String:
            case gson::Type::Int:
            case gson::Type::Float:
                p.print(ent.valueText(str));
                break;
            case gson::Type::Bool:
                p.print((*(ent.value(str)) == 't') ? F("true") : F("false"));
                break;
            default:
                break;
        }
        if (ent.is(gson::Type::String)) p.print('\"');
    }

    void _stringify(Print& p, parent_t& index, parent_t parent, uint8_t& dep) {
        bool first = true;
        while (index < ens->length()) {
            gsutil::Entry_t ent = ens->get(index);
            if (ent.parent != parent) return;

            if (first) first = false;
            else p.print(",\n");

            if (ent.isContainer()) {
                _printTab(p, dep);
                if (ent.key_offs) {
                    p.print('\"');
                    p.print(ent.keyText(str));
                    p.print("\": ");
                }
                p.print((ent.isArray()) ? '[' : '{');
                p.print('\n');
                parent_t prev = index;
                index++;
                dep++;
                _stringify(p, index, prev, dep);
                dep--;
                p.print('\n');
                _printTab(p, dep);
                p.print((ent.isArray()) ? ']' : '}');
            } else {
                _printTab(p, dep);
                _print(p, ent);
                index++;
            }
        }
    }
};

}  // namespace gson