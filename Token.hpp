#ifndef token_h
#define token_h
#include "TokenType.hpp"
#include <string>
#include <iostream>
#include <ostream>
#include <optional>



class Token {
	
	TokenType mTokenType;


	 // contains actual string of charecters for operators and strings
	 std::optional<std::string> mIdentifierStr;

	 // If number then it's value is here
	 std::optional<double> mNumberVal;
	 
	 int mLine;

	public:	
		Token() = default;
		// For EOF only
		Token(TokenType type, const int mLine);

		//For Numbers
		Token(TokenType type,const std::optional<double> numVal, const int mLine);
		
		//For anything else
		Token(TokenType type, const std::optional<std::string> identStr, const int mLine);

	
		inline  std::optional<std::string>& getName() { return this->mIdentifierStr;};
		inline  std::optional<double>& getVal() { return this->mNumberVal; };
		inline const int getLine() { return this->mLine; };
		inline TokenType getType() { return this->mTokenType; };
		friend std::ostream& operator<< (std::ostream& os, Token& obj);
}; 
#endif // !token