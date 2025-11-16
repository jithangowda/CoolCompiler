//
// Created by jithan on 13/11/25.
//

#include "cool/Lexer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>


namespace cool {
    cool::Lexer::Lexer(const std::string &filePath) {
        std::ifstream file(filePath);

        if (!file) {
            throw std::runtime_error("Could not open file: " + filePath);
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        source = buffer.str();
    }

    std::vector<cool::Token> cool::Lexer::tokenize() {
        tokens.clear();
        while (pos < source.length()) {

            skipWhitespace();

            if (pos >= source.length())
                break;

            char current = source[pos];

            if (isdigit(current))
                tokens.emplace_back(readNumber());

            else if (current == '"')
                tokens.emplace_back(readString());

            else if (isalpha(current) || current == '_')
                tokens.emplace_back(readIdentifier());

            else if (ispunct(current)) {
                if (current == '-' && pos+1 < source.length() && peek() == '-' ) {
                    skipLineComment();
                    continue;
                }

                else if (current == '(' && peek() == '*') {
                    skipBlockComment();
                    continue;
                }
                tokens.emplace_back(readOperator());
            }

            else {
                tokens.emplace_back(TokenType::UNKNOWN, std::string(1, current), line, column);
                advance();
            }
        }

        tokens.emplace_back(TokenType::END_OF_FILE, "", line, column);
        return tokens;
    }

    void cool::Lexer::skipWhitespace() {
        while (pos < source.length() && std::isspace(source[pos]))
            advance();
    }

    char cool::Lexer::peek() const {
        if (pos + 1 < source.length())
            return source[pos + 1];

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

    // anything between '--' and \n are skipped (page 15 cool-manual)
    void cool::Lexer::skipLineComment() {
        while (pos < source.length() && source[pos] != '\n')
            advance();
    }

    // anything inside  (∗ . . . ∗) are skipped (page 15 cool-manual)
    // it does nested comments .. I dont know why but ok ;(
    /*
    (*
    This is a comment
    (* This is a nested comment *)
    Back in the outer comment
    *)
    */
    void cool::Lexer::skipBlockComment() {
        advance(); // consuem (
        advance(); // consume *

        int depth {1};
        while (pos < source.length() && depth > 0) {

            if (source[pos] == '(' && peek() == '*') {
                depth++;
                advance();
                advance();
            }
            else if (source[pos] == '*' && peek() == ')') {
                depth--;
                advance(); // consume *
                advance(); // consmue )
            }
            else {
                advance();
            }
        }

        if (depth > 0)
            throw std::runtime_error("Unterminated Block comment");
    }

    // eg - 68
    cool::Token cool::Lexer::readNumber() {
        int startLine {line};
        int startCol {column};
        std::string number;

        while (pos < source.length() && std::isdigit(source[pos])) {
            number += advance();
        }

        return cool::Token{TokenType::INTEGER, number, startLine, startCol};
    }

    // eg - "HelloWorld"
    cool::Token cool::Lexer::readString() {
        int startLine {line};
        int startCol {column};
        std::string str;

        advance(); // to skip first "

        while (pos < source.length() && source[pos] != '"') {
            if (source[pos] == '\\') { // '\\' meas backslash
                advance();

                switch (source[pos]) {
                    case 'b': str += '\b'; break;
                    case 't': str += '\t'; break;
                    case 'n': str += '\n'; break;
                    case 'f': str += '\f'; break; // formfeed
                    case '\\': str += '\\'; break;
                    case '"': str += '"'; break; // incase " comes inside again
                    default: str += source[pos]; break;

                }
                advance();
            }

            else if (source[pos] == '\n')
                throw std::runtime_error("Unterminated String");

            else
                str += advance();

            // Max string len i want is 1024
            if (str.length() > 1024)
                throw std::runtime_error("String too long: Max 1024 char");
        }

        if (pos >= source.length())
            throw std::runtime_error("Unterminated String");

        advance(); //consume "

        return cool::Token{TokenType::STRING, str, startLine, startCol};

    }


    cool::Token cool::Lexer::readIdentifier() {
        int startLine {line};
        int startCol {column};
        std::string identifier;

        identifier += advance();

        while (pos < source.length() && (isalnum(source[pos]) || source[pos] == '_')) {
            identifier += advance();
        }

        auto k_it = KEYWORDS.find(identifier);
        if (k_it != KEYWORDS.end())
            return Token{k_it->second, identifier, startLine, startCol};

        auto s_it = SPECIAL_IDS.find(identifier);
        if (s_it != SPECIAL_IDS.end())
            return Token{s_it->second, identifier, startLine, startCol};

        TokenType type = isupper(identifier[0]) ? TokenType::TYPE_ID : TokenType::OBJECT_ID;
        return Token{type, identifier, startLine, startCol};

    }

    cool::Token cool::Lexer::readOperator() {
        int startLine {line};
        int startCol {column};
        char first_operator = advance();

        std::string op(1, first_operator);

        if (pos < source.length()) {
            char second_operator = source[pos];

            std::string twoCharOp = op + std::string(1, second_operator);

            if (twoCharOp == "<-") {
                advance();
                return Token{TokenType::ASSIGN, twoCharOp, startLine, startCol};
            }

            else if (twoCharOp == "<=") {
                advance();
                return Token{TokenType::LESS_EQUAL, twoCharOp, startLine, startCol};
            }
        }

        switch (first_operator) {
            case '+': return Token{TokenType::PLUS, op, startLine, startCol};
            case '-': return Token{TokenType::MINUS, op, startLine, startCol};
            case '*': return Token{TokenType::STAR, op, startLine, startCol};
            case '/': return Token{TokenType::SLASH, op, startLine, startCol};
            case '=': return Token{TokenType::EQUAL, op, startLine, startCol};
            case '<': return Token{TokenType::LESS_THAN, op, startLine, startCol};
            case '~': return Token{TokenType::TILDE, op, startLine, startCol};
            case '@': return Token{TokenType::AT, op, startLine, startCol};
            case '(': return Token{TokenType::LPAREN, op, startLine, startCol};
            case ')': return Token{TokenType::RPAREN, op, startLine, startCol};
            case '{': return Token{TokenType::LBRACE, op, startLine, startCol};
            case '}': return Token{TokenType::RBRACE, op, startLine, startCol};
            case ';': return Token{TokenType::SEMICOLON, op, startLine, startCol};
            case ':': return Token{TokenType::COLON, op, startLine, startCol};
            case ',': return Token{TokenType::COMMA, op, startLine, startCol};
            case '.': return Token{TokenType::DOT, op, startLine, startCol};
            default: return Token{TokenType::UNKNOWN, op, startLine, startCol};
        }
    }

    void cool::Lexer::printTokens() const {
        for (auto & token : tokens) {
            TokenType t = token.type;
            std::string s = token.value;

            std::cout << std::left << std::setw(15) << cool::tokensToString(t) << s << '\n';
        }
    }
};