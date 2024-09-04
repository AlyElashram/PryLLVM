#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Expr.hpp"
#include <vector>
#include <map>

class Parser {
	std::vector<Token> mTokens;
	int index = 0;
	std::map<TokenType, int> BinopPrecedence = {
		{tok_equal,2},{tok_less ,10},{tok_greater ,10},{tok_less_equal,10},{tok_greater_equal,10},{tok_equal_equal,12},{tok_bang_equal,12},{tok_minus,15},{tok_plus,20},{tok_star,40},{tok_slash,50}
	};

public :
	Parser(const std::vector<Token>& tokens) : mTokens(std::move(tokens)){}
	inline const Token& getToken() { return this->mTokens.at(index); };
	inline const bool isAtEnd() { return this->index == this->mTokens.size(); };
	std::unique_ptr<Expr> parseUnaryExpression();
	std::unique_ptr<Expr> parseIntExpression();
	std::unique_ptr<Expr> parseDoubleExpression();
	std::unique_ptr<Expr> parseBlock();
	std::unique_ptr<Expr> parseIf();
	std::unique_ptr<Expr> parseForExpr();
	std::unique_ptr<Expr> number();
	std::unique_ptr<Expr> parseIdentifier();
	std::unique_ptr<Expr> parseGroupingExpression();
        std::unique_ptr<Expr> ParseVarExpr();
        std::unique_ptr<Expr> parsePrimary();
	std::unique_ptr<Expr> parseExpression();
	std::unique_ptr<PrototypeAST> parsePrototype();
	std::unique_ptr<FunctionAST> parseDefinition();
	std::unique_ptr<PrototypeAST> ParseExtern();
	std::unique_ptr<FunctionAST> parseTopLevelExpression();
	void mainLoop();

	std::unique_ptr<Expr> parseBinOpRHS(int ExprPrec,
		std::unique_ptr<Expr> LHS);
	int GetTokPrecedence();
	inline void advance() { ++index; };
};



#endif // !PARSER_HPP
