#ifndef tokentype_hpp
#define tokentype_hpp

enum TokenType : int {
	// Special Token
	tok_eof = 0,

	// Operators
	tok_plus = 1,
	tok_minus = 2,
	tok_slash = 3,
	tok_star = 4,
	tok_equal = 5,
	tok_equal_equal = 6,
	tok_bang = 7,
	tok_bang_equal = 8,
	tok_greater = 9,
	tok_greater_equal = 10,
	tok_less = 11,
	tok_less_equal = 12,
	tok_left_paren = 13,
	tok_right_paren = 14,
	tok_left_brace = 15,
	tok_right_brace = 16,
	tok_comma = 17,
	tok_dot = 18,
	tok_semicolon = 19,

	// Keywords
	tok_and = 20,
	tok_class = 21,
	tok_else = 22,
	tok_false = 23,
	tok_fun = 24,
	tok_for = 25,
	tok_if = 26,
	tok_nil = 27,
	tok_or = 28,
	tok_print = 29,
	tok_return = 30,
	tok_super = 31,
	tok_this = 32,
	tok_true = 33,
	tok_var = 34,
	tok_while = 35,

	// Primary
	tok_identifier = 36,
	tok_number = 37,
	tok_string = 38,

	// Commands
	tok_def = 39,
	tok_extern = 40,
	tok_in = 41
};

#endif