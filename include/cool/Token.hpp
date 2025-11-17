//
// Created by jithan on 13/11/25.
//

#pragma once

#include <string>
#include <unordered_map>

namespace cool {
    enum class TokenType {
        // Keywords
        CLASS,
        ELSE,
        FALSE,
        FI,
        IF,
        IN,
        INHERITS,
        ISVOID,
        LET,
        LOOP,
        POOL,
        THEN,
        WHILE,
        CASE,
        ESAC,
        NEW,
        OF,
        NOT,
        TRUE,

        // Identifers & Literals
        TYPE_ID, // Begins with capital letter ( eg: Main, IO, String)
        OBJECT_ID, // Begins with small letter
        INTEGER,
        STRING,

        // Special Identifiers (not treated as keywords, In page 15 of cool-manual.pdf)
        SELF,
        SELF_TYPE,


        //Operators
        ASSIGN, // <-
        PLUS, // +
        MINUS, // -
        STAR, //*
        SLASH, // /
        EQUAL, // =
        LESS_THAN, // <
        LESS_EQUAL, // <=
        TILDE, // ~ (for some integer complement )
        AT, // @ (static dispatch)
        DARROW, // =>

        // Brackets & special symbols
        LPAREN, // (
        RPAREN, // )
        LBRACE, // {
        RBRACE, // }
        SEMICOLON, // ;
        COLON, // :
        COMMA, // ,
        DOT, // .

        //Special
        END_OF_FILE,
        UNKNOWN,
        ERROR
    };


    //----------------------------------------------------------------------------------------
    // Token Structure
    struct Token {
        TokenType type;
        std::string value;
        int line;
        int column;

        Token(TokenType t, const std::string& v, int l, int c)
            : type(t), value(v), line(l), column(c) {}
    };

    //----------------------------------------------------------------------------------------
    static const std::unordered_map<std::string, TokenType> KEYWORDS = {
        {"class", TokenType::CLASS},
        {"else", TokenType::ELSE},
        {"false", TokenType::FALSE},
        {"fi", TokenType::FI},
        {"if", TokenType::IF},
        {"in", TokenType::IN},
        {"inherits", TokenType::INHERITS},
        {"isvoid", TokenType::ISVOID},
        {"let", TokenType::LET},
        {"loop", TokenType::LOOP},
        {"pool", TokenType::POOL},
        {"then", TokenType::THEN},
        {"while", TokenType::WHILE},
        {"case", TokenType::CASE},
        {"esac", TokenType::ESAC},
        {"new", TokenType::NEW},
        {"of", TokenType::OF},
        {"not", TokenType::NOT},
        {"true", TokenType::TRUE}
    };

    //----------------------------------------------------------------------------------------
    // Special identifiers (not keywords but treated specially)
    static const std::unordered_map<std::string, TokenType> SPECIAL_IDS = {
        {"self", TokenType::SELF},
        {"SELF_TYPE", TokenType::SELF_TYPE}
    };

    //----------------------------------------------------------------------------------------
    // convert type name to string
    std::string tokensToString(TokenType type);
}
