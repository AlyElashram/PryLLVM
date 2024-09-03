#ifndef token_h
#define token_h
#include "TokenType.hpp"
#include <string>
#include <iostream>
#include <ostream>
#include <optional>
#include <variant>



class Token {
	TokenType mTokenType;
	std::variant<std::monostate, int, double, std::string> value;
	std::string lexeme;
	int mLine;

public:
	Token() = default;
	Token(TokenType type, const std::variant<std::monostate, int, double, std::string> val, std::string lexeme, const int mLine);
	inline const int getLine() { return this->mLine; };
	inline const std::variant< std::monostate, int, double, std::string> getValue() const { return value; };
	inline const int getLine() const { return this->mLine; };
	inline TokenType getType() { return this->mTokenType; };
	inline const TokenType getType() const { return this->mTokenType; };
	inline std::string getLexeme() { return this->lexeme; };
	inline const std::string getLexeme() const { return this->lexeme; };
	void printVariantType(const std::variant<std::monostate, int, double, std::string>& var);
	friend std::ostream& operator<< (std::ostream& os, Token& obj);
};
#endif