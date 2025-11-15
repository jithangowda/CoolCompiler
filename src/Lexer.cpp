//
// Created by jithan on 13/11/25.
//

#include "cool/Lexer.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

cool::Lexer::Lexer(const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
}

std::vector<cool::Token> cool::Lexer::tokenize() {
    std::vector<cool::Token> tokens;

    while (pos < source.length()) {

        skipWhitespace();

        if (pos >= source.length())
            break;

    }

    return tokens;
}

void cool::Lexer::skipWhitespace() {
    while (pos < source.length() && std::isspace(source[pos]))
        advance();
}

char cool::Lexer::peek() const {
    if (pos + 1 < source.length())
        return source[pos+1];

    return '\0';
}

char cool::Lexer::advance() {
    if (pos >= source.length())
        return '\0';

    char current = source[pos++];

    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }

    return current;
}
