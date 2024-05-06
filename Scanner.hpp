#ifndef scanner_hpp
#define scanner_hpp
#include <string>
#include<vector>
#include <iostream>
#include <vector>
#include "Token.hpp"
#include "ErrorReporter.hpp"

class Scanner {
	int mCurrentLine;
	std::string mSourceCode;
	std::vector<Token> mTokens;
	ErrorReporter reporter;

public:
	Scanner(const ErrorReporter& reporter);
	void getTokens();
	void readFile(const std::string& path);
	inline const std::string& getSourceCode() { return this->mSourceCode; };
	inline void setSourceCode(std::string& src) { this->mSourceCode = std::move(src); };
	bool isAtEnd();
	void addToken(Token token);
	bool match(const char& charToMatch);
	void advance();
	char peek();
	char peekNext();
	void number();
	void identifier();
	void string();
	void scan(const std::string& path);
	inline const std::vector<Token>& getTokenVector() { return this->mTokens; };
	void addEOF(Token token) {
		this->addToken(token);
		this->mTokens.shrink_to_fit();
	};
	TokenType identifierType(size_t start, size_t current);
	TokenType checkKeyword(int start, int length,
		size_t current, size_t TokenStart,
		const std::string& rest, TokenType type);
	friend std::ostream& operator<<(std::ostream&, Scanner& obj);
};

#endif