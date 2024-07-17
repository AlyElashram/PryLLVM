#include "Token.hpp"
#include <iostream>
Token::Token(const TokenType type, std::variant<std::monostate, int, double, std::string> val, std::string lexeme,int mLine) {
	Token::mTokenType = type;
	Token::mLine = mLine;
    Token::value = std::move(val);
    Token::lexeme = std::move(lexeme);
}

void printVariantType(const std::variant<std::monostate, int, double, std::string>& var) {
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>; // Determine the type of arg

        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "null" << std::endl; // std::monostate representing "null"
        }
        else if constexpr (std::is_same_v<T, int>) {
            std::cout << "int: " << arg << std::endl;
        }
        else if constexpr (std::is_same_v<T, double>) {
            std::cout << "double: " << arg << std::endl;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "string: " << arg << std::endl;
        }
        }, var);
}

std::ostream& operator<< (std::ostream& os, Token& obj) {
 	std::cout << "current Token: " << obj.getType() << std::endl;
	std::cout << "Token Line: " << obj.getLine() << std::endl;
    std::cout << "Token value: ";
    printVariantType(obj.value);
    std::cout << "Lexeme: " << obj.lexeme << std::endl;
	std::cout << "-------------------------------" << std::endl;
	return std::cout;
}