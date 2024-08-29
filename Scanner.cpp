#include "Scanner.hpp"
#include "Token.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
static size_t index = 0;
Scanner::Scanner() {
	Scanner::mCurrentLine = 0;
	Scanner::mSourceCode = "";
}

void Scanner::readFile(const std::string& path) {
	std::ifstream fileStream(path, std::ios::binary);
	if (fileStream.fail())
	{
		std::cout << "Failed to read source code";
		exit(1);
	}
	std::string sourceCode;
	std::string str;
	while (std::getline(fileStream, str))
	{
		sourceCode += str;
		sourceCode.push_back('\n');
	}
	Scanner::setSourceCode(sourceCode);
	fileStream.close();
}
void Scanner::advance() {
	index++;
}
char Scanner::peek() {
	return this->mSourceCode[index];
}

char Scanner::peekNext() {
	if (isAtEnd()) return '\0';
	return this->mSourceCode[1];
}

bool Scanner::isAtEnd() {
	return index == this->mSourceCode.size();
}
bool Scanner::match(const char& charToMatch) {
	if (this->isAtEnd()) {
		return false;
	}
	if (this->mSourceCode[index + 1] != charToMatch) return false;
	++index;
	return true;
}
void Scanner::addToken(Token token) {
	this->mTokens.emplace_back(token);
}

void Scanner::number() {
	std::string wholeNum;
	while (std::isdigit(peek())) {
		wholeNum.push_back(peek());
		advance();
	}
	std::string fractional;
	// Floating point number
	if (peek() == '.') {
		advance();
		while (std::isdigit(peek())) {
			fractional.push_back(peek());
			advance();
		}
		addToken(Token(tok_number, std::stod(wholeNum + '.' + fractional) , wholeNum + '.' + fractional, this->mCurrentLine));
		return;
	}
	Token token = Token(tok_number, std::stoi(wholeNum), wholeNum, this->mCurrentLine);
	addToken(token);
}
void Scanner::identifier() {
	size_t startingIndex = index;
	while (std::isalpha(peek()) || std::isdigit(peek())) advance();
	TokenType type = identifierType(startingIndex, index);
	std::string lexeme = this->mSourceCode.substr(startingIndex, index - startingIndex);
	addToken(Token(type, lexeme,lexeme, this->mCurrentLine));

}
void Scanner::string() {
	size_t startingIndex = index;
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') ++(this->mCurrentLine);
		advance();
	}

	if (isAtEnd()) {
		std::cout << this->mCurrentLine << index << "Unterminated String";
		return;
	}

	// The closing quote.
	advance();
	std::string lexeme = this->mSourceCode.substr(startingIndex, index - startingIndex);
	addToken(Token(tok_string,lexeme,lexeme, this->mCurrentLine));

}

TokenType Scanner::checkKeyword(int start, int length,
	size_t TokenStart, size_t current,
	const std::string& rest, TokenType type) {
	// I want to compare the string I have
	// With the string in the source code

	const std::string& myStr = this->mSourceCode.substr(TokenStart + 1, current - TokenStart - 1);
	// ma3aya rest hena
	// comapre lengths of both 
	// Compare strings
	bool flag = myStr.compare(rest) == 0;
	if (flag)
	{
		return type;
	}

	return tok_identifier;
}

TokenType Scanner::identifierType(size_t start, size_t current) {
	switch (this->mSourceCode[start]) {
	case 'a': return checkKeyword(1, 2, start, current, std::string("nd"), tok_and);
	case 'c': return checkKeyword(1, 4, start, current, "lass", tok_class);
	case'd': return checkKeyword(1, 2, start, current, "ef", tok_def);
	case 'e': 
		if (start - current > 1) {
			switch (this->mSourceCode[start + 1]) {
			case 'l':
				return checkKeyword(2, 3, start, current, "lse", tok_else);
			case 'x':
				return checkKeyword(2, 4, start, current, "xtern", tok_extern);
			}
		}
		break;
	case 'f':
		if (start - current > 1) {
			switch (this->mSourceCode[start + 1]) {
			case 'a': return checkKeyword(2, 3, start+1, current, "lse", tok_false);
			case 'o': return checkKeyword(2, 1, start+1, current, "r", tok_for);
			case 'u': return checkKeyword(2, 1, start+1, current, "n", tok_fun);
			}
		}
		break;

	case 'i': return checkKeyword(1, 1, start, current, "f", tok_if);
	case 'n': return checkKeyword(1, 2, start, current, "il", tok_nil);
	case 'o': return checkKeyword(1, 1, start, current, "r", tok_or);
	case 'p': return checkKeyword(1, 4, start, current, "rint", tok_print);
	case 'r': return checkKeyword(1, 5, start, current, "eturn", tok_return);
	case 's': return checkKeyword(1, 4, start, current, "uper", tok_super);
	case 't':
		if (current - start > 1) {
			switch (this->mSourceCode[start + 1]) {
			case 'h': return checkKeyword(2, 2, start+1, current, "is", tok_this);
			case 'r': return checkKeyword(2, 2, start+1, current, "ue", tok_true);
			}
		}
		break;
	case 'v': return checkKeyword(1, 2, start, current, std::string("ar"), tok_var);
	case 'w': return checkKeyword(1, 4, start, current, "hile", tok_while);
	}
	return tok_identifier;
}


void Scanner::getTokens() {
	const std::string& sourceCode = Scanner::getSourceCode();
	while (!isAtEnd()) {

		switch (peek()) {
		case '*':
			addToken(Token(tok_star,std::monostate{}, "*", this->mCurrentLine));
			advance();
			break;
		case '(':
			addToken(Token(tok_left_paren, std::monostate{}, "(", this->mCurrentLine));
			advance();
			break;
		case ')':
			addToken(Token(tok_right_paren, std::monostate{}, ")", this->mCurrentLine));
			advance();
			break;
		case '{':
			addToken(Token(tok_left_brace, std::monostate{}, "{", this->mCurrentLine));
			advance();
			break;
		case '}':
			addToken(Token(tok_right_brace, std::monostate{}, "}", this->mCurrentLine));
			advance();
			break;
		case ',':
			addToken(Token(tok_comma, std::monostate{}, ",", this->mCurrentLine));
			advance();
			break;
		case '.':
			addToken(Token(tok_dot, std::monostate{}, ".", this->mCurrentLine));
			advance();
			break;
		case '-':
			addToken(Token(tok_minus, std::monostate{}, "-", this->mCurrentLine));
			advance();
			break;
		case '+':
			addToken(Token(tok_plus, std::monostate{}, "+", this->mCurrentLine));
			advance();
			break;
		case ';':
			addToken(Token(tok_semicolon, std::monostate{}, ";", this->mCurrentLine));
			advance();
			break;
		case '!':
			if (match('=')) {
				addToken(Token(tok_bang_equal, std::monostate{}, "!=", this->mCurrentLine));
				advance();
				break;
			}
			addToken(Token(tok_bang, std::monostate{}, "!", this->mCurrentLine));
			advance();
			break;
		case '<':
			if (match('=')) {
				addToken(Token(tok_less_equal, std::monostate{}, "<=", this->mCurrentLine));
				advance();
				break;
			}
			addToken(Token(tok_less, std::monostate{}, "<", this->mCurrentLine));
			advance();
			break;
		case '>':
			if (match('=')) {
				addToken(Token(tok_greater_equal, std::monostate{}, ">=", this->mCurrentLine));
				advance();
				break;
			}
			addToken(Token(tok_greater, std::monostate{}, ">", this->mCurrentLine));
			advance();
			break;
		case '/':
			// Comment entire line
			if (match('/')) {
				while (peek() != '\n' && !isAtEnd()) {
					advance();
				}
				break;
			}
			addToken(Token(tok_slash, std::monostate{}, "/", this->mCurrentLine));
			advance();
			break;
		case '=':
			if (match('=')) {
				addToken(Token(tok_equal_equal, std::monostate{}, "==", this->mCurrentLine));
				advance();
				break;
			}
			addToken(Token(tok_equal, std::monostate{}, "=", this->mCurrentLine));
			advance();
			break;
		case '\n':
			++(this->mCurrentLine);
			advance();
			break;

		default:
			char lastChar = peek();
			if (std::isdigit(lastChar)) { number(); }
			else if (std::isspace(lastChar)) {
				advance();
			}
			else if (std::isalpha(lastChar)) { identifier(); }
			else {
				// Unmatched Characters
				std::cout << this->mCurrentLine << " Unrecognized Character Supplied: "  << peek();
			}
			break;
		}
	}
	if (isAtEnd()) {
		addToken(Token(tok_eof, std::monostate{}, "\0",this->mCurrentLine));
		return;
	}
}


void Scanner::scan(const std::string& path) {
	this->readFile(path);
	this->getTokens();
}
std::ostream& operator<<(std::ostream&, Scanner& obj) {
	for (Token tok : obj.mTokens) {
		std::cout << tok;
	}
	return std::cout;
}

