#ifndef __PARSER_H__
#define __PARSER_H__
#include <iostream>

#include "tree.h"

enum class TokenType {
  Empty,
  Operand,
  Operator,
  Block
};

class ExpressionParser {
public:
  static ExpressionParser parseStream(std::istream&);

  EvaluationTree& getTree() { return result_; }
  std::size_t getCharsRead() const { return charsRead_; }

  bool nothingRead() const {
    return lastRead_ == TokenType::Empty;
  }

  static bool isTerminal(const char);
  static bool isDecimalPoint(const char);

private:
  ExpressionParser(std::istream& s): stream_(s) {
    parse();
  }

  void parse();
  void parseNext();

  double readDouble();
  std::string readBadSymbols();

  char readNextChar();

  std::istream& stream_;

  std::size_t charsRead_ = 0;
  bool exprEndReached_ = false;
  TokenType lastRead_ = TokenType::Empty;
  
  EvaluationTree result_;
};

#endif
