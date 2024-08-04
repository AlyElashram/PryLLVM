#include "Expr.hpp"
#include "Compiler.hpp"
using namespace llvm;

Value* intExpr::codegen() {
	return Compiler::getInstance().emitInt(this->mVal);
}
Value* doubleExpr::codegen() {
	return Compiler::getInstance().emitDouble(this->mVal);
}
Value* VariableExpr::codegen() {
	// Look this variable up in the function.
	Value* V = Compiler::getInstance().getNamedValues()[Name];
	if (!V)
		std::cout<< "Unknown variable name";
	return V;
}
Value* BinaryExpr::codegen() {
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if (!L || !R)
        return nullptr;

    switch (op) {
    case tok_plus:
        return Compiler::getInstance().emitAddition(L,R);
    case tok_minus:
        return Compiler::getInstance().emitSubtraction(L, R);
    case tok_star:
        return Compiler::getInstance().emitMultiplication(L, R);
    case tok_slash:
        return Compiler::getInstance().emitDivision(L, R);
    default:
        std::cout << "invalid binary operator";
        return nullptr;
    }
}
Value* CallExpr::codegen() {
    // Look up the name in the global module table.
    Function* CalleeF = Compiler::getInstance().getFunction(Callee);
    if (!CalleeF) {
        std::cout << "Unknown function referenced";
        return nullptr;
    }
    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size()) {
    	std::cout << "Incorrect # arguments passed";
		return nullptr;
    }
    std::vector<Value*> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back())
            return nullptr;
    }
    return Compiler::getInstance().emitCall(CalleeF, ArgsV, "calltmp");
}
Function* PrototypeAST::codegen() {
    return Compiler::getInstance().emitPrototype(Name, Args);
}
Function* FunctionAST::codegen() {
    // First, check for an existing function from a previous 'extern' declaration.
    Function* func = Compiler::getInstance().checkFunctionExists(Proto->getName());
    if (!func) func = Proto->codegen();
    if (!func) {
        std::cout << "Failed To creat function prototype";
        return nullptr;
    }
    if (!func->empty()) {
        std::cout << "Function cannot be redefined.";
        return (Function*)nullptr;
    }
    Value* retVal = Body->codegen();
    if (!retVal) {
        std::cout << "Failed to generate function body";
    }
    return Compiler::getInstance().emitFunction(func,retVal);
    
}