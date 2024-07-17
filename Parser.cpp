#include "Parser.hpp"

std::unique_ptr<Expr> Parser::parseUnaryExpression()
{
	return std::unique_ptr<Expr>();
}

std::unique_ptr<Expr> Parser::parseBinaryExpression()
{
	return std::unique_ptr<Expr>();
}
std::unique_ptr<Expr> Parser::number() {
	const Token& curTok = getToken();

	auto val = std::visit([this](auto&& arg) -> std::unique_ptr<Expr> {
		using T = std::decay_t<decltype(arg)>; // Determine the type of arg

		if constexpr (std::is_same_v<T, int>) {
			return parseIntExpression();
		}
		else if constexpr (std::is_same_v<T, double>) {
			return parseDoubleExpression();
		}
		else if constexpr (std::is_same_v<T, std::string>) {
			return parseIdentifier();
		}
		else if constexpr (std::is_same_v<T, std::monostate>) {
			reporter.parsingError("Why are we sending mono state into a number parser?");
			return nullptr;
		}
		}, curTok.getValue());
	return std::move(val);
}

std::unique_ptr<Expr> Parser::parseIntExpression()
{
	const Token& curTok = getToken();
	// el mafrood hena hayerga3ly int
	auto val = std::get<int>(curTok.getValue());
	auto result = std::make_unique<intExpr>(val);
	advance();
	return std::move(result);
}

std::unique_ptr<Expr> Parser::parseDoubleExpression()
{
	const Token& curTok = getToken();
	// el mafrood hena hayerga3ly double
	auto val = std::get<double>(curTok.getValue());
	auto result = std::make_unique<doubleExpr>(val);
	advance();
	return std::move(result);
}

std::unique_ptr<Expr> Parser::parseIdentifier()
{
	const Token& curTok = getToken();
	// el mafrood hena hayerga3ly string
	auto val = std::get<std::string>(curTok.getValue());
	auto result = std::make_unique<VariableExpr>(val);
	advance();
	return std::move(result);
}

std::unique_ptr<Expr> Parser::parseGroupingExpression()
{
	// eat '(' 
	advance();
	auto expression = parseExpression();
	if (!expression)
		return nullptr;
	const Token& curTok = getToken();
	if (curTok.getType() != tok_right_paren)
		return nullptr;
	advance(); // eat ).
	return expression;

}

std::unique_ptr<Expr> Parser::parsePrimary() {
	const Token& curTok = getToken();
	switch (curTok.getType()) {
	case tok_identifier:
		return parseIdentifier();
	case tok_number:
		return number();
	case tok_left_paren:
		return parseGroupingExpression();
	case tok_eof:
	default:
		return nullptr;
	};
	
}
int Parser::GetTokPrecedence()
{
	const Token& curTok = getToken();
	auto it = BinopPrecedence.find(curTok.getType());

	if (it == BinopPrecedence.end()) {
		return -1;
	}
	return it->second;
}
std::unique_ptr<Expr> Parser::parseBinOpRHS(int ExprPrec,
	std::unique_ptr<Expr> LHS) {
	// If this is a binop, find its precedence.
	while (true) {

		int TokPrec = GetTokPrecedence();
		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec) {
			return LHS;
		}
		int BinOp = getToken().getType();
		advance();  // eat binop

		if (BinOp == tok_eof) {
			int x = 0;
		}

		// Parse the primary expression after the binary operator.
		auto RHS = parsePrimary();
		if (!RHS)
			return nullptr;
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
			if (!RHS)
				return nullptr;
		}
		// Merge LHS/RHS.
		LHS = std::make_unique<BinaryExpr>(BinOp, std::move(LHS),
			std::move(RHS));
	}
}
std::unique_ptr<Expr> Parser::parseExpression() {
	auto LHS = parsePrimary();
	if (!LHS)
		return nullptr;

	return parseBinOpRHS(0, std::move(LHS));
}