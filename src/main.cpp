#include <iostream>
#include "cool/Lexer.hpp"
#include "cool/Parser.hpp"

int main(const int argc, char** argv) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<filePath>" << '\n';
        return -1;
    }

    const std::string filePath = argv[1];


    try {
        cool::Lexer lex(filePath);
        auto tokens = lex.tokenize();
        lex.printTokens();

        cool::Parser parser(tokens);
        const auto ast = parser.parse();

        std::cout << "\n=== AST ===\n";
        std::cout << "AST pointer: " << ast.get() << '\n';
        if (ast) {
            std::cout << "Number of classes: " << ast->classes.size() << '\n';
            ast->print(0);
        } else {
            std::cout << "AST is NULL!\n";
        }


    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }



    return 0;

}