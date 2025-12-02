//
// Created by jithan on 18/11/25.
//
// jit later
//
#include "cool/AST.hpp"

namespace cool {

static int ASTline{0};
// print func helper func to print indent
static void printIndent(int indent) {
    std::cout << "[" << ASTline++ << "]";
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
}

//----------------------------------------------------------------------------------------
// Program node
void ProgramNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Program:\n";
    for (const auto &cls : classes) {
        cls->print(indent + 1);
    }
}

//----------------------------------------------------------------------------------------
// Class node
void ClassNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Class: " << name;
    if (!parent.empty()) {
        std::cout << " inherits " << parent;
    }
    std::cout << '\n';

    for (const auto &feature : features) {
        feature->print(indent + 1);
    }
}

//----------------------------------------------------------------------------------------
// Attributr node
void AttributeNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Attribute: " << name << " : " << type;
    if (init_expr) {
        std::cout << " <- ";
        init_expr->print(0);
    }

    std::cout << '\n';
}

//----------------------------------------------------------------------------------------
// Method node
void MethodNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Method: " << name << "(";

    // formals
    for (size_t i = 0; i < formals.size(); ++i) {
        if (i > 0)
            std::cout << ",";
        std::cout << formals[i].first << " : " << formals[i].second;
    }

    std::cout << ") : " << return_type << '\n';

    printIndent(indent + 1);
    std::cout << "Body:\n";

    if (body) {
        body->print(indent + 2);
    }
}

//----------------------------------------------------------------------------------------
// Identifier node
void IdentifierNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Identifier: " << name << '\n';
}

//----------------------------------------------------------------------------------------
// Integer node
void IntegerNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Integer: " << value << '\n';
}

//----------------------------------------------------------------------------------------
// String node
void StringNode::print(int indent) const {
    printIndent(indent);
    std::cout << "String: \"" << value << "\"" << '\n';
}

//----------------------------------------------------------------------------------------
// Bool node
void BoolNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Bool: " << (value ? "true" : "false") << '\n';
}

//----------------------------------------------------------------------------------------
// New node
void NewNode::print(int indent) const {
    printIndent(indent);
    std::cout << "New: " << type_name << '\n';
}

//----------------------------------------------------------------------------------------
// IsVoid node

void IsVoidNode::print(int indent) const {
    printIndent(indent);
    std::cout << "IsVoid ";
    if (expr) {
        expr->print(indent + 1);
    }
}

//----------------------------------------------------------------------------------------
// Assignment node

void AssignmentNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Assignment: " << identifier << '\n';

    if (expr) {
        expr->print(indent + 1);
    }
}

//----------------------------------------------------------------------------------------
// Dispatch node
void DispatchNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Dispatch " << method_name << '\n';

    printIndent(indent + 1);
    std::cout << "Object:\n";
    if (object) {
        object->print(indent + 2);
    }

    if (!arguments.empty()) {
        printIndent(indent + 1);
        std::cout << "Arguments:\n";
        for (const auto &arg : arguments) {
            arg->print(indent + 2);
        }
    }
}

//----------------------------------------------------------------------------------------
// StaticDispatch node
void StaticDispatchNode::print(int indent) const {
    printIndent(indent);
    std::cout << "StaticDispatch " << method_name << " @ " << type_name << '\n';

    printIndent(indent + 1);
    std::cout << "Object:\n";
    if (object) {
        object->print(indent + 2);
    }

    if (!arguments.empty()) {
        printIndent(indent + 1);
        std::cout << "Arguments:\n";
        for (const auto &arg : arguments) {
            arg->print(indent + 2);
        }
    }
}

//----------------------------------------------------------------------------------------
// If node
void IfNode::print(int indent) const {
    printIndent(indent);
    std::cout << "If:\n";

    printIndent(indent + 1);
    std::cout << "Condition:\n";
    if (condition) {
        condition->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "Then:\n";
    if (then_branch) {
        then_branch->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "Else:\n";
    if (else_branch) {
        else_branch->print(indent + 2);
    }
}

//----------------------------------------------------------------------------------------
// While node
void WhileNode::print(int indent) const {
    printIndent(indent);
    std::cout << "While:\n";

    printIndent(indent + 1);
    std::cout << "Condition:\n";
    if (condition) {
        condition->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "Body:\n";
    if (body) {
        body->print(indent + 2);
    }
}

//----------------------------------------------------------------------------------------
// Block Node

void BlockNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Block:\n";
    for (const auto &expr : expressions) {
        expr->print(indent + 1);
    }
}

//----------------------------------------------------------------------------------------
// Let Node

void LetNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Let:\n";

    printIndent(indent + 1);
    std::cout << "Bindings:\n";
    for (const auto &binding : bindings) {
        printIndent(indent + 2);
        std::cout << binding.identifier << " : " << binding.type_name;
        if (binding.init_expr) {
            std::cout << " <- ";
            binding.init_expr->print(0);
        }
        std::cout << "\n";
    }

    printIndent(indent + 1);
    std::cout << "Body:\n";
    if (body) {
        body->print(indent + 2);
    }
}

//----------------------------------------------------------------------------------------
// CaseBranch Node

void CaseBranchNode::print(int indent) const {
    printIndent(indent);
    std::cout << "CaseBranch: " << identifier << " : " << type_name << " =>\n";
    if (expr) {
        expr->print(indent + 1);
    }
}

//----------------------------------------------------------------------------------------
// Case Node

void CaseNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Case:\n";

    printIndent(indent + 1);
    std::cout << "Expression:\n";
    if (expr) {
        expr->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "Branches:\n";
    for (const auto &branch : branches) {
        branch->print(indent + 2);
    }
}

//----------------------------------------------------------------------------------------
// BinaryOp Node

void BinaryOpNode::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryOp: " << tokensToString(op) << "\n";

    printIndent(indent + 1);
    std::cout << "Left:\n";
    if (left) {
        left->print(indent + 2);
    }

    printIndent(indent + 1);
    std::cout << "Right:\n";
    if (right) {
        right->print(indent + 2);
    }
}

//----------------------------------------------------------------------------------------
// UnaryOp Node

void UnaryOpNode::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryOp: " << tokensToString(op) << "\n";

    printIndent(indent + 1);
    std::cout << "Expression:\n";
    if (expr) {
        expr->print(indent + 2);
    }
}

} // namespace cool
