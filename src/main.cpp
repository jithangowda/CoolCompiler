#include <iostream>
#include "cool/Lexer.hpp"

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

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }



    return 0;

}