#ifndef token_h
#define token_h
#include <string>
#include <iostream>
#include <ostream>


enum TokenType {
	tok_eof = 0 ,

	//operators
	tok_plus=1,
	tok_minus=2,
	tok_slash=3,
	tok_star=4,
	tok_equal=5, 
	tok_equal_equal=6,
	tok_bang=7,
	tok_bang_equal=8,

	// Keywords.
	tok_and=9,
	tok_or=10,
	tok_true=11,
	tok_false=12,

	// commands
	tok_def=13,
	tok_extern=14,

	// primary
	tok_identifier=15,
	tok_string=16,
	tok_number=17,
	tok_class = 18,
	tok_else = 19,
	tok_for = 20,
	tok_print = 21,
	tok_return = 22,
	tok_fun = 23,
	tok_nil = 24,
	tok_if = 25,
	tok_super = 26,
	tok_this = 27,
	tok_var = 28,
	tok_while = 29,
};


class Token {
	 TokenType mTokenType;
	 std::string mIdentifierStr;
	 double mNumberVal;
	 int mLine;

	public:	
		Token() = default;
		Token(TokenType type, std::string identStr,double numVal,int mLine);
		inline std::string& getName() { return this->mIdentifierStr;};
		inline double getVal() { return this->mNumberVal; };
		inline int getLine() { return this->mLine; };
		inline TokenType getType() { return this->mTokenType; };
		friend std::ostream& operator<< (std::ostream& os, Token& obj);
}; 
#endif // !token