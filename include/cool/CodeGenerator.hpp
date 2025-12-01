#pragma once

#include "cool/AST.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace cool {

class CodeGenerator {
public:
  CodeGenerator();

  void generate(ProgramNode *program);
  void writeToFile(const std::string &filename);

private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

  llvm::Function *mallocFunc; // new
  llvm::Function *printfFunc; // out_string

  // for storing var values
  std::unordered_map<std::string, llvm::Value *> variables;

  void declareRuntimeFunctions();

  llvm::Value *generateExpr(ExpressionNode *expr);
  llvm::Value *generateInteger(IntegerNode *intNode);
  llvm::Value *generateBool(BoolNode *boolNode);
  llvm::Value *generateString(StringNode *strNode);
  llvm::Value *generateIdentifier(IdentifierNode *id);
  llvm::Value *generateAssignment(AssignmentNode *assign);
  llvm::Value *generateBinaryOp(BinaryOpNode *binaryOp);
  llvm::Value *generateIf(IfNode *ifExpr);
  llvm::Value *generateWhile(WhileNode *whileExpr);
  llvm::Value *generateBlock(BlockNode *block);
  llvm::Value *generateDispatch(DispatchNode *dispatch);
  llvm::Value *generateNew(NewNode *newExpr);

  llvm::Value *createStringConstant(const std::string &value);

  void outputIR(llvm::raw_ostream &os);
};

} // namespace cool