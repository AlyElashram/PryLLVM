#include "Scanner.hpp"
#include "ErrorReporter.hpp"
#include "Parser.hpp"
#include <iostream>

int main(int argc,char* argv []) {
	static ErrorReporter reporter = ErrorReporter() ;
	if (argv[1]) {
		std::string path = argv[1];
		Scanner scanner = Scanner(reporter);
		scanner.scan(path);
		Parser parser = Parser(scanner.getTokenVector(), reporter);
		auto ast = parser.parseExpression();
		std::cout << scanner;
	}
	else {
		std::cout << "Invalid usage no path supplied";
	}

	return 0;
}