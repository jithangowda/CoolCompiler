#include "cool/CodeGenerator.hpp"
#include "cool/AST.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Triple.h>

namespace cool {

CodeGenerator::CodeGenerator()
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>("CoolModule", *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context)) {

  declareRuntimeFunctions();
}

//----------------------------------------------------------------------------------------
void CodeGenerator::declareRuntimeFunctions() {
  llvm::Type *int8PtrType = llvm::PointerType::getUnqual(*context);
  llvm::FunctionType *printfType = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(*context), {int8PtrType}, true);
  printfFunc = llvm::Function::Create(
      printfType, llvm::Function::ExternalLinkage, "printf", module.get());

  llvm::FunctionType *mallocType = llvm::FunctionType::get(
      int8PtrType, {llvm::Type::getInt64Ty(*context)}, false);
  mallocFunc = llvm::Function::Create(
      mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
}

//----------------------------------------------------------------------------------------
// generate main IR from AST
void CodeGenerator::generate(ProgramNode *program) {
  // Generate main function from program AST
  llvm::FunctionType *mainType =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
  llvm::Function *mainFunc = llvm::Function::Create(
      mainType, llvm::Function::ExternalLinkage, "main", module.get());

  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(*context, "entry", mainFunc);
  builder->SetInsertPoint(entry);

  variables.clear();

  // find Main class
  ClassNode *mainClass = nullptr;
  for (auto &cls : program->classes) {
    if (cls->name == "Main") {
      mainClass = cls.get();
      break;
    }
  }

  if (!mainClass) {
    throw std::runtime_error(
        "Error: No 'Main' class found in program. "
        "COOL requires a class named 'Main' with a 'main()' method.");
  }

  // init attributes from Main class
  for (auto &feature : mainClass->features) {
    if (auto attr = dynamic_cast<AttributeNode *>(feature.get())) {
      if (attr->init_expr) {
        llvm::Value *initValue = generateExpr(attr->init_expr.get());
        if (!initValue) {
          initValue =
              llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
        }
        variables[attr->name] = initValue;
      } else {
        variables[attr->name] =
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
      }
    }
  }

  // find and execute main method in Main class
  bool mainMethodFound = false;
  for (auto &feature : mainClass->features) {
    if (auto method = dynamic_cast<MethodNode *>(feature.get())) {
      if (method->name == "main") {
        llvm::Value *result = generateExpr(method->body.get());

        if (result && result->getType() == llvm::Type::getInt32Ty(*context)) {
          builder->CreateRet(result);
        } else {
          builder->CreateRet(
              llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
        }
        mainMethodFound = true;
        break;
      }
    }
  }

  if (!mainMethodFound) {
    throw std::runtime_error(
        "Error: No 'main()' method found in Main class. "
        "COOL requires a 'main()' method in the Main class.");
  }

  std::string error;
  llvm::raw_string_ostream errorStream(error);
  if (llvm::verifyFunction(*mainFunc, &errorStream)) {
    throw std::runtime_error("Function verification failed: " + error);
  }
}

//----------------------------------------------------------------------------------------
// appropriate expr generator
llvm::Value *CodeGenerator::generateExpr(ExpressionNode *expr) {
  if (!expr) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
  }

  if (auto id = dynamic_cast<IdentifierNode *>(expr)) {
    return generateIdentifier(id);
  } else if (auto intNode = dynamic_cast<IntegerNode *>(expr)) {
    return generateInteger(intNode);
  } else if (auto boolNode = dynamic_cast<BoolNode *>(expr)) {
    return generateBool(boolNode);
  } else if (auto strNode = dynamic_cast<StringNode *>(expr)) {
    return generateString(strNode);
  } else if (auto assign = dynamic_cast<AssignmentNode *>(expr)) {
    return generateAssignment(assign);
  } else if (auto binaryOp = dynamic_cast<BinaryOpNode *>(expr)) {
    return generateBinaryOp(binaryOp);
  } else if (auto ifExpr = dynamic_cast<IfNode *>(expr)) {
    return generateIf(ifExpr);
  } else if (auto whileExpr = dynamic_cast<WhileNode *>(expr)) {
    return generateWhile(whileExpr);
  } else if (auto block = dynamic_cast<BlockNode *>(expr)) {
    return generateBlock(block);
  } else if (auto dispatch = dynamic_cast<DispatchNode *>(expr)) {
    return generateDispatch(dispatch);
  } else if (auto newExpr = dynamic_cast<NewNode *>(expr)) {
    return generateNew(newExpr);
  }

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
}

//----------------------------------------------------------------------------------------
llvm::Value *CodeGenerator::generateInteger(IntegerNode *intNode) {
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context),
                                intNode->value);
}

//----------------------------------------------------------------------------------------
llvm::Value *CodeGenerator::generateBool(BoolNode *boolNode) {
  return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context),
                                boolNode->value);
}

//----------------------------------------------------------------------------------------
llvm::Value *CodeGenerator::generateString(StringNode *strNode) {
  return createStringConstant(strNode->value);
}

//----------------------------------------------------------------------------------------
llvm::Value *CodeGenerator::generateIdentifier(IdentifierNode *id) {
  auto it = variables.find(id->name);
  if (it != variables.end()) {
    return it->second;
  }

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
}

//----------------------------------------------------------------------------------------
llvm::Value *CodeGenerator::generateAssignment(AssignmentNode *assign) {
  llvm::Value *rhs = generateExpr(assign->expr.get());
  variables[assign->identifier] = rhs;
  return rhs;
}

//----------------------------------------------------------------------------------------
// IR for binary operations
llvm::Value *CodeGenerator::generateBinaryOp(BinaryOpNode *binaryOp) {
  llvm::Value *left = generateExpr(binaryOp->left.get());
  llvm::Value *right = generateExpr(binaryOp->right.get());

  if (!left || !right) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
  }

  if (left->getType() != llvm::Type::getInt32Ty(*context)) {

    left = builder->CreateIntCast(left, llvm::Type::getInt32Ty(*context), true);
  }
  if (right->getType() != llvm::Type::getInt32Ty(*context)) {
    right =
        builder->CreateIntCast(right, llvm::Type::getInt32Ty(*context), true);
  }

  switch (binaryOp->op) {
  case TokenType::PLUS:
    return builder->CreateAdd(left, right, "addtmp");
  case TokenType::MINUS:
    return builder->CreateSub(left, right, "subtmp");
  case TokenType::STAR:
    return builder->CreateMul(left, right, "multmp");
  case TokenType::SLASH:
    return builder->CreateSDiv(left, right, "divtmp");
  case TokenType::LESS_THAN:
    return builder->CreateICmpSLT(left, right, "lttmp");
  case TokenType::LESS_EQUAL:
    return builder->CreateICmpSLE(left, right, "letmp");
  case TokenType::EQUAL:
    return builder->CreateICmpEQ(left, right, "eqtmp");
  default:
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
  }
}
//----------------------------------------------------------------------------------------
// IR for IF
llvm::Value *CodeGenerator::generateIf(IfNode *ifExpr) {
  llvm::Value *cond = generateExpr(ifExpr->condition.get());
  if (!cond) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
  }

  llvm::Function *currentFunc = builder->GetInsertBlock()->getParent();
  llvm::BasicBlock *thenBB =
      llvm::BasicBlock::Create(*context, "then", currentFunc);
  llvm::BasicBlock *elseBB =
      llvm::BasicBlock::Create(*context, "else", currentFunc);
  llvm::BasicBlock *mergeBB =
      llvm::BasicBlock::Create(*context, "merge", currentFunc);

  builder->CreateCondBr(cond, thenBB, elseBB);

  builder->SetInsertPoint(thenBB);
  llvm::Value *thenValue = generateExpr(ifExpr->then_branch.get());
  builder->CreateBr(mergeBB);
  llvm::BasicBlock *thenEnd = builder->GetInsertBlock();

  builder->SetInsertPoint(elseBB);
  llvm::Value *elseValue = generateExpr(ifExpr->else_branch.get());
  builder->CreateBr(mergeBB);
  llvm::BasicBlock *elseEnd = builder->GetInsertBlock();

  builder->SetInsertPoint(mergeBB);
  llvm::Type *phiType =
      thenValue ? thenValue->getType() : llvm::Type::getInt32Ty(*context);
  llvm::PHINode *phi = builder->CreatePHI(phiType, 2, "if_result");

  if (thenValue) {
    phi->addIncoming(thenValue, thenEnd);
  } else {
    phi->addIncoming(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0), thenEnd);
  }

  if (elseValue) {
    phi->addIncoming(elseValue, elseEnd);
  } else {
    phi->addIncoming(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0), elseEnd);
  }

  return phi;
}

//----------------------------------------------------------------------------------------
// IR for while
llvm::Value *CodeGenerator::generateWhile(WhileNode *whileExpr) {
  llvm::Function *currentFunc = builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *condBB =
      llvm::BasicBlock::Create(*context, "while_cond", currentFunc);
  llvm::BasicBlock *bodyBB =
      llvm::BasicBlock::Create(*context, "while_body", currentFunc);
  llvm::BasicBlock *endBB =
      llvm::BasicBlock::Create(*context, "while_end", currentFunc);

  builder->CreateBr(condBB);

  builder->SetInsertPoint(condBB);
  llvm::Value *cond = generateExpr(whileExpr->condition.get());
  if (!cond)
    cond = llvm::ConstantInt::getFalse(*context);
  builder->CreateCondBr(cond, bodyBB, endBB);

  builder->SetInsertPoint(bodyBB);
  generateExpr(whileExpr->body.get());
  builder->CreateBr(condBB);

  builder->SetInsertPoint(endBB);

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
}

//----------------------------------------------------------------------------------------
// IR for block
llvm::Value *CodeGenerator::generateBlock(BlockNode *block) {
  llvm::Value *result =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);

  for (auto &expr : block->expressions) {
    result = generateExpr(expr.get());
  }

  return result;
}

//----------------------------------------------------------------------------------------
// Ir for method dispatch
llvm::Value *CodeGenerator::generateDispatch(DispatchNode *dispatch) {
  if (dispatch->method_name == "out_int") {
    if (!dispatch->arguments.empty()) {
      llvm::Value *arg = generateExpr(dispatch->arguments[0].get());
      if (arg) {

        llvm::Value *format = createStringConstant("%d\n");

        if (arg->getType() != llvm::Type::getInt32Ty(*context)) {
          arg = builder->CreateIntCast(arg, llvm::Type::getInt32Ty(*context),
                                       true);
        }
        return builder->CreateCall(printfFunc, {format, arg});
      }
    }
  } else if (dispatch->method_name == "out_string") {
    if (!dispatch->arguments.empty()) {
      llvm::Value *arg = generateExpr(dispatch->arguments[0].get());
      if (arg) {
        return builder->CreateCall(printfFunc, {arg});
      }
    }
  }

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
}

//----------------------------------------------------------------------------------------
// Ir for object creation
llvm::Value *CodeGenerator::generateNew(NewNode *newExpr) {
  if (newExpr->type_name == "Int") {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
  } else if (newExpr->type_name == "Bool") {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), false);
  } else if (newExpr->type_name == "String") {
    return createStringConstant("");
  }

  llvm::Value *size =
      llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 8);
  llvm::Value *ptr = builder->CreateCall(mallocFunc, {size});

  return builder->CreateBitCast(ptr, llvm::PointerType::getUnqual(*context));
}

//----------------------------------------------------------------------------------------
llvm::Value *CodeGenerator::createStringConstant(const std::string &value) {
  llvm::Constant *strConstant =
      llvm::ConstantDataArray::getString(*context, value, true);

  llvm::GlobalVariable *globalStr = new llvm::GlobalVariable(
      *module, strConstant->getType(), true, llvm::GlobalValue::PrivateLinkage,
      strConstant, ".str");

  llvm::Constant *zero =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
  llvm::Constant *indices[] = {zero, zero};
  llvm::Constant *strPtr = llvm::ConstantExpr::getGetElementPtr(
      strConstant->getType(), globalStr, indices, true);

  return strPtr;
}

//----------------------------------------------------------------------------------------
void CodeGenerator::outputIR(llvm::raw_ostream &os) {
  module->print(os, nullptr);
}

//----------------------------------------------------------------------------------------
void CodeGenerator::writeToFile(const std::string &filename) {
  std::error_code ec;
  llvm::raw_fd_ostream out(filename, ec);
  if (ec) {
    throw std::runtime_error("Failed to open file: " + filename);
  }
  outputIR(out);
}

} // namespace cool