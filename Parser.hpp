#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "ErrorReporter.hpp"
#include "Expr.hpp"
#include <vector>
#include <map>

class Parser {
	std::vector<Token> mTokens;
	ErrorReporter reporter;
	int index = 0;
	std::map<TokenType, int> BinopPrecedence = {
		{tok_less ,10},{tok_greater ,10},{tok_minus,15},{tok_plus,20},{tok_star,40},{tok_slash,50}
	};

public :
	Parser(const std::vector<Token>& tokens, ErrorReporter& reporter) : mTokens(std::move(tokens)) , reporter(reporter) {}
	inline const Token& getToken() { return this->mTokens.at(index);};
	std::unique_ptr<Expr> parseUnaryExpression();
	std::unique_ptr<Expr> parseBinaryExpression();
	std::unique_ptr<Expr> parseIntExpression();
	std::unique_ptr<Expr> parseDoubleExpression();
	std::unique_ptr<Expr> number();
	std::unique_ptr<Expr> parseIdentifier();
	std::unique_ptr<Expr> parseGroupingExpression();
	std::unique_ptr<Expr> parsePrimary();
	std::unique_ptr<Expr> parseExpression();
	std::unique_ptr<Expr> parseBinOpRHS(int ExprPrec,
		std::unique_ptr<Expr> LHS);
	int GetTokPrecedence();
	inline void advance() { ++index; };
};



#endif // !PARSER_HPP
