#include "Expr.hpp"
#include "Compiler.hpp"
using namespace llvm;

Value *intExpr::codegen() {
  return Compiler::getInstance().emitInt(this->mVal);
}
Value *doubleExpr::codegen() {
  return Compiler::getInstance().emitDouble(this->mVal);
}
llvm::Value *boolExpr::codegen() {
  return Compiler::getInstance().emitBoolean(this->mVal);
}
Value *VariableExpr::codegen() {
  // Look this variable up in the function.
  AllocaInst *V = Compiler::getInstance().getNamedValue(Name);
  if (!V) {
    std::cout << "Unknown variable name";
    return nullptr;
  }

  return Compiler::getInstance().emitLoad(V, Name);
}
llvm::Value *VarExprAST::codegen() {
  return Compiler::getInstance().emitVar(std::move(VarNames));
}
Value *BinaryExpr::codegen() {
  // Special case '=' because we don't want to emit the LHS as an expression.
  if (op == tok_equal) {
    auto LHSE = static_cast<VariableExpr *>(LHS.get());

    if (!LHSE) {
      errs() << "destination of '=' must be a variable.";
      return nullptr;
    }

    Value *Val = RHS->codegen();
    if (!Val)
      return nullptr;

    // Look up the name.
    Value *Variable = Compiler::getInstance().getNamedValue(LHSE->getName());
    if (!Variable) {
      errs() << "Unknown Variable Name";
      return nullptr;
    }
    Compiler::getInstance().StoreValueInVariable(Val, Variable);

    return Val;
  }
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (op) {
  case tok_plus:
    return Compiler::getInstance().emitAddition(L, R);
  case tok_minus:
    return Compiler::getInstance().emitSubtraction(L, R);
  case tok_star:
    return Compiler::getInstance().emitMultiplication(L, R);
  case tok_slash:
    return Compiler::getInstance().emitDivision(L, R);
  case tok_less:
    return Compiler::getInstance().emitLessThan(L, R);
  case tok_greater:
    return Compiler::getInstance().emitGreaterThan(L, R);
  case tok_less_equal:
    return Compiler::getInstance().emitLessThanOrEqual(L, R);
  case tok_greater_equal:
    return Compiler::getInstance().emitGreaterThanOrEqual(L, R);
  case tok_equal_equal:
    return Compiler::getInstance().emitEquality(L, R);
  case tok_bang_equal:
    return Compiler::getInstance().emitInequality(L, R);
  default:
    std::cout << "invalid binary operator";
    return nullptr;
  }
}
Value *CallExpr::codegen() {
  // Look up the name in the global module table.
  Function *CalleeF = Compiler::getInstance().getFunction(Callee);
  if (!CalleeF) {
    std::cout << "Unknown function referenced";
    return nullptr;
  }
  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size()) {
    std::cout << "Incorrect # arguments passed";
    return nullptr;
  }
  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }
  return Compiler::getInstance().emitCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value *ForExpr::codegen() {

  return Compiler::getInstance().emitForLoop(VarName, std::move(Start),
                                             std::move(End), std::move(Step),
                                             std::move(Body));
}

Function *PrototypeAST::codegen() {
  return Compiler::getInstance().emitPrototype(Name, Args);
}
Function *FunctionAST::codegen() {

  // First, check for an existing function from a previous 'extern' declaration.
  Function *func =
      Compiler::getInstance().checkFunctionExists(Proto->getName());
  if (!func)
    func = Proto->codegen();
  if (!func) {
    std::cout << "Failed To creat function prototype";
    return nullptr;
  }
  if (!func->empty()) {
    std::cout << "Function cannot be redefined.";
    return (Function *)nullptr;
  }

  return Compiler::getInstance().emitFunction(func, std::move(Body));
}

llvm::Value *UnaryExpr::codegen() {
  // TODO:Add other tokenTypes here when we start supporting other data types
  // that have other operators
  auto L = LHS->codegen();
  switch (op.getType()) {
  case tok_minus:
    return Compiler::getInstance().emitNegation(L);
  }
}

llvm::Value *Block::codegen() {
  Value *finalVal = nullptr;
  for (auto const &expr : expressions) {
    finalVal = expr->codegen();
    if (!finalVal) {
      errs() << "Failed To generate expression in a block";
      return nullptr;
    }
  }
  return finalVal;
}

llvm::Value *IfExpr::codegen() {
  Value *PhiNode = Compiler::getInstance().emitIfThenElse(
      std::move(Cond), std::move(Then), std::move(Else));
  if (!PhiNode) {
    std::cout << "Failed to generate condition";
    return nullptr;
  }
  return PhiNode;
}
