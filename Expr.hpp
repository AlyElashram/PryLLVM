#ifndef expr_hpp
#define expr_hpp
#include <string>
#include <vector>
#include <map>
#include "Token.hpp"
#include "llvm/IR/BasicBlock.h"
class Expr {
public:
	virtual ~Expr() = default;
	virtual llvm::Value* codegen() = 0;
};

class intExpr : public Expr {
	int mVal;

public:
	intExpr(const int& Val) : mVal(Val) {};
	llvm::Value* codegen() override;
};

class doubleExpr : public Expr {
	double mVal;

public:
	doubleExpr(const double& Val) : mVal(Val) {};
	llvm::Value* codegen() override;
};

class VariableExpr : public Expr {
	std::string Name;
public:
	VariableExpr(const std::string& Name) : Name(Name) {}
	llvm::Value* codegen() override;
};
class GroupingExpr : public Expr {
	std::unique_ptr<Expr> LHS;
public:
	GroupingExpr(std::unique_ptr<Expr> LHS) : LHS(std::move(LHS)) {};
	llvm::Value* codegen() { return LHS->codegen(); };
};
class BinaryExpr : public Expr {
	int op;
	std::unique_ptr<Expr> LHS, RHS;

public:
	BinaryExpr(int Op, std::unique_ptr<Expr> LHS,
		std::unique_ptr<Expr> RHS)
		: op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {};
	llvm::Value* codegen() override;
};

// represents something like !true or -a 
class UnaryExpr : public Expr {
	Token op;
	std::unique_ptr<Expr> LHS;
public:
	UnaryExpr(Token op, std::unique_ptr<Expr> LHS) : op(op), LHS(std::move(LHS)) {};
	llvm::Value* codegen() { return nullptr; };
};
class IfExpr : public Expr {
	std::unique_ptr<Expr> Cond, Then, Else;
	public:
		IfExpr(std::unique_ptr<Expr> Cond, std::unique_ptr<Expr> Then, std::unique_ptr<Expr> Else) : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {};
		llvm::Value* codegen() { return nullptr; };
};
class BlockExpr : public Expr {
	std::vector<std::unique_ptr<Expr>> mExprs;
public:
	BlockExpr(std::vector<std::unique_ptr<Expr>> Exprs) : mExprs(std::move(Exprs)) {};
	llvm::Value* codegen() { return nullptr; };
};
class CallExpr : public Expr {
	std::string Callee;
	std::vector<std::unique_ptr<Expr>> Args;

public:
	CallExpr(const std::string& Callee,
		std::vector<std::unique_ptr<Expr>> Args)
		: Callee(Callee), Args(std::move(Args)) {};
	llvm::Value* codegen() override;
};
class PrototypeAST {
	std::string Name;
	std::vector<std::string> Args;

public:
	PrototypeAST(const std::string& Name, std::vector<std::string> Args)
		: Name(Name), Args(std::move(Args)) {}

	const std::string& getName() const { return Name; }
	llvm::Function* codegen();
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<Expr> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto,
		std::unique_ptr<Expr> Body)
		: Proto(std::move(Proto)), Body(std::move(Body)) {}
	llvm::Function* codegen();
};
#endif 