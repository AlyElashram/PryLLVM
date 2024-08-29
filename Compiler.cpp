#include "Compiler.hpp"
#include "Expr.hpp"
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
Value* Compiler::emitLessThan(Value* Left, Value* Right) {
	Left = Builder->CreateFCmpULT(Left, Right, "cmptmp");
	return Builder->CreateUIToFP(Left, Type::getDoubleTy(*TheContext), "booltmp");
}
Value* Compiler::emitLessThanOrEqual(Value* Left, Value* Right)
{
	return Builder->CreateFCmpULE(Left, Right, "cmptmp");
}
Value* Compiler::emitGreaterThan(Value* Left, Value* Right)
{
	return Builder->CreateFCmpUGT(Left, Right, "cmptmp");
}
Value* Compiler::emitGreaterThanOrEqual(Value* Left, Value* Right)
{
	return Builder->CreateFCmpUGE(Left, Right, "cmptmp");
}
Value* Compiler::emitEquality(Value* Left, Value* Right)
{
	return Builder->CreateFCmpUEQ(Left, Right, "cmptmp");
}
Value* Compiler::emitInequality(Value* Left, Value* Right)
{
	return Builder->CreateFCmpUNE(Left, Right, "cmptmp");
}
Value* Compiler::emitIfThenElse(std::unique_ptr<Expr> condition,std::unique_ptr<Expr> then , std::unique_ptr<Expr> Else) {

	Value* condV = condition->codegen();
	if(!condV) {
		errs() << "Failed to generate condition";
	}
	Function* TheFunction = Builder->GetInsertBlock()->getParent();

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.

	// Implicitly adds the then block into the end of the function
	BasicBlock* ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
	
	// Free Floating blocks for now , we don't want to insert them until we finish the if to handle nesting
	BasicBlock* ElseBB = BasicBlock::Create(*TheContext, "else");
	BasicBlock* MergeBB = BasicBlock::Create(*TheContext, "ifcont");
	
	// Create the conditional branch instruction.
	Builder->CreateCondBr(condV, ThenBB, ElseBB);
	Builder->SetInsertPoint(ThenBB);
	Value* ThenVal = then->codegen();

	if (!ThenVal) {
		std::cout << "Failed to generate a value for the then block";
		return nullptr;
	}
	
	// Create an unconditional branch to terminate the 'then' block
	Builder->CreateBr(MergeBB);

	// Reset the ThenBB to the current block (Recursive if calls)
	// we update this block to later insert it into the phi node
	ThenBB = Builder->GetInsertBlock();
	
	// Emit else block.
	TheFunction->insert(TheFunction->end(), ElseBB);
	Builder->SetInsertPoint(ElseBB);
	Value* ElseVal = Else->codegen();
	if (!ElseVal) {
		std::cout << "Failed to generate a value for the else block";
		return nullptr;
	}
	// Again, create an unconditional branch to the merge block
	Builder->CreateBr(MergeBB);
	// codegen of 'Else' can change the current block, update ElseBB for the phi node.
	ElseBB = Builder->GetInsertBlock();
	// Emit merge block.
	TheFunction->insert(TheFunction->end(), MergeBB);
	Builder->SetInsertPoint(MergeBB);
	PHINode* PN =
		Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, "iftmp");

	PN->addIncoming(ThenVal, ThenBB);
	PN->addIncoming(ElseVal, ElseBB);
	return PN;
}
Value* Compiler::emitCall(llvm::Function* F, std::vector<Value*> Args, std::string name) {
	return Builder->CreateCall(F, Args, name);
}
Function* Compiler::emitPrototype(const std::string& name, std::vector<std::string> args) {
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
Function* Compiler::emitFunction(Function* TheFunction, std::unique_ptr<Expr> Body)
{
	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
	Builder->SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	NamedValues.clear();
	for (auto& Arg : TheFunction->args())
		NamedValues[std::string(Arg.getName())] = &Arg;

	// Finish off the function.
	Value* retVal = Body->codegen();
	if(!retVal) {
		std::cout << "Failed to generate a value for the function";
		return nullptr;
	}
	Builder->CreateRet(retVal);

	// Validate the generated code, checking for consistency.
	verifyFunction(*TheFunction);

	return TheFunction;

}
