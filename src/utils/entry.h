#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entry_stack.h"
#include "types.h"

namespace gson {

class Entry : public su::Text {
   public:
    Entry(const gsutil::EntryStack* ens = nullptr, parent_t idx = 0) : ens(ens), idx(idx) {
        if (_valid()) *((su::Text*)this) = ens->valueText(idx);
    }

    // ===================== BY KEY =====================

    // получить элемент по ключу
    Entry get(const su::Text& key) const {
        if (_valid() && ens->get(idx).isObject()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx &&
                    ens->get(i).key_offs &&
                    key.compare(ens->keyText(i))) return Entry(ens, i);
            }
        }
        return Entry();
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
        if (_valid() && ens->hashed() && ens->get(idx).isObject()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx && ens->hash[i] == hash) return Entry(ens, i);
            }
        }
        return Entry();
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
        if (_valid() && (size_t)index < ens->length() && ens->get(idx).isContainer()) {
            for (uint16_t i = idx + 1; i < ens->length(); i++) {
                if (ens->get(i).parent == idx) {
                    if (!index) return Entry(ens, i);
                    index--;
                }
            }
        }
        return Entry();
    }

    // доступ по индексу (контейнер - Array или Object)
    Entry operator[](int index) const {
        return get(index);
    }

    // ===================== MISC =====================

    // получить ключ
    su::Text key() const {
        return _valid() ? ens->keyText(idx) : su::Text();
    }

    // получить хэш ключа
    size_t keyHash() const {
        return _valid() ? (ens->hashed() ? ens->hash[idx] : ens->keyText(idx).hash()) : false;
    }

    // получить значение
    su::Text value() const {
        return *this;
    }

    // получить размер для объектов и массивов
    size_t length() const {
        if (!_valid() || !ens->get(idx).isContainer()) return 0;
        size_t len = 0;
        for (size_t i = 0; i < ens->length(); i++) {
            if (ens->get(i).parent == idx) len++;
        }
        return len;
    }

    // получить тип элемента
    gson::Type type() const {
        return _valid() ? ens->get(idx).type : gson::Type::None;
    }

    // сравнить тип элемента
    bool is(gson::Type type) const {
        return _valid() ? ens->get(idx).type == type : false;
    }

    // элемент Array или Object
    bool isContainer() const {
        return _valid() ? ens->get(idx).isContainer() : false;
    }

    // элемент Object
    bool isObject() const {
        return _valid() ? ens->get(idx).isObject() : false;
    }

    // элемент Array
    bool isArray() const {
        return _valid() ? ens->get(idx).isArray() : false;
    }

    // вывести в Print с форматированием
    void stringify(Print& pr) const {
        if (!_valid()) return;
        if (ens->get(idx).isContainer()) {
            uint8_t depth = 1;
            parent_t index = idx + 1;
            pr.println(ens->get(idx).isObject() ? '{' : '[');
            _stringify(pr, index, ens->get(index).parent, depth);
            pr.println();
            pr.print(ens->get(idx).isObject() ? '}' : ']');
        } else {
            _print(pr, idx);
        }
        pr.println();
    }

    // индекс элемента в общем массиве парсера
    parent_t index() {
        return idx;
    }

   private:
    const gsutil::EntryStack* ens = nullptr;
    parent_t idx = 0;

    // массив и строка существуют
    bool _valid() const {
        return ens && ens->valid();
    }

    void _printTab(Print& p, uint8_t amount) const {
        while (amount--) {
            p.print(' ');
            p.print(' ');
        }
    }

    void _print(Print& p, parent_t idx) const {
        gsutil::Entry_t& ent = ens->get(idx);
        if (ent.key_offs) {
            p.print('\"');
            p.print(ens->keyText(idx));
            p.print("\":");
        }
        if (ent.is(gson::Type::String)) p.print('\"');
        switch ((gson::Type)ent.type) {
            case gson::Type::String:
            case gson::Type::Int:
            case gson::Type::Float:
                p.print(ens->valueText(idx));
                break;
            case gson::Type::Bool:
                p.print((ens->valueText(idx)[0] == 't') ? F("true") : F("false"));
                break;
            default:
                break;
        }
        if (ent.is(gson::Type::String)) p.print('\"');
    }

    void _stringify(Print& p, parent_t& idx, parent_t parent, uint8_t& depth) const {
        bool first = true;
        while (idx < ens->length()) {
            gsutil::Entry_t& ent = ens->get(idx);
            if (ent.parent != parent) return;

            if (first) first = false;
            else p.print(",\n");

            if (ent.isContainer()) {
                _printTab(p, depth);
                if (ent.key_offs) {
                    p.print('\"');
                    p.print(ens->keyText(idx));
                    p.print("\": ");
                }
                p.print((ent.isArray()) ? '[' : '{');
                p.print('\n');
                parent_t prev = idx;
                idx++;
                depth++;
                _stringify(p, idx, prev, depth);
                depth--;
                p.print('\n');
                _printTab(p, depth);
                p.print((ent.isArray()) ? ']' : '}');
            } else {
                _printTab(p, depth);
                _print(p, idx);
                idx++;
            }
        }
    }
};

}  // namespace gson