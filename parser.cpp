#include <cstdlib>
#include <iostream>

#include "parser.h"
#include "exceptions.h"

ExpressionParser ExpressionParser::parseStream(std::istream& stream) {
  ExpressionParser parser(stream);

  // Garbage left?
  const char c = stream.get();

  if (!ExpressionParser::isTerminal(c)) {
    auto up = Exceptions::UnexpectedSymbol(c);
    up.movePos(parser.getCharsRead() + 1);
    throw up;
  }

  return parser;
}

void ExpressionParser::parse() {
  while (stream_.good() && !exprEndReached_) {    
    try {
      parseNext();

      if (exprEndReached_ && !result_.isReady())
	throw Exceptions::UnexpectedExpressionEnd();
    }
    catch (Exceptions::ParsingException& e) {
      e.movePos(getCharsRead());
      throw;
    }
  }
}

bool ExpressionParser::isTerminal(char c) {
  return c == EOF || c == '\n';
}

bool ExpressionParser::isDecimalPoint(char c) {
  return c == '.' || c == ',';
}

void ExpressionParser::parseNext() {
  const char c = stream_.peek();

  if (isTerminal(c) || c == ')') {
    exprEndReached_ = true;
    return;
  }

  if (std::isspace(c)) // Ignore whitespace
    readNextChar();
  else if (std::isdigit(c) || isDecimalPoint(c)) {
    if (lastRead_ == TokenType::Block)
      result_.insertOperator('*');

    result_.insertOperand(readDouble());
    lastRead_ = TokenType::Operand;
  }
  else if (c == '+' || c == '-' || c == '*' || c == '/') {
    result_.insertOperator(readNextChar());
    lastRead_ = TokenType::Operator;
  }
  else if (c == '(') {
    readNextChar();

    if (lastRead_ == TokenType::Operand || lastRead_ == TokenType::Block)
      result_.insertOperator('*');

    ExpressionParser recParser(stream_);
    charsRead_+= recParser.getCharsRead();

    if (recParser.nothingRead())
      throw Exceptions::UnexpectedExpressionEnd();

    result_.insertSubTree(recParser.getTree());
    lastRead_ = TokenType::Block;

    if (!stream_.good() || readNextChar() != ')')
      throw Exceptions::UnexpectedExpressionEnd();
  }
  else
    throw Exceptions::BadSymbols(readBadSymbols());
}

double ExpressionParser::readDouble() {
  std::string dStr;
  char c;
  bool hasDecPoint = false;

  while ((c = stream_.get())) {
    if (isDecimalPoint(c)) {
      if (hasDecPoint) {
	++charsRead_;
	throw Exceptions::UnexpectedSymbol(c);
      }

      hasDecPoint = true;
      dStr.push_back('.');
    }
    else if (std::isdigit(c))
      dStr.push_back(c);
    else {
      stream_.unget();
      break;
    }

    ++charsRead_;
  }

  // Forbid .
  if (hasDecPoint && dStr.size() == 1)
    throw Exceptions::UnexpectedSymbol(dStr[0]);

  return std::atof(dStr.c_str());
}

std::string ExpressionParser::readBadSymbols() {
  /* For some reason according to the ToR we're supposed to return the
     whole line of bad symbols in the exception, not just the first
     one. Well, whatever. */
  static const std::string goodSymbols = " +-*/().,0123456789";
  std::string badSymbols;

  while (!isTerminal(stream_.peek()) && goodSymbols.find(stream_.peek()) == std::string::npos)
    badSymbols.push_back(stream_.get()); // Do not increase counter
  
  return badSymbols;
}

char ExpressionParser::readNextChar() {
  ++charsRead_;
  return stream_.get();
}
