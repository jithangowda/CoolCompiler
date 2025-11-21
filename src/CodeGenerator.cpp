//
// Created by jithan on 18/11/25.
//

#include "cool/CodeGenerator.hpp"


namespace cool {

    CodeGenerator::CodeGenerator() {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("CoolProgram", *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);

        voidType = llvm::Type::getVoidTy(*context);
        intType = llvm::Type::getInt32Ty(*context);
        boolType = llvm::Type::getInt1Ty(*context);
        stringType = llvm::PointerType::getUnqual(*context);
        objectType = llvm::PointerType::getUnqual(*context);

        currentClass = "";
        currentObject = nullptr;

        declareRuntimeFunc();
        generateBasicIO();
    }

    //----------------------------------------------------------------------------------------
    // it starts from here
    // setClassHeirarchy will make the class metadata
    // generateProgram will generate the Ir code using the metadata
    void CodeGenerator::generateCode(ProgramNode* program) {
        setClassHierarchy(program);
        generateProgram(program); // to genrate IR code
    }

    //----------------------------------------------------------------------------------------
    // iterates all classes and creates the structType for each class
    void CodeGenerator::setClassHierarchy(ProgramNode* program) {
        for (auto& cls : program->classes) {
            createClassType(cls.get());
        }

        for (auto& cls : program->classes) {
            if (!cls->parent.empty())
                inheritClassInfo(cls->name, cls->parent);

            createVTable(cls->name);
        }
    }

    //----------------------------------------------------------------------------------------

    void CodeGenerator::createClassType(ClassNode* classNode) {
        ClassInfo info;
        info.parent = classNode->parent;

        for (auto& feature : classNode->features) {
            if (auto attr = dynamic_cast<AttributeNode*>(feature.get())) {
                info.attributes.push_back(attr->name);

                if (attr->init_expr)
                    info.attributeInitializers[attr->name] = nullptr; // placeholder till IR is generated for this
            } else if (auto method = dynamic_cast<MethodNode*>(feature.get())) {
                info.methods.push_back(method->name);
            }
        }

        std::vector<llvm::Type*> fieldTypes;
        fieldTypes.push_back(llvm::PointerType::getUnqual(*context));

        for (size_t i = 0; i < info.attributes.size(); ++i) {
            fieldTypes.push_back(intType);
        }

        info.type = llvm::StructType::create(*context, fieldTypes, classNode->name + "_type");
        classInfo[classNode->name] = std::move(info);
    }

    //----------------------------------------------------------------------------------------
    // incase the class is a derived class
    // this adds the methds in prent class and not in child class
    // also does attributes
    void CodeGenerator::inheritClassInfo(const std::string& className, const std::string& parentName) {
        ClassInfo& childInfo = classInfo[className];
        ClassInfo& parentInfo = classInfo[parentName];

        for (const auto& method : parentInfo.methods) {
            if (std::find(childInfo.methods.begin(), childInfo.methods.end(),method)
                == childInfo.methods.end()) {
                childInfo.methods.push_back(method);
            }
        }

        // insert all parent attr to starting
        childInfo.attributes.insert(
            childInfo.attributes.end(),
            parentInfo.attributes.begin(),
            parentInfo.attributes.end());

        for (const auto& init : parentInfo.attributeInitializers) {
            if (childInfo.attributeInitializers.find(init.first)
                == childInfo.attributeInitializers.end()) {
                childInfo.attributeInitializers[init.first] = nullptr;
            }
        }
    }

    //----------------------------------------------------------------------------------------
    // creates the vtable for class
    // vtable is a llvm global var
    // init the vtable global var of struct ClassInfo
    void CodeGenerator::createVTable(const std::string& className) {
        ClassInfo& info = classInfo[className];

        llvm::Type* vTableType = llvm::ArrayType::get(llvm::PointerType::getUnqual(*context),
                                info.methods.size());

        std::string vTableName = className + "_vtable";

        info.vtable = new llvm::GlobalVariable(*module, vTableType, true,
                                                llvm::GlobalValue::ExternalLinkage,
                                                nullptr, vTableName);

        for (size_t i = 0; i < info.methods.size(); ++i) {
            info.methods.push_back(info.methods[i]);
        }
    }

    //----------------------------------------------------------------------------------------





}