#ifndef errorreporter_h
#define errorreporter_h

#include "Token.hpp"
#include <string>

class ErrorReporter {
	bool hasSyntaxError;
	bool hasResolverError;

public :
	ErrorReporter();
	void error(const int& line, const std::string& message);
	void error(const int& line,const int& column, const std::string& message);
	void parsingError(const std::string& message);
	inline bool getSyntaxError() { return this->hasSyntaxError; };
	inline void setSyntaxError(bool err) { this->hasSyntaxError = err; };
	
};

#endif