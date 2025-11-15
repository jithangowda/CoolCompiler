//
// Created by jithan on 13/11/25.
//

#pragma once

#include <vector>
#include "cool/Token.hpp"


namespace cool {

    class Lexer {

    public:
        explicit Lexer(const std::string& filePath);
        std::vector<Token> tokenize();

    private:
        char peek() const;
        char advance();
        void skipWhitespace();
        void skipLineComment();
        void skipBlockComment();
        Token readNumber();
        Token readString();
        Token readIdentifier();
        Token readOperator();

        std::string source;
        size_t pos {0};
        int line {1};
        int column {1};
    };
}

