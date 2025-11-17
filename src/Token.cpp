//
// Created by jithan on 16/11/25.
//

#include "cool/Token.hpp"

namespace cool {

    std::string tokensToString(TokenType type) {
        switch (type) {
            case TokenType::CLASS: return "CLASS";
        case TokenType::ELSE: return "ELSE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::FI: return "FI";
        case TokenType::IF: return "IF";
        case TokenType::IN: return "IN";
        case TokenType::INHERITS: return "INHERITS";
        case TokenType::ISVOID: return "ISVOID";
        case TokenType::LET: return "LET";
        case TokenType::LOOP: return "LOOP";
        case TokenType::POOL: return "POOL";
        case TokenType::THEN: return "THEN";
        case TokenType::WHILE: return "WHILE";
        case TokenType::CASE: return "CASE";
        case TokenType::ESAC: return "ESAC";
        case TokenType::NEW: return "NEW";
        case TokenType::OF: return "OF";
        case TokenType::NOT: return "NOT";
        case TokenType::TRUE: return "TRUE";
        case TokenType::TYPE_ID: return "TYPE_ID";
        case TokenType::OBJECT_ID: return "OBJECT_ID";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::STRING: return "STRING";
        case TokenType::SELF: return "SELF";
        case TokenType::SELF_TYPE: return "SELF_TYPE";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::LESS_THAN: return "LESS_THAN";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::TILDE: return "TILDE";
        case TokenType::AT: return "AT";
                case TokenType::DARROW: return "DARROW";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::UNKNOWN: return "UNKNOWN";
        case TokenType::ERROR: return "ERROR";
        default: return "UNKNOWN_TYPE";
        }
    }
}