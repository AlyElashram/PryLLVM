#include "Scanner.hpp"
#include "Token.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
static size_t index = 0;
Scanner::Scanner(const ErrorReporter& reporter) {
    Scanner::mCurrentLine = 0;
    Scanner::mSourceCode = "";
    Scanner::reporter = reporter;
}

void Scanner::readFile(const std::string& path) {
    std::ifstream fileStream(path,std::ios::binary);
    if(fileStream.fail())
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
        addToken(Token(tok_number, "", std::stod(wholeNum + '.' + fractional), this->mCurrentLine));
        return;
    }
    addToken(Token(tok_number, "", std::stod(wholeNum), this->mCurrentLine));
}
void Scanner::identifier(){
    size_t startingIndex = index;
    while (std::isalpha(peek()) || std::isdigit(peek())) advance();
    TokenType type = identifierType(startingIndex,index);
    switch (type) {
    case tok_identifier:
        addToken(Token(type, this->mSourceCode.substr(startingIndex, index-startingIndex), 0, this->mCurrentLine));
        break;
    default:
        addToken(Token(type, "", 0, this->mCurrentLine));
        break;
    }

}
void Scanner::string() {
    size_t startingIndex = index;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') ++(this->mCurrentLine);
        advance();
    }

    if (isAtEnd()) {
        reporter.error(this->mCurrentLine,index,"Unterminated String");
        reporter.setSyntaxError(true);
        return;
    }

    // The closing quote.
    advance();
    addToken(Token(tok_string, this->mSourceCode.substr(startingIndex, index),0, this->mCurrentLine));

}

TokenType Scanner::checkKeyword(int start, int length,
    size_t TokenStart, size_t current,
    const std::string& rest, TokenType type) {
    // I want to compare the string I have
    // With the string in the source code

    const std::string& myStr = this->mSourceCode.substr(TokenStart+1, current-TokenStart-1);
    // ma3aya rest hena
    // comapre lengths of both 
    // Compare strings
    bool flag = myStr.compare(rest) == 0;
    if (current - TokenStart == start + length && myStr.compare(rest) == 0)
    {
        return type;
    }

    return tok_identifier;
}

TokenType Scanner::identifierType(size_t start, size_t current) {
    switch (this->mSourceCode[start]) {
    case 'a': return checkKeyword(1, 2, start, current, std::string("nd"), tok_and);
    case 'c': return checkKeyword(1, 4, start, current, "lass", tok_class);
    case 'e': return checkKeyword(1, 3, start, current, "lse", tok_else);
    case 'f':
        if (start - current > 1) {
            switch (this->mSourceCode[start + 1]) {
            case 'a': return checkKeyword(2, 3, start, current, "lse", tok_false);
            case 'o': return checkKeyword(2, 1, start, current, "r", tok_for);
            case 'u': return checkKeyword(2, 1, start, current, "n", tok_fun);
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
            case 'h': return checkKeyword(2, 2, start, current, "is", tok_this);
            case 'r': return checkKeyword(2, 2, start, current, "ue", tok_true);
            }
        }
        break;
    case 'v': return checkKeyword(1, 2, start, current, std::string("ar"), tok_var);
    case 'w': return checkKeyword(1, 4, start, current, "hile", tok_while);
    }
    return tok_identifier;
}


void Scanner::getTokens() {
    const std::string & sourceCode = Scanner::getSourceCode();
    while (!isAtEnd()) {
           
        if (std::isdigit(peek())) { number();}
        if (std::isalpha(peek())) { identifier();}

        switch (peek()) {
        case '*':
            addToken(Token(tok_star, "", 0, this->mCurrentLine));
            advance();
            break;
        case '/':
            //Commented entire line
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) {
                    advance();
                }
                break;
            }
            addToken(Token(tok_slash, "", 0, this->mCurrentLine));
            advance();
            break;
        case '=':
            if (match('=')) {
                addToken(Token(tok_equal_equal, "", 0, this->mCurrentLine));
                advance();
                break;
            }
            addToken(Token(tok_equal, "", 0, this->mCurrentLine));
            advance();
            break;
        case '\n':
            ++(this->mCurrentLine);
            advance();
            break;
        default:
            if (std::isspace(peek())) { 
                advance(); 
            }
            else {
                // Unmatched Characters
                reporter.setSyntaxError(true);
                reporter.error(this->mCurrentLine,"Unrecognized Character Supplied: " + peek());
            }
            break;
        }
    }
    if (isAtEnd()) {
        addEOF(Token(tok_eof, "", 0, this->mCurrentLine));
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

