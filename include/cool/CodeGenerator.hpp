//
// Created by jithan on 18/11/25.
//

#pragma once


#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <unordered_map>
#include "cool/AST.hpp"

namespace cool {

    // this if for storing data about each class
    struct ClassInfo {
        llvm::StructType* type;
        llvm::GlobalVariable* vtable;
        std::string parent;
        std::vector<std::string> attributes;
        std::vector<std::string> methods;
        std::unordered_map<std::string, int> methodIndices;
        std::unordered_map<std::string, std::unique_ptr<ExpressionNode>> attributeInitializers;

    };

    class CodeGenerator {
    public:
        CodeGenerator();
        ~CodeGenerator() = default;

        void generateCode(ProgramNode* program);
        void printIR() const;
        void optimizeIR();
        void writeToFile(const std::string& fileName);

    private:
        // LLVM CORE
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::Module> module;

        // Symbo ltable
        std::unordered_map<std::string, llvm::Value*> namedValues;
        std::unordered_map<std::string, llvm::Function*> functions;
        std::unordered_map<std::string, ClassInfo> classInfo;

        // current state
        std::string currentClass;
        llvm::Value* currentObject;

        // Type Definiton
        llvm::Type* voidType;
        llvm::Type*  intType;;
        llvm::Type*  boolType;
        llvm::PointerType*  stringType;
        llvm::PointerType*  objectType;

        // Runtime func declaration
        llvm::Function* mallocFunc; // for new keyword
        llvm::Function* printfFunc; // out_string
        llvm::Function* exitFunc; // exit

        // class structure for inheritance
        void setClassHierarchy(ProgramNode* program); // creates the class hierarchy
        void createClassType(ClassNode* classNode);

        // is a llvm global var that points to vtable array of func pointer for that class
        void createVTable(const std::string& className);

        // copies all from parent class tot child calss
        void inheritClassInfo(const std::string& className, const std::string& parentName);

        // program structure
        void generateProgram(ProgramNode* program);
        void generateClass(ClassNode* classNode);
        void generateMethod(MethodNode* methodNode);
        void generateMainFunttion(ProgramNode* program); // this will make the execution start from Main.main()

        // epresssion code generation
        llvm::Value* generateExpression(ExpressionNode* expr);
        llvm::Value* generateIdentifier(IdentifierNode* id);
        llvm::Value* generateInteger(IntegerNode* integer);
        llvm::Value* generateString(StringNode* str);
        llvm::Value* generateBool(BoolNode* boolNode);
        llvm::Value* generateNew(NewNode* newExpr);
        llvm::Value* generateIsVoid(IsVoidNode* isvoid);
        llvm::Value* generateAssignment(AssignmentNode* assign);
        llvm::Value* generateDispatch(DispatchNode* dispatch);
        llvm::Value* generateStaticDispatch(StaticDispatchNode* dispatch);
        llvm::Value* generateIf(IfNode* ifExpr);
        llvm::Value* generateWhile(WhileNode* whileExpr);
        llvm::Value* generateBlock(BlockNode* block);
        llvm::Value* generateLet(LetNode* letExpr);
        llvm::Value* generateCase(CaseNode* caseExpr);
        llvm::Value* generateBinaryOp(BinaryOpNode* binaryOp);
        llvm::Value* generateUnaryOp(UnaryOpNode* unaryOp);

        // object helpers
        llvm::Value* createObject(const std::string& className);
        llvm::Function* getMethodFromVTable(llvm::Value* object, const std::string& methodName);
        llvm::Value* getAttribute(llvm::Value* object, const std::string& attrName);
        void setAttribute(llvm::Value* object, const std::string& attrName, llvm::Value* value);


        // helper
        void declareRuntimeFunc();
        void generateBasicIO();
        llvm::Type* getLLVMType(const std::string& coolType);
        llvm::Value* createStringConstant(const std::string& str);
        int getMethodIndex(const std::string& className, const std::string& methodName);

    };
}
