#pragma once
#include <Arduino.h>
#include <limits.h>

namespace gson {

#if (UINT_MAX == UINT32_MAX)
typedef uint16_t parent_t;
#define GSON_MAX_INDEX 0xffff
#else
// 16 бит адрес не имеет смысла на слабых платформах
typedef uint8_t parent_t;
#define GSON_MAX_INDEX 0xff
#endif

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
};

}  // namespace gson