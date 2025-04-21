#pragma once
#include <GTL.h>

#include "parser.h"

namespace gson {

// ================== STREAM ==================
class ParserStream : public Parser {
   public:
    ParserStream(size_t size = 0) : Parser(size) {}

    // прочитать из потока и сохранить себе
    bool parse(Stream* stream, size_t length) {
        if (!stream || !length || !json.resize(length)) return 0;

        if (stream->readBytes(json, length) != length) {
            json.reset();
            return 0;
        }
        return Parser::parse(json.buf(), length);
    }

    // прочитать из строки и сохранить себе
    bool parse(const char* str, size_t length) {
        if (!str || !length || !json.resize(length)) return 0;

        memcpy((void*)json.buf(), str, length);
        return Parser::parse(json.buf(), length);
    }

    // освободить память
    void reset() {
        json.reset();
        Parser::reset();
    }

    // получить скачанный json пакет как Text
    Text getRaw() {
        return json ? Text(json.buf(), json.size()) : Text();
    }

   private:
    using Parser::parse;
    gtl::array<uint8_t> json;
};

}  // namespace gson