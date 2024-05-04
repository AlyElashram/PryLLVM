#include "Token.hpp"
#include <iostream>
Token::Token(const TokenType type, int mLine) {
	Token::mTokenType = type;
	Token::mLine = mLine;
}

Token::Token(TokenType type, const std::optional<double> numVal, const int mLine) {
	this->mTokenType = type;
	this->mLine = mLine;
	this->mNumberVal = std::optional<double>(numVal);
}
Token::Token(TokenType type, const std::optional<std::string> identStr, const int mLine) {
	this->mTokenType = type;
	this->mLine = mLine;
	this->mIdentifierStr = std::optional<std::string>(std::move(identStr));
}

std::ostream& operator<< (std::ostream& os, Token& obj) {
 	std::cout << "current Token: " << obj.getType() << "\n";
	std::cout << "Token Line: " << obj.getLine() << "\n";
	if (obj.getVal()) {
		std::cout << "Token Value: " << obj.getVal().value() << "\n";
	}
	if (obj.getName()) {
		std::cout << "Token Identifier: " << obj.getName().value() << "\n";
	}
	std::cout << "-------------------------------" << "\n";
	
	return std::cout;
}

