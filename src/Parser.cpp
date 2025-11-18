//
// Created by jithan on 16/11/25.
//


#include "cool/Parser.hpp"

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
        .       9
        @       9
        ~       8
        isvoid  8
        * /     7
        + -     6
        <= <    5
        =       4
        not     3
        <-      2
    */
    int Parser::getPrecedence(TokenType op) {
        switch (op) {
            case TokenType::DOT:
            case TokenType::AT: return 9;
            case TokenType::TILDE:
            case TokenType::ISVOID: return 8;
            case TokenType::STAR:
            case TokenType::SLASH: return 7;
            case TokenType::PLUS:
            case TokenType::MINUS: return 6;
            case TokenType::LESS_THAN:
            case TokenType::LESS_EQUAL: return 5;
            case TokenType::EQUAL: return 4;
            case TokenType::NOT: return 3;
            case TokenType::ASSIGN: return 2;
            default: return 0;
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
            case TokenType::DOT:
            case TokenType::AT:
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

        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
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
        consume(TokenType::LPAREN, "Expected '(' after method name");
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
    /* expr    ::= ID <- expr
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

    using parseExpression to start the parsing of expression
    eg:  x <- a + b * c
        AssignmentNode(
            identifier: "x",
            expr: BinaryOpNode(+,
                IdentifierNode("a"),
                BinaryOpNode(*,
                    IdentifierNode("b"),
                    IdentifierNode("c")
                    )
                )
            )

    */

    std::unique_ptr<ExpressionNode> Parser::parseExpression() {
        return parseAssignment();
    }

    //----------------------------------------------------------------------------------------
    // Assignemnt :: ID <- Expression
    std::unique_ptr<ExpressionNode> Parser::parseAssignment() {
        auto left = parseBinaryOp(parseUnary(), 0);

        if (match(TokenType::ASSIGN)) {

            // if left is identifier
            // will only work if left is an identifier
            if (auto id_node = dynamic_cast<IdentifierNode*>(left.get())) {
                auto assign = std::make_unique<AssignmentNode>();
                assign->identifier = id_node->name;
                assign->expr = parseAssignment();
                return assign;
            }
            else {
                throw std::runtime_error("Left side of asignment must be an identifier");
            }
        }

        return left;
    }

    //----------------------------------------------------------------------------------------
    // Dispatch helper (written dispatch in cool-manual)
    // is like '.' nad '->' operator to access like in C
    // Dynamic Dispatch ::= obj.method(arg1, arg2)
    //  Static Dispatch :: obj@ParentType.method(arg1, arg2)
    std::unique_ptr<ExpressionNode> Parser::parseDispatch(std::unique_ptr<ExpressionNode> object) {

        // if '.' dispatch
        if (match(TokenType::DOT)) {
            auto dispatch = std::make_unique<DispatchNode>();
            dispatch->object = std::move(object);
            dispatch->method_name = consume(TokenType::OBJECT_ID, "Expected method name after '.").value;
            consume(TokenType::LPAREN, "Expected ')' after method name");

            if (!check(TokenType::RPAREN)) {
                do {
                    dispatch->arguments.push_back(parseExpression());

                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "Expected ')' after method arguments");

            return dispatch;
        }
        // if '@' dispatch
        else if (match(TokenType::AT)) {
            auto static_dispatch = std::make_unique<StaticDispatchNode>();
            static_dispatch->object = std::move(object);
            static_dispatch->type_name = consume(TokenType::TYPE_ID, "Expexted  type name after '@'").value;
            consume(TokenType::DOT, "Expected '.' after type name");

            static_dispatch->method_name = consume(TokenType::OBJECT_ID, "Expected method name after '.'").value;

            consume(TokenType::LPAREN, "Expected ')' after method name");

            if (!check(TokenType::RPAREN)) {
                do {
                    static_dispatch->arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "Expected ')' after method arguments");

            return static_dispatch;

        }

        return object;
    }

    //----------------------------------------------------------------------------------------
    // If ::= if expr then expr else expr fi
    std::unique_ptr<ExpressionNode> Parser::parseIf() {
        consume(TokenType::IF, "Expected 'if' ");

        auto if_node = std::make_unique<IfNode>();
        if_node->condition = parseExpression();
        consume(TokenType::THEN, "Expected 'then'");
        if_node->then_branch = parseExpression();
        consume(TokenType::ELSE, "Expected 'else'");
        if_node->else_branch = parseExpression();
        consume(TokenType::FI, "Expected 'fi'");

        return if_node;
    }

    //----------------------------------------------------------------------------------------
    // While ::= while expr loop expr pool
    std::unique_ptr<ExpressionNode> Parser::parseWhile() {
        consume(TokenType::WHILE, "Expected 'while'");

        auto while_node = std::make_unique<WhileNode>();
        while_node->condition = parseExpression();
        consume(TokenType::LOOP, "Expected 'loop'");
        while_node->body = parseExpression();
        consume(TokenType::POOL, "Expected 'pool'");

        return while_node;
    }

    //----------------------------------------------------------------------------------------
    // Block ::=  { [[expr; ]]+}
    std::unique_ptr<ExpressionNode> Parser::parseBlock() {
        consume(TokenType::LBRACE, "Expected '{' ");

        auto block_node = std::make_unique<BlockNode>();

        do {
            block_node->expressions.push_back(parseExpression());
            consume(TokenType::SEMICOLON, "Expected ';' after expression in block");
        } while (!check(TokenType::RBRACE));

        consume(TokenType::RBRACE, "Expected '}'");

        return block_node;
    }

    //----------------------------------------------------------------------------------------
    // Let ::= let ID : TYPE [ <- expr ] [[,ID : TYPE [ <- expr ]]]∗ in expr
    std::unique_ptr<ExpressionNode> Parser::parseLet() {
        consume(TokenType::LET, "Expected 'let'");

        auto let_node = std::make_unique<LetNode>();

        do {
            LetNode::Binding binding;
            binding.identifier = consume(TokenType::OBJECT_ID, "Expected  variable name").value;
            consume(TokenType::COLON, "Expected ':' after identifier");
            binding.type_name = consume(TokenType::TYPE_ID, "Expexted variable type").value;

            if (match(TokenType::ASSIGN)) {
                binding.init_expr = parseExpression();
            }

            let_node->bindings.push_back(std::move(binding));

        }while (match(TokenType::COMMA));

        consume(TokenType::IN   , "Expected 'in; after bindings");
        let_node->body = parseExpression();

        return let_node;
    }

    //----------------------------------------------------------------------------------------
    // Case ::= case expr of [[ID : TYPE => expr; ]]+esac
    std::unique_ptr<ExpressionNode> Parser::parseCase() {
        consume(TokenType::CASE, "Expected 'case'");

        auto case_node = std::make_unique<CaseNode>();
        case_node->expr = parseExpression();
        consume(TokenType::OF, "Expected 'of' after expression");

        do {
            case_node->branches.push_back(parseCaseBranch());
        } while (!check(TokenType::ESAC));

        consume(TokenType::ESAC, "Expected 'esac'");

        return case_node;
    }

    //----------------------------------------------------------------------------------------
    // Case branch ::= ID : TYPE => expr;
    std::unique_ptr<CaseBranchNode> Parser::parseCaseBranch() {
        auto branch = std::make_unique<CaseBranchNode>();

        branch->identifier = consume(TokenType::OBJECT_ID, "Expected variable name in case branch").value;
        consume(TokenType::COLON, "Expected ':' after variable name");
        branch->type_name = consume(TokenType::TYPE_ID, "Expexted variable type in case branch").value;
        consume(TokenType::DARROW, "Expected '=>' after assignment");
        branch->expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after expr");

        return branch;
    }

    //----------------------------------------------------------------------------------------
    // New ::= new TYPE
    std::unique_ptr<ExpressionNode> Parser::parseNew() {
        consume(TokenType::NEW, "Expected 'new'");

        // Allow both TYPE_ID and SELF_TYPE
        std::string type;
        if (check(TokenType::TYPE_ID)) {
            type = consume(TokenType::TYPE_ID, "Expected type").value;
        } else if (check(TokenType::SELF_TYPE)) {
            type = consume(TokenType::SELF_TYPE, "Expected type").value;
        } else {
            throw std::runtime_error("Expected type name after 'new'");
        }

        auto new_node = std::make_unique<NewNode>(type);
        return new_node;
    }

    //----------------------------------------------------------------------------------------
    // IsVoid ::=  isvoid expr
    std::unique_ptr<ExpressionNode> Parser::parseIsVoid() {
        consume(TokenType::ISVOID, "Expected 'isvoid'");

        auto isvoid_node = std::make_unique<IsVoidNode>();
        isvoid_node->expr = parseExpression();

        return isvoid_node;
    }

    //----------------------------------------------------------------------------------------
    // parsePrimary() will be used to handle atomic exprs
    // - Identifiers:     x, self, Main, String
    // - Literals:        42, "hello", true, false
    // - Parentheses:     (x + y)
    // - Blocks:          { expr1; expr2; }
    // - Control flow:    if...then...else...fi, while...loop...pool
    // - Declarations:    let x: Int in..., case expr of...esac
    // - Object creation: new Type, isvoid expr

    std::unique_ptr<ExpressionNode> Parser::parsePrimary() {

        // check if it is identifier or method caalls
        if (check(TokenType::OBJECT_ID)) {
            std::string name = current().value;
            match(TokenType::OBJECT_ID);

            // check if it is method call like method() or self.method()
            if (check(TokenType::LPAREN)) {
                auto dispatch = std::make_unique<DispatchNode>();
                dispatch->object = std::make_unique<IdentifierNode>("self"); // implicit
                dispatch->method_name = name;

                consume(TokenType::LPAREN, "Expected '('");
                if (!check(TokenType::RPAREN)) {
                    do {
                        dispatch->arguments.push_back(parseExpression());
                    }while (match(TokenType::COMMA));
                }

                consume(TokenType::RPAREN, "Expected ')'");
                return dispatch;
            }
            // return this if only just identifier
            return std::make_unique<IdentifierNode>(name);
        }

        // handle Type_ID like Main, IO
        else if (check(TokenType::TYPE_ID)) {
            std::string name = current().value;
            match(TokenType::TYPE_ID);
            return std::make_unique<IdentifierNode>(name);
        }

        // handle integer literals (0 , 42, 12)
        else if (check(TokenType::INTEGER)) {
            int value = std::stoi(current().value);
            match(TokenType::INTEGER);
            return std::make_unique<IntegerNode>(value);
        }

        // handle string literals("hello bro")
        else if (check(TokenType::STRING)) {
            std::string value = current().value;
            match(TokenType::STRING);
            return std::make_unique<StringNode>(value);
        }

        // handle boolean true
        else if (check(TokenType::TRUE)) {
            match(TokenType::TRUE);
            return std::make_unique<BoolNode>(true);
        }

        // handle boolean false
        else if (check(TokenType::FALSE)) {
            match(TokenType::FALSE);
            return std::make_unique<BoolNode>(false);
        }

        // handle self
        else if (check(TokenType::SELF)) {
            match(TokenType::SELF);
            return std::make_unique<IdentifierNode>("self");
        }

        // handle parenthesized expression (x+y)
        else if (match(TokenType::LPAREN)) {
            auto expr = parseExpression();
            consume(TokenType::RPAREN, "Expected ')'");
            return expr;
        }

        // handle if stmt
        else if (check(TokenType::IF)) {
            return parseIf();
        }

        // handle while loop
        else if (check(TokenType::WHILE)) {
            return parseWhile();
        }

        // handle expr block { epr1, expr2 }
        else if (check(TokenType::LBRACE)) {
            return parseBlock();
        }

        // handle let
        else if (check(TokenType::LET)) {
            return parseLet();
        }

        // handle case
        else if (check(TokenType::CASE)) {
            return parseCase();
        }

        // handel new
        else if (check(TokenType::NEW)) {
            return parseNew();
        }

        // handle isVoid
        else if (check(TokenType::ISVOID)) {
            return parseIsVoid();
        }


        // throw exception if nothing matches
        throw std::runtime_error("Unexpected token: " + current().value);

    }

    //----------------------------------------------------------------------------------------
    // Unary operators
    std::unique_ptr<ExpressionNode> Parser::parseUnary() {
        if (check(TokenType::TILDE) || check(TokenType::NOT)) {
            TokenType op = current().type;
            match(op);
            auto unary = std::make_unique<UnaryOpNode>();
            unary->op = op;
            unary->expr = parseUnary();
            return unary;

        }

        auto expr = parsePrimary();

        // incase it is a dispatch
        while (check(TokenType::DOT) || check(TokenType::AT)) {
            expr = parseDispatch(std::move(expr));
        }

        return expr;
    }

    //----------------------------------------------------------------------------------------
    // Binary operators
    std::unique_ptr<ExpressionNode> Parser::parseBinaryOp(
        std::unique_ptr<ExpressionNode> left,
        int min_precedence) {

        while (true) {
            TokenType op = current().type;
            if (!isBinaryOp(op) || getPrecedence(op) < min_precedence) {
                break;
            }

            consume(op, "Expected operator");
            auto right = parseUnary();

            while (isBinaryOp(current().type) &&
                getPrecedence(current().type) > getPrecedence(op)) {

                right = parseBinaryOp(std::move(right), getPrecedence(current().type));
            }

            auto binary = std::make_unique<BinaryOpNode>();
            binary->op = op;
            binary->left = std::move(left);
            binary->right = std::move(right);
            left = std::move(binary);
        }

        return left;
    }



} // namespace cool
