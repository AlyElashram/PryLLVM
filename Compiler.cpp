#include "Compiler.hpp"
using namespace llvm;


Value* Compiler::emitInt(int value) {
	return ConstantFP::get(*TheContext, APFloat(value + 0.0));
}
Value* Compiler::emitDouble(double value) {
	return ConstantFP::get(*TheContext, APFloat(value));
}

Value* Compiler::emitAddition(Value* Left, Value* Right)
{
	return Builder->CreateFAdd(Left, Right, "addtmp");
}

Value* Compiler::emitSubtraction(Value* Left, Value* Right)
{
    return Builder->CreateFSub(Left, Right, "subtmp");
}

Value* Compiler::emitMultiplication(Value* Left, Value* Right)
{
	return Builder->CreateFMul(Left, Right, "multmp");
}

Value* Compiler::emitDivision(Value* Left, Value* Right)
{
	return Builder->CreateFDiv(Left, Right, "divtmp");
}
Value* Compiler::emitCall(llvm::Function* F, std::vector<Value*> Args, std::string name) {
	return Builder->CreateCall(F, Args, name);
}
llvm::Function* Compiler::emitPrototype(const std::string& name, std::vector<std::string> args) {
	std::vector<Type*> Doubles(args.size(),
		Type::getDoubleTy(*TheContext));
	FunctionType* FT =
		FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
	Function* F =
		Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto& Arg : F->args())
		Arg.setName(args[Idx++]);
	return F;
}

Function* Compiler::emitFunction(Function* TheFunction, Value* retVal)
{
    // Create a new basic block to start insertion into.
    BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto& Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

        // Finish off the function.
        Builder->CreateRet(retVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        return TheFunction;

}
