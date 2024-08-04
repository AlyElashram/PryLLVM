#include "Scanner.hpp"
#include "Parser.hpp"
#include "Expr.hpp"
#include <iostream>
#include "Compiler.hpp"

int main(int argc,char* argv []) {
	if (argv[1]) {
		std::string path = argv[1];
		Scanner scanner = Scanner();
		scanner.scan(path);
		Parser parser = Parser(scanner.getTokenVector());
		parser.mainLoop();
		Compiler::getInstance().printIR();
		// std::cout << scanner;
	}
	else {
		std::cout << "Invalid usage no path supplied";
	}

 	return 0;
}
