#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Lexer.h"

// readFile function
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char **argv) {
    if (argc != 2 ) {
        std::cerr << "Usage: " << argv[0] << " <filepath>\n" ;
        return 1;
    }

    const std::string filePath = argv[1];
    std::string source;

    try {
        source = readFile(filePath);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // std::cout << source << '\n';
    return 0;
}