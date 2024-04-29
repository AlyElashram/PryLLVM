#ifndef token_h
#define token_h
#include "TokenType.hpp"
#include <string>
#include <iostream>
#include <ostream>
#include <optional>



class Token {
	
	TokenType mTokenType;

	 // Lexeme is only used for operators and strings
	 std::optional<std::string> lexeme;

	 // If identifier type then it has an identifier name
	 std::optional<std::string> mIdentifierStr;

	 // If number then it's value is here
	 std::optional<double> mNumberVal;
	 
	 int mLine;

	public:	
		Token() = default;
		Token(TokenType type, const int mLine);
		Token(TokenType type,const std::optional<double> numVal, const int mLine);
		Token(TokenType type, const std::optional<std::string> identStr, const int mLine);

		// Constructor for operators and lexemes
		Token(const TokenType type, const std::optional<char> lexeme, const int mLine);
		inline  std::optional<std::string>& getName() { return this->mIdentifierStr;};
		inline  std::optional<double>& getVal() { return this->mNumberVal; };
		inline int getLine() { return this->mLine; };
		inline TokenType getType() { return this->mTokenType; };
		friend std::ostream& operator<< (std::ostream& os, Token& obj);
}; 
#endif // !token