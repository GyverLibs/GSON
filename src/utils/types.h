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
};

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

}  // namespace gson