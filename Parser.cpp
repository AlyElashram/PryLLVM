#include "Parser.hpp"
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
#include <map>
using namespace llvm;
std::unique_ptr<Expr> Parser::parseUnaryExpression()
{
	return std::unique_ptr<Expr>();
}
std::unique_ptr<Expr> Parser::number() {
	const Token& curTok = getToken();

	auto val = std::visit([this](auto&& arg) -> std::unique_ptr<Expr> {
		using T = std::decay_t<decltype(arg)>; // Determine the type of arg

		if constexpr (std::is_same_v<T, int>) {
			return parseIntExpression();
		}
		else if constexpr (std::is_same_v<T, double>) {
			return parseDoubleExpression();
		}
		// This will never be visited
		else{
			std::cout << "Parsed Identifier inside a number parser";
			return nullptr;
		}
		}, curTok.getValue());
	return std::move(val);
}

std::unique_ptr<Expr> Parser::parseIntExpression()
{
	const Token& curTok = getToken();
	// el mafrood hena hayerga3ly int
	auto val = std::get<int>(curTok.getValue());
	auto result = std::make_unique<intExpr>(val);
	advance();
	return std::move(result);
}

std::unique_ptr<Expr> Parser::parseDoubleExpression()
{
	const Token& curTok = getToken();
	// el mafrood hena hayerga3ly double
	auto val = std::get<double>(curTok.getValue());
	auto result = std::make_unique<doubleExpr>(val);
	advance();
	return std::move(result);
}

std::unique_ptr<Expr> Parser::parseIf()
{
	advance(); // eat if
	if (getToken().getType() != tok_left_paren) {
		std::cout << "Expected '(' after if";
		return nullptr;
	}
	advance(); // eat (
	auto condition = parseExpression();
	if (!condition) {
		std::cout << "Failed To parse condition expression";
	}
	if (getToken().getType() != tok_right_paren) {
		std::cout << "Expected ')' after if condition";
		return nullptr;
	}
	advance(); // eat )
	auto thenBlock = parseExpression();
	if(!thenBlock) {
		std::cout << "Syntax Error when parsing the IF block";
		return nullptr;
	}
	// TODO: Handle Nested Ifs inside the else block and then return the value
	if (getToken().getType() == tok_else) {
		advance(); // eat else
		/*if(getToken().getType() == tok_if) {
			auto nestedIf = parseIf();
			if(!nestedIf) {
				std::cout << "Syntax Error when parsing the nested IF block";
				return nullptr;
			}
		}*/
		auto elseBlock = parseExpression();
		if (!elseBlock) {
			std::cout << "Syntax Error when parsing the ELSE block";
			return nullptr;
		}
		return std::make_unique<IfExpr>(std::move(condition), std::move(thenBlock), std::move(elseBlock));
	}
	return std::make_unique<IfExpr>(std::move(condition), std::move(thenBlock), nullptr);
}

std::unique_ptr<Expr> Parser::parseIdentifier()
{
	const Token& curTok = getToken();
	// el mafrood hena hayerga3ly string
	auto val = std::get<std::string>(curTok.getValue());
	advance();
	auto currentToken = getToken();
	if (currentToken.getType() != tok_left_paren) {
		// Not a Call for a function then you can make it a variable reference
		return std::make_unique<VariableExpr>(val);
	}
	// else it's a function we need to parse the arguments
	// advance to eat the opening bracket (
	advance();
	std::vector<std::unique_ptr<Expr>> Args;
	while (true) {
		// Arguments can be expressions
		if (auto Arg = parseExpression()) {
			// if Parse Expression doesn't return a null ptr
			Args.push_back(std::move(Arg));
		}
		else {
			// Could Not Parse the expression probably an error occured
			return nullptr;
		}
		if (getToken().getType() == tok_right_paren) {
			// We reached the end of the function call
			break;
		}
		if (getToken().getType() != tok_comma) {
			std::cout << "Expected ')' or ',' in argument list";
			return nullptr;
		}
		// Eat the comma and repeat the loop
		advance();
	}

	// Eat the ')'.
	advance();
	// Return a Call Expression with val equal to the function name and Args as the arguments
	return std::make_unique<CallExpr>(val, std::move(Args));
}

std::unique_ptr<Expr> Parser::parseGroupingExpression()
{
	// eat '(' 
	advance();
	auto expression = parseExpression();
	if (!expression)
		return nullptr;
	const Token& curTok = getToken();
	if (curTok.getType() != tok_right_paren) {
		std::cout << "Expected ')' after expression";
		return nullptr;
	}
	advance(); // eat ).
	return expression;

}

std::unique_ptr<Expr> Parser::parsePrimary() {
	const Token& curTok = getToken();
	switch (curTok.getType()) {
	case tok_identifier:
		return parseIdentifier();
	case tok_number:
		return number();
	case tok_left_paren:
		return parseGroupingExpression();
	case tok_if:
		return parseIf();
	case tok_eof:
	default:
		std::cout << "Unknown token when expecting an expression";
		return nullptr;
	};

}
int Parser::GetTokPrecedence()
{
	const Token& curTok = getToken();
	auto it = BinopPrecedence.find(curTok.getType());

	if (it == BinopPrecedence.end()) {
		return -1;
	}
	return it->second;
}
std::unique_ptr<Expr> Parser::parseBinOpRHS(int ExprPrec,
	std::unique_ptr<Expr> LHS) {
	// If this is a binop, find its precedence.
	while (true) {

		int TokPrec = GetTokPrecedence();
		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec) {
			return LHS;
		}
		int BinOp = getToken().getType();
		advance();  // eat binop

		if (BinOp == tok_eof) {
			int x = 0;
		}

		// Parse the primary expression after the binary operator.
		auto RHS = parsePrimary();
		if (!RHS)
			return nullptr;
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
			if (!RHS)
				return nullptr;
		}
		// Merge LHS/RHS.
		LHS = std::make_unique<BinaryExpr>(BinOp, std::move(LHS),
			std::move(RHS));
	}
}
std::unique_ptr<Expr> Parser::parseExpression() {
	auto LHS = parsePrimary();
	if (!LHS)
		return nullptr;

	if (auto val = parseBinOpRHS(0, std::move(LHS))) {
		return val;
	}
}
/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> Parser::parsePrototype()
{
	if (getToken().getType() != tok_identifier) {
		std::cout << "Expected function name in prototype";
		return nullptr;
	}

	std::string FnName = std::get<std::string>(getToken().getValue());
	advance();

	if (getToken().getType() != tok_left_paren) {
		std::cout << "Expected '(' in prototype";
		return nullptr;
	}
	advance();  // eat '('.

	// Read the list of argument names.
	std::vector<std::string> ArgNames;
	while (getToken().getType() == tok_identifier) {
		ArgNames.push_back(std::get<std::string>(getToken().getValue()));
		advance();
	}
	if (getToken().getType() != tok_right_paren) {
		std::cout << "Expected ')' in prototype";
		return nullptr;
	}
	// success.
	advance();  // eat ')'.

	return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}
/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> Parser::parseDefinition()
{
	advance();  // eat def.
	auto Proto = parsePrototype();
	if (!Proto) return nullptr;
	//Eat Opening Curly Brace
	if (getToken().getType() != tok_left_brace) {
		std::cout << "Expected '{' in function definition";
		return nullptr;
	}
	advance();
	auto E = parseExpression();
	if (!E) {
		return nullptr;
	}

	//Eat Closing Curly Brace
	if (getToken().getType() != tok_right_brace) {
		std::cout << "Expected '}' to close function definition";
		return nullptr;
	}
	return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern()
{
	advance();  // eat extern.
	return parsePrototype();
}
std::unique_ptr<FunctionAST> Parser::parseTopLevelExpression() {
	if (auto E = parseExpression()) {
		// Make an anonymous proto.
		auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
		return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	}
	return nullptr;
}

void Parser::mainLoop() {
	while (!isAtEnd()) {
		switch (getToken().getType()) {
		case tok_eof:
			return;
		case tok_def:
			if (auto fn = parseDefinition()) {
				fn->codegen();
			}
			else {
				// Skip token for error recovery.
				advance();
			}
			break;
		case tok_extern:
			if (auto fn = ParseExtern()) {
				fn->codegen();
			}
			else {
				// Skip token for error recovery.
				advance();
			}
			break;
		default:
			if (auto fn = parseTopLevelExpression()) {
				fn->codegen();
			}
			else {
				// Skip token for error recovery.
				advance();
			}
			break;
		}
	}
}

