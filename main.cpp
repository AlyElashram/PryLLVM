#include "Scanner.hpp"
#include "ErrorReporter.hpp"
#include <iostream>

int main(int argc,char* argv []) {
	static const ErrorReporter reporter = ErrorReporter() ;
	if (argv[1]) {
		std::string path = argv[1];
		Scanner scanner = Scanner(reporter);
		scanner.scan(path);
		int x = 0;
		std::cout << scanner;
	}
	else {
		std::cout << "Invalid usage no path supplied";
	}
	

	return 0;
}