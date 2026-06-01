#pragma once
#include <Arduino.h>
#include <StringUtilsGyver.h>
#include <limits.h>

#include "types.h"

namespace gson {
#ifdef GSON_NO_LIMITS
#define GSON_MAX_KEY_LEN 255
#if (UINT_MAX == UINT32_MAX)
#define GSON_PARENT_BIT 16
typedef uint16_t parent_t;
#else
#define GSON_PARENT_BIT 8
typedef uint8_t parent_t;
#endif

#else  // GSON_NO_LIMITS
#define GSON_MAX_KEY_LEN 31
#if (UINT_MAX == UINT32_MAX)
#define GSON_PARENT_BIT 9
typedef uint16_t parent_t;
#else
#define GSON_PARENT_BIT 8
typedef uint8_t parent_t;
#endif

#endif  // GSON_NO_LIMITS
}  // namespace gson

#define GSON_MAX_LEN 0xffff
#define GSON_MAX_INDEX ((1 << GSON_PARENT_BIT) - 1)

namespace gsutil {

struct Entry_t {
#ifdef GSON_NO_LIMITS
    gson::parent_t parent;  // 16/8 bit
    uint8_t type;           // 8 bit
    uint8_t key_len;        // 8 bit
    uint16_t val_len;       // 16 bit
#else                       // GSON_NO_LIMITS
    gson::parent_t parent : GSON_PARENT_BIT;  // 512/256
    uint8_t type : 3;                         // 7
    uint8_t key_len : 5;                      // 31
    uint16_t val_len : 15;                    // 32 767
#endif                      // GSON_NO_LIMITS

    uint16_t key_offs;
    uint16_t val_offs;

    void reset() {
        key_offs = val_offs = key_len = val_len = 0;
        setType(gson::Type::None);
    }

    inline const char* key(const char* json) const {
        return json + key_offs;
    }
    inline const char* value(const char* json) const {
        return json + val_offs;
    }

    inline Text keyText(const char* json) const {
        return Text(key(json), key_len);
    }
    inline Text valueText(const char* json) const {
        return Text(value(json), val_len);
    }

    inline bool is(gson::Type t) const {
        return getType() == t;
    }
    bool isContainer() const {
        return is(gson::Type::Array) || is(gson::Type::Object);
    }
    inline bool isObject() const {
        return is(gson::Type::Object);
    }
    inline bool isArray() const {
        return is(gson::Type::Array);
    }

    inline gson::Type getType() const {
        return gson::Type(type);
    }
    inline void setType(gson::Type t) {
        type = uint8_t(t);
    }
};

}  // namespace gsutil