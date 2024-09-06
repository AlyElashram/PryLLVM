#ifndef COMPILER_HPP
#define COMPILER_HPP
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <memory>
#include "llvm/IR/Verifier.h"
#include "Expr.hpp"
#include <map>
using namespace llvm;
class Compiler {
	std::unique_ptr<LLVMContext> TheContext;
	std::unique_ptr<IRBuilder<>> Builder; 
	std::unique_ptr<Module> TheModule;
  // Keep a vector of maps here , each map represents a scope
  // we add a scope when we enter a block wether that be a function block or an if block or an else block
  // we pop a scope when we leave a block
  // any operations that happen in between will only happen in this scope(Map)
  // when we want to fetch a variable in the the Map we need to traverse the chain of scopes then
  // will creating the alloca in the functions in different locations actually affect the outcome?
  // what about edge cases such as var a(this resembeles the local a) = a(this resembles the global a)
	std::vector<std::map<std::string, AllocaInst*>> NamedValues;
    Compiler() {
        TheContext = std::make_unique<llvm::LLVMContext>();
        TheModule = std::make_unique<llvm::Module>("my cool jit", *TheContext);
        Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
        //NamedValues.push_back({});
    }
public:
    static Compiler& getInstance() {
        static Compiler instance;
        return instance;
    }

    ~Compiler() {
      TheContext.release();
      Builder.release();
      TheModule.release();
    }
    // Delete copy constructor and assignment operator
    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    std::vector<std::map<std::string, AllocaInst *>> &getNamedValues() {
      return NamedValues;
    }
    AllocaInst *CreateEntryBlockAlloca(Function *TheFunction,
                                       StringRef VarName);
    AllocaInst *getNamedValue(const std::string &name);
    Value *emitLoad(AllocaInst *alloca_inst, const std::string &var_name);
    Value *emitAbsolute(Value *value);
    Value *emitInt(int value);
    Value *emitDouble(double value);
    Value *emitAddition(Value *Left, Value *Right);
    Value *emitSubtraction(Value *Left, Value *Right);
    Value *emitMultiplication(Value *Left, Value *Right);
    Value *emitDivision(Value *Left, Value *Right);
    Value *emitLessThan(Value *Left, Value *Right);
    Value *emitLessThanOrEqual(Value *Left, Value *Right);
    Value *emitGreaterThan(Value *Left, Value *Right);
    Value *emitGreaterThanOrEqual(Value *Left, Value *Right);
    Value *emitEquality(Value *Left, Value *Right);
    Value *emitInequality(Value *Left, Value *Right);
    Value *emitIfThenElse(std::unique_ptr<Expr> condition,
                          std::unique_ptr<Expr> then,
                          std::unique_ptr<Expr> Else);
    Constant *emitForLoop(const std::string &varName,
                          std::unique_ptr<Expr> Start,
                          std::unique_ptr<Expr> End, std::unique_ptr<Expr> Step,
                          std::unique_ptr<Expr> Body);
    Function *getFunction(std::string name) {
      return TheModule->getFunction(name);
    };
    Function *checkFunctionExists(std::string name) {
      Function *func = TheModule->getFunction(name);
      if (!func) {
        return nullptr;
      }
      return func;
    };
    Value *emitCall(Function *F, std::vector<Value *> args, std::string name);
    Function *emitPrototype(const std::string &name,
                            std::vector<std::string> args);
    Function *emitFunction(Function *TheFunction, std::unique_ptr<Expr> Body);
    Value *emitNegation(Value *value);
    void StoreValueInVariable(Value *val, Value *value);
    void createScope();
    void popScope();
    Value *
    emitVar(const std::vector<std::pair<std::string, std::unique_ptr<Expr>>>
                &VarNames);
    void printIR() {
      bool invalid = verifyModule(*TheModule, &errs());
      if (invalid) {
        errs() << "Encountered invalid Module";
      } else {
        errs() << "Valid Module Check ";
        TheModule->print(errs(), nullptr);
        std::error_code EC;
        raw_fd_ostream out("/home/aly/Desktop/LLVMPRY/output.ll", EC,
                           sys::fs::OF_Text);

        // Check if file opening was successful
        if (EC) {
          errs() << "Error opening file: " << EC.message() << "\n";
        }
        // Print the module to the file
        TheModule->print(out, nullptr);
      }
    }
};
#endif