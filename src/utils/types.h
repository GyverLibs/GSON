#pragma once
#include <Arduino.h>

namespace gson {

enum class Type : uint8_t {
    None,
    Object,
    Array,
    String,
    Int,
    Float,
    Bool,
    Null,
};

static const __FlashStringHelper* readType(Type t) {
    switch (t) {
        case Type::Object: return F("Object");
        case Type::Array: return F("Array");
        case Type::String: return F("String");
        case Type::Int: return F("Int");
        case Type::Float: return F("Float");
        case Type::Bool: return F("Bool");
        case Type::Null: return F("Null");
        default: return F("None");
    }
}

enum class Error : uint8_t {
    None,
    Alloc,
    TooDeep,
    NoParent,
    NotContainer,
    UnexComma,
    UnexColon,
    UnexToken,
    UnexQuotes,
    UnexOpen,
    UnexClose,
    UnknownToken,
    BrokenToken,
    BrokenString,
    BrokenContainer,
    EmptyKey,
    IndexOverflow,
    LongPacket,
    LongKey,
    EmptyString,
};

static const __FlashStringHelper* readError(Error e) {
    switch (e) {
        case Error::Alloc: return F("Alloc");
        case Error::TooDeep: return F("TooDeep");
        case Error::NoParent: return F("NoParent");
        case Error::NotContainer: return F("NotContainer");
        case Error::UnexComma: return F("UnexComma");
        case Error::UnexColon: return F("UnexColon");
        case Error::UnexToken: return F("UnexToken");
        case Error::UnexQuotes: return F("UnexQuotes");
        case Error::UnexOpen: return F("UnexOpen");
        case Error::UnexClose: return F("UnexClose");
        case Error::UnknownToken: return F("UnknownToken");
        case Error::BrokenToken: return F("BrokenToken");
        case Error::BrokenString: return F("BrokenString");
        case Error::BrokenContainer: return F("BrokenContainer");
        case Error::EmptyKey: return F("EmptyKey");
        case Error::IndexOverflow: return F("IndexOverflow");
        case Error::LongPacket: return F("LongPacket");
        case Error::LongKey: return F("LongKey");
        case Error::EmptyString: return F("EmptyString");
        default: return F("None");
    }
}

}  // namespace gson