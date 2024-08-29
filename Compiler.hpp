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
#include "llvm/IR/Verifier.h"
#include "Expr.hpp"
#include <map>
using namespace llvm;
class Compiler {
	std::unique_ptr<LLVMContext> TheContext;
	std::unique_ptr<IRBuilder<>> Builder; 
	std::unique_ptr<Module> TheModule; 
	std::map<std::string, Value*> NamedValues;
    Compiler() {
        TheContext = std::make_unique<llvm::LLVMContext>();
        TheModule = std::make_unique<llvm::Module>("my cool jit", *TheContext);
        Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
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
    };
    // Delete copy constructor and assignment operator
    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    std::map<std::string, Value*>& getNamedValues() { return NamedValues; }
	Value* emitInt(int value);
	Value* emitDouble(double value);
	Value* emitAddition(Value* Left, Value* Right);
	Value* emitSubtraction(Value* Left, Value* Right);
	Value* emitMultiplication(Value* Left, Value* Right);
	Value* emitDivision(Value* Left, Value* Right);
    Value* emitLessThan(Value* Left, Value* Right);
    Value* emitLessThanOrEqual(Value* Left, Value* Right);
    Value* emitGreaterThan(Value* Left, Value* Right);
    Value* emitGreaterThanOrEqual(Value* Left, Value* Right);
    Value* emitEquality(Value* Left, Value* Right);
    Value* emitInequality(Value* Left, Value* Right);
    Value* emitIfThenElse(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> then, std::unique_ptr<Expr> Else);
    Function* getFunction(std::string name) {
        return TheModule->getFunction(name);
    };
    Function* checkFunctionExists(std::string name){
        Function* func = TheModule->getFunction(name);
        if(!func) {
			return nullptr;
		}
        return func;
    };
    Value* emitCall(Function* F, std::vector<Value*> args , std::string name);
    Function* emitPrototype(const std::string& name, std::vector<std::string> args);
    Function* emitFunction(Function* TheFunction, std::unique_ptr<Expr> Body);
    void printIR() {
    	bool invalid = verifyModule(*TheModule,&errs());
    	if(invalid) {
    		errs() << "Encountered invalid Module";
    	}else {
    		errs() << "Valid Module Check ";
    		TheModule->print(errs(), nullptr);
    	}
    	}
};