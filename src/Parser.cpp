//
// Created by jithan on 16/11/25.
//


#include "cool/Parser.h++"

namespace cool {

    // Parser public methods

    Parser::Parser(std::vector<Token> &tok) : tokens(tok) {}

    std::unique_ptr<ProgramNode> Parser::parse() {
        ast = parseProgram();
        return std::move(ast);
    }

    void Parser::printAST() const {
        if (ast)
            ast->print(0);
    }

    //---------------------------------------------------------------------------------------
    // precedence table for operators
    /*
        .
        @
        ~
        isvoid
        * /
        + -
        <= <
        =
        not
        <-
    */
    int Parser::getPrecedence(TokenType op) {
        switch (op) {
            case TokenType::DOT:
                return 9;
            case TokenType::AT:
                return 9;
            case TokenType::TILDE:
                return 8;
            case TokenType::ISVOID:
                return 8;
            case TokenType::STAR:
                return 7;
            case TokenType::SLASH:
                return 7;
            case TokenType::PLUS:
                return 6;
            case TokenType::MINUS:
                return 6;
            case TokenType::LESS_THAN:
                return 5;
            case TokenType::LESS_EQUAL:
                return 5;
            case TokenType::EQUAL:
                return 4;
            case TokenType::NOT:
                return 3;
            case TokenType::ASSIGN:
                return 2;
            default:
                return 0;
        }
    }

    // to check if the operator is a binary operator
    bool Parser::isBinaryOp(TokenType op) {
        switch (op) {
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::STAR:
            case TokenType::SLASH:
            case TokenType::LESS_EQUAL:
            case TokenType::LESS_THAN:
            case TokenType::EQUAL:
                return true;
            default:
                return false;
        }
    }

    //---------------------------------------------------------------------------------------
    // Parser Helper Methods
    Token &Parser::current() {
        if (current_token >= tokens.size())
            throw std::runtime_error("Unexpected end of file");

        return tokens[current_token];
    }

    Token &Parser::peek() {
        if (current_token + 1 >= tokens.size())
            throw std::runtime_error("Unexpected end of file");

        return tokens[current_token + 1];
    }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            current_token++;
            return true;
        }
        return false;
    }

    bool Parser::check(TokenType type) { return (current_token < tokens.size() && current().type == type); }

    Token Parser::consume(TokenType type, const std::string &err_msg) {
        if (check(type))
            return tokens[current_token++];

        std::stringstream ss;
        ss << err_msg << "at line " << current().line << ", column " << current().column;
        ss << ". Found: " << tokensToString(current().type) << " '" << current().value;

        throw std::runtime_error(ss.str());
    }

    void Parser::synchronize() {
        while (current_token < tokens.size()) {
            if (current().type == TokenType::SEMICOLON) {
                current_token++;
                return;
            }

            // incase it does not find the safe point ';' so we check for
            // class, if, while, let, case
            switch (current().type) {
                case TokenType::CLASS:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::LET:
                case TokenType::CASE:
                    return;
                default:
                    current_token++;
            }
        }
    }

    //----------------------------------------------------------------------------------------
    // Program ::= [Class}+
    std::unique_ptr<ProgramNode> Parser::parseProgram() {

        auto program = std::make_unique<ProgramNode>();

        while (current_token < tokens.size() && check(TokenType::CLASS))
            program->classes.push_back(parseClass());

        if (program->classes.empty())
            throw std::runtime_error("Program must have atleast one class");

        return program;
    }


    //----------------------------------------------------------------------------------------
    // Class ::= class TYPE [inherits TYPE] { [feature]* }
    std::unique_ptr<ClassNode> Parser::parseClass() {

        consume(TokenType::CLASS, "Expected 'class'");

        auto class_node = std::make_unique<ClassNode>();
        class_node->name = consume(TokenType::TYPE_ID, "Expexted class name").value;

        // if there is inheritence
        if (match(TokenType::INHERITS)) {
            class_node->parent = consume(TokenType::TYPE_ID, "Expected parent class name").value;
        }

        consume(TokenType::LBRACE, "Expected '{' after class name");

        if (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            class_node->features.push_back(parseFeature());
        }

        consume(TokenType::RBRACE, "Expected '} after class features'");
        consume(TokenType::SEMICOLON, "Expected ';' after class definition");

        return class_node;
    }


    //----------------------------------------------------------------------------------------
    // Feature ::= ID( [ formal [[, formal]]∗] ) : TYPE { expr }
    //             | ID : TYPE [ <- expr ]
    std::unique_ptr<FeatureNode> Parser::parseFeature() {
        auto name_token = consume(TokenType::OBJECT_ID, "Expected feature  name");

        if (check(TokenType::LPAREN)) {
            return parseMethod(name_token.value);
        } else {
            return parseAttribute(name_token.value);
        }
    }

    //----------------------------------------------------------------------------------------
    // Attribute ::= ID : TYPE [ <- Expr ]
    std::unique_ptr<AttributeNode> Parser::parseAttribute(const std::string &name) {

        auto attr = std::make_unique<AttributeNode>();
        attr->name = name;

        consume(TokenType::COLON, "Expected ':' after attribute name");
        attr->type = consume(TokenType::TYPE_ID, "Expexted attribute type").value;

        // assign is optional
        // x : String; or x : Int <- 0;
        if (match(TokenType::ASSIGN)) {
            attr->init_expr = parseExpression();
        }

        consume(TokenType::SEMICOLON, "Expected ';' after attribute");

        return attr;
    }

    //----------------------------------------------------------------------------------------
    // Method ::= ID( [Formal[,Formal]*] ) : TYPE { Expr }
    std::unique_ptr<MethodNode> Parser::parseMethod(const std::string &name) {

        auto method = std::make_unique<MethodNode>();
        method->name = name;

        // again method can be
        // add(x : Int, y :Int) or add()
        if (!check(TokenType::RPAREN)) {
            do {
                auto formal_name = consume(TokenType::OBJECT_ID, "Expected formal parameter name").value;

                consume(TokenType::COLON, "Expected ':' after formal parameter name");
                auto formal_type = consume(TokenType::TYPE_ID, "Expexted formal parameter type").value;

                method->formals.emplace_back(formal_name, formal_type);
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RPAREN, "Expected ')' after formal parameter");
        consume(TokenType::COLON, "Expected ':' after method formals");
        method->return_type = consume(TokenType::TYPE_ID, "Expexted return type").value;
        consume(TokenType::LBRACE, "Expected '{' after return type");

        method->body = parseExpression();

        consume(TokenType::RBRACE, "Expected '}' after method body");
        consume(TokenType::SEMICOLON, "Expected ';' after method");

        return method;
    }

    //----------------------------------------------------------------------------------------

} // namespace cool
