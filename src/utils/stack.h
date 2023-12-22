#pragma once
#include <Arduino.h>

namespace gsutil {

// capacity -1 == dynamic
template <typename T, int16_t capacity = -1>
class Stack {
   public:
    Stack(uint16_t size = 0) {
        if (capacity < 0) reserve(size);
        else arr = _arr;
    }
    ~Stack() {
        if (capacity < 0 && arr) free(arr);
    }

    void clear() {
        len = 0;
    }

    bool push(const T& val) {
        if (capacity < 0) {
            if (len >= cap && !reserve(cap + 1)) return 0;
        } else {
            if (len >= cap) return 0;
        }
        arr[len] = val;
        len++;
        return 1;
    }

    bool pop() {
        if (!len) return 0;
        len--;
        return 1;
    }

    T& peek() const {
        return arr[len ? (len - 1) : 0];
    }

    bool reserve(uint16_t size) {
        if (capacity < 0) {
            T* narr = (T*)realloc(arr, size * sizeof(T));
            if (narr) {
                arr = narr;
                cap = size;
                return 1;
            }
        }
        return 0;
    }

    uint16_t length() const {
        return len;
    }

    T& operator[](int idx) const {
        return arr[idx >= 0 ? idx : 0];
    }

    T* arr = nullptr;

   private:
    uint16_t len = 0;
    uint16_t cap = (capacity >= 0 ? capacity : 0);
    T _arr[capacity >= 0 ? capacity : 0];
};

}  // namespace gsutil