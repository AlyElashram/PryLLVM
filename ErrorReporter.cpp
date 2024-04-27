#include "ErrorReporter.hpp"
#include <iostream>
#include <string>


ErrorReporter::ErrorReporter()
{
	this->hasResolverError = false;
	this->hasSyntaxError = false;
}
void ErrorReporter::error(const int& line, const std::string& message) {
	std::cout << message + " \n" << "At Line: " << line << " \n";
}
void ErrorReporter::error(const int& line, const int& column,const std::string& message) {
	std::cout << message + " \n" << "At Line: " << line << " \n" << "At index: " << column;
}
