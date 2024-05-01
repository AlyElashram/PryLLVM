#ifndef token_h
#define token_h
#include "TokenType.hpp"
#include <string>
#include <iostream>
#include <ostream>
#include <optional>


template<class T>
class Token {

	TokenType mTokenType;

	// Stores the actual source code string of the expr
	// Operator or string or class or whatever
	std::optional<std::string> mLexeme;

	// Stores the actual value of the expression 
	// Double Value or String or whatever
	std::optional<T> mliteral;
	int mLine;

public:
	Token() = default;
	Token(TokenType type, const int mLine);
	Token(TokenType type, const std::optional<double> numVal, const int mLine);
	Token(TokenType type, const std::optional<std::string> identStr, const int mLine);

	// Constructor for operators and lexemes
	Token(const TokenType type, const std::optional<char> lexeme, const int mLine);
	inline  std::optional<double>& getVal() { return this->mNumberVal; };
	inline int getLine() { return this->mLine; };
	inline TokenType getType() { return this->mTokenType; };
	friend std::ostream& operator<< (std::ostream& os, Token& obj);
};
#endif // !token