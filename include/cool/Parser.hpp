//
// Created by jithan on 16/11/25.
//

#pragma once

/* page 17 of coool-manual
The precedence of infix binary and prefix unary operations, from highest to lowest, is given by the
following table:
    .
    @
    ~
    isvoid
    * /
    + -
    <= < =
    not
    <-
All binary operations are left-associative, with the exception of assignment, which is right-associative,
and the three comparison operations, which do not associate.

COOL SYNTAX

program ::= [[class; ]]+
class   ::= class TYPE [inherits TYPE] { [[feature; ]]∗}
feature ::= ID( [ formal [[, formal]]∗] ) : TYPE { expr }
            | ID : TYPE [ <- expr ]
formal  ::= ID : TYPE
expr    ::= ID <- expr
            | expr[@TYPE].ID( [ expr [[, expr]]∗] )
            | ID( [ expr [[, expr]]∗] )
            | if expr then expr else expr fi
            | while expr loop expr pool
            | { [[expr; ]]+}
            | let ID : TYPE [ <- expr ] [[,ID : TYPE [ <- expr ]]]∗ in expr
            | case expr of [[ID : TYPE => expr; ]]+esac
            | new TYPE
            | isvoid expr
            | expr + expr
            | expr − expr
            | expr ∗ expr
            | expr / expr
            | ˜expr
            | expr < expr
            | expr <= expr
            | expr = expr
            | not expr
            | (expr)
            | ID
            | integer
            | string
            | true
            | false

*/
#include <llvm/IR/Value.h>
#include <memory>
#include  <sstream>
#include <string>
#include <vector>
#include "cool/Lexer.hpp"
#include "cool/AST.hpp"


namespace cool {

    class Parser {
    public:
        explicit Parser(std::vector<Token> &tok);

        std::unique_ptr<ProgramNode> parse();
        void printAST() const;

    private:
        std::unique_ptr<ProgramNode> parseProgram();
        std::unique_ptr<ClassNode> parseClass();
        std::unique_ptr<FeatureNode> parseFeature();
        std::unique_ptr<AttributeNode> parseAttribute(const std::string &name);
        std::unique_ptr<MethodNode> parseMethod(const std::string &name);
        std::unique_ptr<ExpressionNode> parseExpression();
        std::unique_ptr<ExpressionNode> parseAssignment();
        std::unique_ptr<ExpressionNode> parseDispatch(std::unique_ptr<ExpressionNode> object);
        std::unique_ptr<ExpressionNode> parseIf();
        std::unique_ptr<ExpressionNode> parseWhile();
        std::unique_ptr<ExpressionNode> parseBlock();
        std::unique_ptr<ExpressionNode> parseLet();
        std::unique_ptr<ExpressionNode> parseCase();
        std::unique_ptr<CaseBranchNode> parseCaseBranch();
        std::unique_ptr<ExpressionNode> parseNew();
        std::unique_ptr<ExpressionNode> parseIsVoid();
        std::unique_ptr<ExpressionNode> parsePrimary();
        std::unique_ptr<ExpressionNode> parseUnary();
        std::unique_ptr<ExpressionNode> parseBinaryOp(std::unique_ptr<ExpressionNode> left, int min_precedence);

        // Helper func
        Token &current();
        Token &peek();
        bool match(TokenType type);
        bool check(TokenType type);
        Token consume(TokenType type, const std::string &err_msg);
        void synchronize(); // error recovery to get to ';' incase syntax error

        // Precedence Table
        static int getPrecedence(TokenType op);
        static bool isBinaryOp(TokenType op);

        std::vector<Token> &tokens;
        size_t current_token{0};
        std::unique_ptr<ProgramNode> ast;
    };


} // namespace cool
