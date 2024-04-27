#include "Token.hpp"
#include <iostream>
Token::Token(const TokenType type, std::string identStr, double numVal, int mLine) {
	Token::mTokenType = type;
	Token::mIdentifierStr = std::move(identStr);
	Token::mLine = mLine;
	Token::mNumberVal = numVal;
}
std::ostream& operator<< (std::ostream& os, Token& obj) {
 	std::cout << "current Token: " << obj.getType() << "\n";
	std::cout << "Token Line: " << obj.getLine() << "\n";
	std::cout << "Token Value: " << obj.getVal() << "\n";
	std::cout << "Token Identifier: " << obj.getName() << "\n";
	return std::cout;
}

