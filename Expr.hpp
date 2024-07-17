#ifndef expr_hpp
#define expr_hpp
#include <string>
#include <vector>
#include "Token.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

class Expr {
public:
	virtual ~Expr() = default;
	virtual Value* codegen() = 0;
};

class intExpr : public Expr {
	int mVal;

public:
	intExpr(const int& Val) : mVal(Val) {};
};

class doubleExpr : public Expr {
	double mVal;

public:
	doubleExpr(const double& Val) : mVal(Val) {};
};

class VariableExpr : public Expr {
	std::string Name;
public:
	VariableExpr(const std::string& Name) : Name(Name) {}
};
class GroupingExpr : public Expr {
	std::unique_ptr<Expr> LHS;
public:
	GroupingExpr(std::unique_ptr<Expr> LHS) : LHS(std::move(LHS)) {}
};
class BinaryExpr : public Expr {
	int op;
	std::unique_ptr<Expr> LHS, RHS;

public:
	BinaryExpr(int Op, std::unique_ptr<Expr> LHS,
		std::unique_ptr<Expr> RHS)
		: op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

// represents something like !true or -a 
class UnaryExpr : public Expr {
	Token op;
	std::unique_ptr<Expr> LHS;
public:
	UnaryExpr(Token op, std::unique_ptr<Expr> LHS) : op(op), LHS(std::move(LHS)) {}
};



#endif
