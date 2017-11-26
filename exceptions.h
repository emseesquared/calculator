#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <string>

namespace Exceptions {

class ParsingException {
 public:
  virtual ~ParsingException() = default;
  virtual std::string what() const = 0;

  void movePos(const std::size_t offset) { 
    pos_+= offset;
  }

 protected:
  std::size_t pos_ = 0;
};

struct UnexpectedOperator: ParsingException {
  std::string what() const override;
};

struct UnexpectedOperand: ParsingException {
  std::string what() const override;
};

struct UnexpectedExpressionEnd: ParsingException { 
  std::string what() const override;
};

class BadSymbols: public ParsingException {
public:
  BadSymbols(std::string&& s): symbols_(std::move(s)) { }
  std::string what() const override;

private:
  std::string symbols_;
};

class UnexpectedSymbol: public ParsingException {
public:
  UnexpectedSymbol(const char c): symbol_(c) { } 
  std::string what() const override;

private:
  char symbol_;
};

}

#endif
