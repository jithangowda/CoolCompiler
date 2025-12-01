#include "cool/CodeGenerator.hpp"
#include "cool/Lexer.hpp"
#include "cool/Parser.hpp"
#include <filesystem>

#include <iostream>

namespace fs = std::filesystem;

std::string generateOutputFilename(const std::string &inputFile,
                                   const std::string &outputDir = "") {

  fs::path inputPath(inputFile);
  std::string stem = inputPath.stem().string();

  std::string outputFilename = "IR_" + stem + ".ll";

  if (!outputDir.empty()) {
    fs::path outputPath(outputDir);
    outputPath /= outputFilename;
    return outputPath.string();
  }

  return outputFilename;
}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.cl> [output_dir]\n";
    std::cerr << "Examples:\n";
    std::cerr << "  " << argv[0] << " program.cl\n";
    std::cerr << "  " << argv[0] << " program.cl ./output\n";
    std::cerr << "  " << argv[0] << " examples/maths.cl\n";
    std::cerr << "\nOutput: Creates IR_<filename>.ll in current or specified "
                 "directory\n";
    return 1;
  }

  const std::string inputFile = argv[1];
  std::string outputDir = "";

  if (argc > 2) {
    outputDir = argv[2];

    if (!fs::exists(outputDir)) {
      fs::create_directories(outputDir);
    }
  }

  std::string outputFile = generateOutputFilename(inputFile, outputDir);

  try {
    std::cout << "COOL Compiler\n";
    std::cout << "=============\n";
    std::cout << "Input:  " << inputFile << "\n";
    std::cout << "Output: " << outputFile << "\n\n";

    // 1. Lexical analysis
    std::cout << "[1/3] Lexing... ";
    cool::Lexer lexer(inputFile);
    auto tokens = lexer.tokenize();
    std::cout << "OK (" << tokens.size() << " tokens)\n";
    std::cout << "\nTokens:\n";
    lexer.printTokens();
    std::cout << "==============================\n\n";

    // 2. Parsing
    std::cout << "[2/3] Parsing... ";
    cool::Parser parser(tokens);
    auto ast = parser.parse();
    std::cout << "AST pointer: " << ast.get() << '\n';
    if (ast)
      ast->print(0);
    std::cout << "OK\n";
    std::cout << "==============================\n\n";

    // 3. Code generation
    std::cout << "[3/3] Generating LLVM IR... ";
    cool::CodeGenerator generator;
    generator.generate(ast.get());

    // Write to file
    generator.writeToFile(outputFile);
    std::cout << "OK\n\n";

    // success mssg if working
    std::cout << "Success! Generated " << outputFile << "\n\n";
    std::cout << "To compile and run:\n";
    std::cout << "  clang " << outputFile << " -o program\n";
    std::cout << "  ./program\n";
    std::cout << "  echo $?   # View return value\n\n";

  } catch (const std::exception &e) {
    std::cerr << "\nError: " << e.what() << '\n';
    return 1;
  }

  return 0;
}