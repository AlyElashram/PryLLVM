#include "Compiler.hpp"
#include "Expr.hpp"
using namespace llvm;

Value * Compiler::emitNegation(Value *value) {
	return Builder->CreateFNeg(value);
}
void Compiler::StoreValueInVariable(Value *val, Value *variable) {
  Builder->CreateStore(val, variable);
}
Value *Compiler::emitVar(
    const std::vector<std::pair<std::string, std::unique_ptr<Expr>>> &VarNames) {
	std::vector<AllocaInst *> OldBindings;

	Function *TheFunction = Builder->GetInsertBlock()->getParent();
	// Register all variables and emit their initializer.
	for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
		const std::string &VarName = VarNames[i].first;
		Expr *Init = VarNames[i].second.get();
		// Emit the initializer before adding the variable to scope, this prevents
		// the initializer from referencing the variable itself, and permits stuff
		// like this:
		//  var a = 1 in
		//    var a = a in ...   # refers to outer 'a'.
		Value *InitVal;
		if (Init) {
			InitVal = Init->codegen();
			if (!InitVal)
				return nullptr;
		} else { // If not specified, use 0.0.
			InitVal = ConstantFP::get(*TheContext, APFloat(0.0));
		}
		AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
		Builder->CreateStore(InitVal, Alloca);
		return Alloca;
	}
}

AllocaInst *Compiler::CreateEntryBlockAlloca(Function *TheFunction,
                                             StringRef VarName) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type::getDoubleTy(*TheContext), nullptr, VarName);
}
Value *Compiler::emitLoad(AllocaInst *alloca_inst, const std::string& var_name) {
	return Builder->CreateLoad(alloca_inst->getAllocatedType(),alloca_inst,var_name.c_str());
}
Value * Compiler::emitInt(int value) {
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

Constant* Compiler::emitForLoop(const std::string &VarName, std::unique_ptr<Expr> Start, std::unique_ptr<Expr> End,
	std::unique_ptr<Expr> Step, std::unique_ptr<Expr> Body) {

  Function *TheFunction = Builder->GetInsertBlock()->getParent();
  // Create an alloca for the variable in the entry block.
  AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);

  // Emit the start code first, without 'variable' in scope.
  Value *StartVal = Start->codegen();
  if (!StartVal)
    return nullptr;

  // Store the value into the alloca.
  Builder->CreateStore(StartVal, Alloca);

  // Make the new basic block for the loop header, inserting after current
  // block.
  BasicBlock *LoopBB = BasicBlock::Create(*TheContext, "loop", TheFunction);

  // Insert an explicit fall through from the current block to the LoopBB.
  Builder->CreateBr(LoopBB);

  // Start insertion in LoopBB.
  Builder->SetInsertPoint(LoopBB);

  // Within the loop, the variable is defined equal to the PHI node.  If it
  // shadows an existing variable, we have to restore it, so save it now.
  AllocaInst *OldVal = NamedValues[VarName];
  NamedValues[VarName] = Alloca;

  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  if (!Body->codegen())
    return nullptr;

  // Emit the step value.
  Value *StepVal = nullptr;
  if (Step) {
    StepVal = Step->codegen();
    if (!StepVal)
      return nullptr;
  } else {
    // If not specified, use 1.0.
    StepVal = ConstantFP::get(*TheContext, APFloat(1.0));
  }

  // Compute the end condition.
  Value *EndCond = End->codegen();
  if (!EndCond)
    return nullptr;

  // Reload, increment, and restore the alloca.  This handles the case where
  // the body of the loop mutates the variable.
  Value *CurVar =
      Builder->CreateLoad(Alloca->getAllocatedType(), Alloca, VarName.c_str());
  Value *NextVar = Builder->CreateFAdd(CurVar, StepVal, "nextvar");
  Builder->CreateStore(NextVar, Alloca);

  // Convert condition to a bool by comparing non-equal to 0.0.
  EndCond = Builder->CreateFCmpONE(
      EndCond, ConstantFP::get(*TheContext, APFloat(0.0)), "loopcond");

  // Create the "after loop" block and insert it.
  BasicBlock *AfterBB =
      BasicBlock::Create(*TheContext, "afterloop", TheFunction);

  // Insert the conditional branch into the end of LoopEndBB.
  Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  Builder->SetInsertPoint(AfterBB);

  // Restore the unshadowed variable.
  if (OldVal)
    NamedValues[VarName] = OldVal;
  else
    NamedValues.erase(VarName);

  // for expr always returns 0.0.
  return Constant::getNullValue(Type::getDoubleTy(*TheContext));
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
        for (auto &Arg : TheFunction->args()) {
          // Create an alloca for this variable.
          AllocaInst *Alloca =
              CreateEntryBlockAlloca(TheFunction, Arg.getName());

          // Store the initial value into the alloca.
          Builder->CreateStore(&Arg, Alloca);

          // Add arguments to variable symbol table.
          NamedValues[std::string(Arg.getName())] = Alloca;
        }

	// Finish off the function.
	Value* retVal = Body->codegen();
	if(!retVal) {
		std::cout << "Failed to generate a value for the function";
		return nullptr;
	}
	Builder->CreateRet(retVal);

	// Validate the generated code, checking for consistency.
	errs() << "Running Function Verifier " << "\n";
	if(verifyFunction(*TheFunction, &errs())) {
		errs() << "Found errors in function";
	}
	return TheFunction;

}
