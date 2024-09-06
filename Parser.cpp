#include "Parser.hpp"
#include "llvm/IR/Verifier.h"
#include <map>
using namespace llvm;
std::unique_ptr<Expr> Parser::parseUnaryExpression() {
  // TODO: Refactor this into a centralized function in the parser that returns
  // a boolean
  //  Function should check if a passed Token& is in one of the unary operators
  if (getToken().getType() != tok_minus) {
    return parsePrimary();
  }
  auto Token = getToken();
  advance();
  auto operand = parseUnaryExpression();

  return std::make_unique<UnaryExpr>(Token, std::move(operand));
}
std::unique_ptr<Expr> Parser::number() {
  const Token &curTok = getToken();

  auto val = std::visit(
      [this](auto &&arg) -> std::unique_ptr<Expr> {
        using T = std::decay_t<decltype(arg)>; // Determine the type of arg

        if constexpr (std::is_same_v<T, int>) {
          return parseIntExpression();
        } else if constexpr (std::is_same_v<T, double>) {
          return parseDoubleExpression();
        }
        // This will never be visited
        else {
          std::cout << "Parsed Identifier inside a number parser";
          return nullptr;
        }
      },
      curTok.getValue());
  return std::move(val);
}

std::unique_ptr<Expr> Parser::parseIntExpression() {
  const Token &curTok = getToken();
  // el mafrood hena hayerga3ly int
  auto val = std::get<int>(curTok.getValue());
  auto result = std::make_unique<intExpr>(val);
  advance();
  return std::move(result);
}

std::unique_ptr<Expr> Parser::parseDoubleExpression() {
  const Token &curTok = getToken();
  // el mafrood hena hayerga3ly double
  auto val = std::get<double>(curTok.getValue());
  auto result = std::make_unique<doubleExpr>(val);
  advance();
  return std::move(result);
}

std::unique_ptr<Expr> Parser::parseBlock() {
  advance(); // Eat Left Curly Brace
  std::vector<std::unique_ptr<Expr>> exprs = {};
  // we should create a new Scope here and add it to the named values

  // Keeps parsing which will definetly ruin the results after right if you
  // forget the brace let's check after if we reached the end and the final
  // token was not a right brace if we didn't find a right brace let's store the
  // beginning line here and report an error on this line
  int begginingLineOfBlock = getToken().getLine();
  while (!isAtEnd() && getToken().getType() != tok_right_brace) {
    auto expr = parseExpression();

    // DO NOT CONSUME A SEMI COLON HERE WE DO NOT WANT SEMICOLONS IN THIS
    // LANGUAGE AFTER EACH EXPRESSION
    if (!expr) {
      errs() << "Failed to parse an expression inside the block";
      return nullptr;
    }
    // should I call move here?
    exprs.push_back(std::move(expr));
  }
  if (getToken().getType() != tok_right_brace) {
    errs() << "Unterminated Block beginning at line : " << begginingLineOfBlock;
  }
  advance(); // Consume Closing brace;
  return std::make_unique<Block>(std::move(exprs));
}

std::unique_ptr<Expr> Parser::parseIf() {
  advance(); // eat if
  if (getToken().getType() != tok_left_paren) {
    std::cout << "Expected '(' after if";
    return nullptr;
  }
  advance(); // eat (
  auto condition = parseExpression();
  if (!condition) {
    std::cout << "Failed To parse condition expression";
  }
  if (getToken().getType() != tok_right_paren) {
    std::cout << "Expected ')' after if condition";
    return nullptr;
  }
  advance(); // eat )
  auto thenBlock = parseBlock();
  if (!thenBlock) {
    std::cout << "Syntax Error when parsing the IF block";
    return nullptr;
  }
  if (getToken().getType() == tok_else) {
    advance(); // eat else
    auto elseBlock = parseBlock();
    if (!elseBlock) {
      std::cout << "Syntax Error when parsing the ELSE block";
      return nullptr;
    }
    return std::make_unique<IfExpr>(std::move(condition), std::move(thenBlock),
                                    std::move(elseBlock));
  }
  return std::make_unique<IfExpr>(std::move(condition), std::move(thenBlock),
                                  nullptr);
}

std::unique_ptr<Expr> Parser::parseForExpr() {
  advance(); // eat the for.
  if (getToken().getType() != tok_identifier) {
    errs() << "Expected an identifier to instanciate for loop";
    return nullptr;
  }
  std::string IdName = std::get<std::string>(getToken().getValue());
  advance(); // eat identifier.

  if (getToken().getType() != tok_equal) {
    errs() << "expected '=' after for";
    return nullptr;
  }
  advance(); // eat '='.

  auto Start = parseExpression();
  if (!Start)
    return nullptr;
  if (getToken().getType() != tok_semicolon) {
    errs() << "expected ',' after for start value";
    return nullptr;
  }
  advance();

  auto End = parseExpression();
  if (!End)
    return nullptr;

  // The step value is optional.
  std::unique_ptr<Expr> Step;
  if (getToken().getType() == tok_semicolon) {
    advance();
    Step = parseExpression();
    if (!Step) {
      errs() << "Failed To Parse Step Expression";
      return nullptr;
    }
  }

  if (getToken().getType() != tok_left_brace) {
    errs() << "expected { after for";
    return nullptr;
  }

  advance(); // eat '{'.

  auto Body = parseExpression();
  if (!Body) {
    errs() << "Failed to parse For Body";
    return nullptr;
  }
  if (getToken().getType() != tok_right_brace) {
    errs() << "expected } after for Body";
    return nullptr;
  }
  advance();
  return std::make_unique<ForExpr>(IdName, std::move(Start), std::move(End),
                                   std::move(Step), std::move(Body));
}

std::unique_ptr<Expr> Parser::parseIdentifier() {
  const Token &curTok = getToken();
  // el mafrood hena hayerga3ly string
  auto val = std::get<std::string>(curTok.getValue());
  advance();
  auto currentToken = getToken();
  if (currentToken.getType() != tok_left_paren) {
    // Not a Call for a function then you can make it a variable reference
    return std::make_unique<VariableExpr>(val);
  }
  // else it's a function we need to parse the arguments
  // advance to eat the opening bracket (
  advance();
  std::vector<std::unique_ptr<Expr>> Args;
  while (getToken().getType() != tok_right_paren) {
    // TODO : This will break the code eventually when you forget the closing
    // parenthesis
    //  Arguments can be expressions
    if (auto Arg = parseExpression()) {
      // TODO: Add poison node here if there's an error else return a null ptr
      //  if Parse Expression doesn't return a null ptr
      Args.push_back(std::move(Arg));
    } else {
      // Could Not Parse the expression probably an error occured
      return nullptr;
    }
    if (getToken().getType() == tok_right_paren) {
      // We reached the end of the function call
      break;
    }
    if (getToken().getType() != tok_comma) {
      std::cout << "Expected ')' or ',' in argument list";
      return nullptr;
    }
    // Eat the comma and repeat the loop
    advance();
  }

  // Eat the ')'.
  advance();
  // Return a Call Expression with val equal to the function name and Args as
  // the arguments
  return std::make_unique<CallExpr>(val, std::move(Args));
}

std::unique_ptr<Expr> Parser::parseGroupingExpression() {
  // eat '('
  advance();
  auto expression = parseExpression();
  if (!expression)
    return nullptr;
  const Token &curTok = getToken();
  if (curTok.getType() != tok_right_paren) {
    std::cout << "Expected ')' after expression";
    return nullptr;
  }
  advance(); // eat ).
  return expression;
}

/// varexpr ::= 'var' identifier ('=' expression)?
//                    (',' identifier ('=' expression)?)* 'in' expression
std::unique_ptr<Expr> Parser::ParseVarExpr() {
  advance(); // eat the var.

  std::vector<std::pair<std::string, std::unique_ptr<Expr>>> VarNames;

  // At least one variable name is required.
  if (getToken().getType() != tok_identifier) {
    errs() << "expected identifier after var , but received " +
                  getToken().getLexeme() + " On line : "
           << getToken().getLine() << "\n";
    return nullptr;
  }
  while (true) {
    std::string Name = get<std::string>(getToken().getValue());
    advance(); // eat identifier.

    // Read the optional initializer.
    std::unique_ptr<Expr> Init;
    if (getToken().getType() == tok_equal) {
      advance(); // eat the '='.
      int line = getToken().getLine();
      Init = parseExpression();
      if (!Init) {
        errs() << format(
            "Failed to parse initializer for variable. On line : %d", line);
        return nullptr;
      }
    }

    // add element to latest map
    VarNames.push_back(std::make_pair(Name, std::move(Init)));

    // End of var list, exit loop.
    if (getToken().getType() != ',')
      break;
    advance(); // eat the ','.

    if (getToken().getType() != tok_identifier) {
      errs() << "expected identifier list after var";
      return nullptr;
    }
  }
  return std::make_unique<VarExprAST>(std::move(VarNames));
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  const Token &curTok = getToken();
  switch (curTok.getType()) {
  case tok_identifier:
    return parseIdentifier();
  case tok_number:
    return number();
  case tok_left_paren:
    return parseGroupingExpression();
  case tok_if:
    return parseIf();
  case tok_for:
    return parseForExpr();
  case tok_var:
    return ParseVarExpr();
  case tok_eof:
  default:
    errs() << "Unknown token when expecting an expression. Token : "
           << curTok.getLexeme() << " On Line : " << curTok.getLine() << "\n";
    return nullptr;
  };
}
int Parser::GetTokPrecedence() {
  const Token &curTok = getToken();
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
    advance(); // eat binop

    if (BinOp == tok_eof) {
      int x = 0;
    }

    // Parse the primary expression after the binary operator.
    auto RHS = parseUnaryExpression();
    if (!RHS)
      return nullptr;
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }
    // Merge LHS/RHS.
    LHS = std::make_unique<BinaryExpr>(BinOp, std::move(LHS), std::move(RHS));
  }
}
std::unique_ptr<Expr> Parser::parseExpression() {
  auto LHS = parseUnaryExpression();
  if (!LHS)
    return nullptr;

  if (auto val = parseBinOpRHS(0, std::move(LHS))) {
    return val;
  }
  errs() << "Failed to parse Expression";
  return nullptr;
}
/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> Parser::parsePrototype() {
  if (getToken().getType() != tok_identifier) {
    std::cout << "Expected function name in prototype";
    return nullptr;
  }

  std::string FnName = std::get<std::string>(getToken().getValue());
  advance();

  if (getToken().getType() != tok_left_paren) {
    std::cout << "Expected '(' in prototype";
    return nullptr;
  }
  advance(); // eat '('.

  // Read the list of argument names.
  std::vector<std::string> ArgNames;
  while (getToken().getType() == tok_identifier) {
    ArgNames.push_back(std::get<std::string>(getToken().getValue()));
    advance();
  }
  if (getToken().getType() != tok_right_paren) {
    std::cout << "Expected ')' in prototype";
    return nullptr;
  }
  // success.
  advance(); // eat ')'.

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}
/// definition ::= 'def' prototype block
std::unique_ptr<FunctionAST> Parser::parseDefinition() {
  advance(); // eat def.
  auto Proto = parsePrototype();
  if (!Proto)
    return nullptr;
  auto E = parseBlock();
  if (!E) {
    return nullptr;
  }
  return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
  advance(); // eat extern.
  return parsePrototype();
}
std::unique_ptr<FunctionAST> Parser::parseTopLevelExpression() {
  if (auto E = parseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

void Parser::mainLoop() {
  while (!isAtEnd()) {
    switch (getToken().getType()) {
    case tok_eof:
      return;
    case tok_def:
      if (auto fn = parseDefinition()) {
        fn->codegen();
      } else {
        // Skip token for error recovery.
        advance();
      }
      break;
    case tok_extern:
      if (auto fn = ParseExtern()) {
        fn->codegen();
      } else {
        // Skip token for error recovery.
        advance();
      }
      break;
    default:
      if (auto fn = parseTopLevelExpression()) {
        fn->codegen();
      } else {
        // Skip token for error recovery.
        advance();
      }
      break;
    }
  }
}
