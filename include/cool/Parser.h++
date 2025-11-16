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
#include <sstream>
#include <string>
#include <vector>
#include "Lexer.hpp"


namespace cool {

    // forward declartionss
    class ASTNode;
    class ProgramNode;
    class ClassNode;
    class FeatureNode;
    class AttributeNode;
    class MethodNode;
    class ExpressionNode;
    class IdentifierNode;
    class IntegerNode;
    class StringNode;
    class BoolNode;
    class NewNode;
    class IsVoidNode;
    class AssignmentNode;
    class DispatchNode; // this and staticDispatch is for . and @  -  its in cool-manual
    class StaticDispatchNode;
    class IfNode;
    class WhileNode;
    class BlockNode;
    class LetNode;
    class CaseNode;
    class CaseBranchNode;
    class BinaryOpNode;
    class UnaryOpNode;

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
        int getPrecedence(TokenType op);
        bool isBinaryOp(TokenType op);

        std::vector<Token> &tokens;
        size_t current_token{0};
        std::unique_ptr<ProgramNode> ast;
    };

    //---------------------------------------------------------------------------------------
    // AST Node - BASE CLASS
    class ASTNode {
    public:
        virtual ~ASTNode() = default;
        virtual llvm::Value *codegen() = 0;
        virtual void print(int indent) const = 0;
    };

    //----------------------------------------------------------------------------------------
    // Program Node - collection of all classes
    class ProgramNode : public ASTNode {
    public:
        std::vector<std::unique_ptr<ClassNode>> classes;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Class Node - class definition
    //(has a vector feature that is collection of attributes and methods)
    class ClassNode : public ASTNode {
    public:
        std::string name;
        std::string parent;
        std::vector<std::unique_ptr<FeatureNode>> features;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //---------------------------------------------------------------------------------------
    // Feature Node - BASE CLASS (attribute and method)
    class FeatureNode : public ASTNode {
    public:
        std::string name;
        ~FeatureNode() override = default;
    };

    //----------------------------------------------------------------------------------------
    // Attribute Feature
    class AttributeNode : public FeatureNode {
    public:
        std::string type;
        std::unique_ptr<ExpressionNode> init_expr;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Method Feature
    class MethodNode : public FeatureNode {
    public:
        std::string return_type;
        std::vector<std::pair<std::string, std::string>> formals; // (name, type for attributes)
        std::unique_ptr<ExpressionNode> body;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //---------------------------------------------------------------------------------------
    // Expression Node - BASE CLASS
    class ExpressionNode : public ASTNode {
    public:
        ~ExpressionNode() override = default;
    };

    // Identifier Epression
    class IdentifierNode : public ExpressionNode {
    public:
        std::string name;

        explicit IdentifierNode(std::string n) : name(std::move(n)) {}

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Integer Literal - parent ExpressionNode
    class IntegerNode : public ExpressionNode {
    public:
        int value;

        explicit IntegerNode(int v) : value(v) {}

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // String Literal - parent ExpressionNode
    class StringNode : public ExpressionNode {
    public:
        std::string value;
        explicit StringNode(std::string v) : value(std::move(v)) {}

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Bool Node - parent ExpressionNode
    class BoolNode : public ExpressionNode {
    public:
        bool value;

        explicit BoolNode(bool v) : value(v) {}

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // New Expression - parent ExpressionNode
    class NewNode : public ExpressionNode {
    public:
        std::string type_name;

        explicit NewNode(std::string v) : type_name(std::move(v)) {}

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // IsVoid Expression - parent ExpressionNode
    class IsVoidNode : public ExpressionNode {
    public:
        std::unique_ptr<ExpressionNode> expr;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Assignment Expression - parent ExpressionNode
    class AssignmentNode : public ExpressionNode {
    public:
        std::string identifier;
        std::unique_ptr<ExpressionNode> expr;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Dispatch Expression - parent ExpressionNode
    class DispatchNode : public ExpressionNode {
    public:
        std::string method_name;
        std::unique_ptr<ExpressionNode> object;
        std::vector<std::unique_ptr<ExpressionNode>> arguments;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // StaticDispatch Expression - parent ExpressionNode
    class StaticDispatchNode : public ExpressionNode {
    public:
        std::string method_name;
        std::string type_name;
        std::unique_ptr<ExpressionNode> object;
        std::vector<std::unique_ptr<ExpressionNode>> arguments;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // If Expression - parent ExpressionNode
    class IfNode : public ExpressionNode {
    public:
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> then_branch;
        std::unique_ptr<ExpressionNode> else_branch;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // While Loop Expression - parent ExpressionNode
    class WhileNode : public ExpressionNode {
    public:
        std::unique_ptr<ExpressionNode> condition;
        std::unique_ptr<ExpressionNode> body;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Block Expression - parent ExpressionNode
    class BlockNode : public ExpressionNode {
    public:
        std::vector<std::unique_ptr<ExpressionNode>> expressions;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Let Expression - parent ExpressionNode
    class LetNode : public ExpressionNode {
    public:
        struct Binding {
            std::string identifier;
            std::string type_name;
            std::unique_ptr<ExpressionNode> init_expr;
        };

        std::vector<Binding> bindings;
        std::unique_ptr<ExpressionNode> body;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // CaseBranch node Expression - parent ExpressionNode
    class CaseBranchNode : public ASTNode {
    public:
        std::string identifier;
        std::string type_name;
        std::unique_ptr<ExpressionNode> expr;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };


    //----------------------------------------------------------------------------------------
    // Case Expression - parent ExpressionNode
    class CaseNode : public ExpressionNode {
    public:
        std::unique_ptr<ExpressionNode> expr;
        std::vector<std::unique_ptr<CaseBranchNode>> branches;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Binaary Operation
    class BinaryOpNode : public ExpressionNode {
    public:
        TokenType op;
        std::unique_ptr<ExpressionNode> left;
        std::unique_ptr<ExpressionNode> right;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };

    //----------------------------------------------------------------------------------------
    // Unary Operation
    class UnaryOpNode : public ExpressionNode {
    public:
        TokenType op;
        std::unique_ptr<ExpressionNode> expr;

        llvm::Value *codegen() override;
        void print(int indent) const override;
    };
} // namespace cool
