#include <cmath>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "exceptions.h"
#include "parser.h"

#define TEST(name) void name()
#define RUNTEST(name) Tester::instance().runTest(#name, &name);

class TestFailed: public std::runtime_error {
public:
  TestFailed(const std::string& expectation, const std::string& reality): std::runtime_error(std::string("Test failed: expecting ") + expectation + " but got " + reality + " :-(") { }
};

class Tester {
public:
  static Tester& instance() {
    static Tester t;
    return t;
  }

  void runTest(const std::string& name, const std::function<void()> test) {
    std::cout << "Running '" << name << "'" << std::endl;

    bool passed = false;
    try {
      test();
      passed = true;
    }
    catch (TestFailed& e) {
      std::cerr << e.what() << std::endl;
    }
    catch (Exceptions::ParsingException& e) {
      std::cerr << "  !Caught a parsing error: " << e.what() << std::endl;
    }
    catch (std::exception& e) {
      std::cerr << " !!Caught internal exception: " << e.what() << std::endl;
    }
    catch (...) {
      std::cerr << "!!!Caught a cold" << std::endl;
    }

    if (!passed) {
      std::cerr << "   the query was: " << lastQuery_ << std::endl;
      failed_.push_back(name);
    }
    else
      ++passed_;
  }

  int statistics() const {
    std::cout << std::endl << "TESTING FINISHED" << std::endl;
    std::cout << "Total: " << (passed_ + failed_.size()) << std::endl;
    std::cout << "Passed: " << passed_ << std::endl;
    if (!failed_.empty()) {
      std::cout << "Failed: " << std::endl;
      for (const auto& s : failed_)
	std::cout << "  " << s << std::endl;

      return 1;
    }

    return 0;
  }

  void setLastQuery(const std::string& q) {
    lastQuery_ = q;
  }

private:
  Tester() = default;
  Tester(const Tester&) = delete;
  Tester(Tester&&) = delete;

  std::vector<std::string> failed_;
  std::string lastQuery_;
  unsigned passed_ = 0;
};

const double eps = 0.01;
void assumeResult(const std::string& inp, const double assumption) {
  Tester::instance().setLastQuery(inp);

  std::istringstream stream(inp);
  auto parser = ExpressionParser::parseStream(stream);
  double result = parser.getTree().evaluate();

  if (std::abs(result - assumption) > eps) {
    std::ostringstream assStr;
    std::ostringstream resStr;

    assStr << std::setprecision(2) << std::fixed << assumption;
    resStr << std::setprecision(2) << std::fixed << result;

    throw TestFailed(assStr.str(), resStr.str());
  }
}

template <typename ExType, typename... Args>
void assumeException(const std::string& inp, const size_t pos, Args... exArgs) {
  Tester::instance().setLastQuery(inp);

  ExType ass/*umption*/(exArgs...);
  ass.movePos(pos);

  double result;
  try {
    std::istringstream stream(inp);
    auto parser = ExpressionParser::parseStream(stream);
    result = parser.getTree().evaluate();
  }
  catch (Exceptions::ParsingException& e) {
    if (ass.what() != e.what())
      throw TestFailed(std::string("exception '") + ass.what() + "'", std::string("exception '") + e.what() + "'");

    return;
  }

  std::ostringstream resStr;
  resStr << std::setprecision(2) << std::fixed << result;

  throw TestFailed(std::string("exception '") + ass.what() + "'", resStr.str());
}

struct Expr {
  virtual ~Expr() = default;
  virtual std::string serialize() const = 0;
  virtual double evaluate() const = 0;
};

struct OperationsChain: Expr {
  std::string serialize() const override { 
    static std::string vals = "+-*/";
    std::ostringstream ser;
    
    ser << "(" << operChain.front()->serialize();
    for (size_t i = 1; i < operChain.size(); ++i) {
      ser << vals[signChain[i-1]] << operChain[i]->serialize();
    }
    ser << ")";

    return ser.str();
  }

  double evaluate() const override {
    double result = operChain.front()->evaluate();

    for (size_t i = 1; i < operChain.size(); ++i) {
      switch(signChain[i-1]) {
      case 0: result+= operChain[i]->evaluate(); break;
      case 1: result-= operChain[i]->evaluate(); break;
      case 2: result*= operChain[i]->evaluate(); break;
      default: result/= operChain[i]->evaluate(); break;
      }
    }

    return result;
  }

  std::vector<std::shared_ptr<Expr>> operChain;
  std::vector<short> signChain;
};

struct Block: Expr {
  std::string serialize() const override { return std::string("(") + content->serialize() + ")"; }
  double evaluate() const override { return content->evaluate(); }

  std::shared_ptr<Expr> content;
};

struct Number: Expr {
  std::string serialize() const override { 
    std::ostringstream str;
    str << std::setprecision(2) << std::fixed << num;
    return str.str();
  }
  double evaluate() const override { return num; }

  double num;
};

int randInt(const int from, const int to) {
  return (rand() % (to - from + 1)) + from;
}

std::shared_ptr<Expr> generateRandomExpression(unsigned depth = 0) {
  /* My random is so big you can't handle it */
  const int choice = depth < 8 ? randInt(0, 2) : 0;
  switch(choice) {
  case 0: {
    auto numExpr = std::make_shared<Number>();
    numExpr->num = static_cast<double>(randInt(-10000, 10000)) / 100.0;
    return numExpr;
  }
  case 1: {
    auto operExpr = std::make_shared<OperationsChain>();
    const int chainSize = randInt(2, 4);
    const int chainPriority = randInt(0, 1);

    operExpr->operChain.reserve(chainSize);
    operExpr->signChain.reserve(chainSize - 1);

    for (int i = 0; i < chainSize; ++i) {
      if (i > 0)
	operExpr->signChain.push_back(static_cast<short>(chainPriority * 2 + randInt(0, 1)));
      operExpr->operChain.push_back(generateRandomExpression(depth+1));
    }

    return operExpr;
  }
  default: {
    auto blExpr = std::make_shared<Block>();
    blExpr->content = generateRandomExpression(depth+1);
    return blExpr;
  }
  }
}

TEST(fundamental_equality) {
  assumeResult("2 * 2", 4);
}

TEST(operator_priorities) {
  assumeResult("1 + 2*3", 7);
  assumeResult("2*3 - 27", -21);
  assumeResult("1 + 2 + 3*8 + 1", 28);

  assumeResult("-2*25", -50);
  assumeResult("3 - 2 * -25", 53);

  assumeResult("2*2*2", 8);
  assumeResult("4/4/4", 0.25);
  assumeResult("8+8+8", 24);
  assumeResult("16-16-16", -16);
}

TEST(braced_structures) {
  assumeResult("3*(1+2)", 9);
  assumeResult("2 + 3 * (2*(3+5))", 50);
  assumeResult("1 - (3 + 5)", -7);
  assumeResult("(2 * 3) + 1", 7);

  // Multiplication
  assumeResult("1+2(3+4)", 15);
  assumeResult("-(2+3)2.5", -12.5);
}

TEST(non_integers) {
  assumeResult("1/2", 0.5);
  assumeResult("0.01 + 1.23 + 7/2", 4.74);
  assumeResult("(1+1)/10", 0.2);
  assumeResult("0.5 - 0,5", 0);

  assumeException<Exceptions::UnexpectedSymbol>("3.2,1", 4, ',');

  assumeResult(",2", 0.2);
  assumeResult(".0 + 2 - ,25", 1.75);
  assumeResult("4*,25", 1);

  // Rouding
  assumeResult("(3*9)/1000", 0.03);
  assumeResult("2/1000", 0);
  assumeResult("8/1000", 0.01);
  assumeResult("0.99", 0.99);
}

TEST(exceptional_cases) {
  assumeResult("-(+2+5)", -7);
  assumeResult("--3", 3);
  assumeResult("3*-2+1", -5);
  assumeResult("(22)", 22);
  assumeResult("3 + (((1+2)*3 + 4)*5 + 6)*7", 500);
  assumeResult("1 + ( 2 *( 2.5 + 2.5 +( 3 - 2)))-  3/ 1.5", 11);
  assumeResult("-10 + (8*2.5) - (3/1,5)", 8);
}

using namespace Exceptions;
TEST(bad_cases) {
  assumeException<BadSymbols>("25 + xyz", 6, "xyz");
  assumeException<UnexpectedOperand>("2+3 7", 5);
  assumeException<UnexpectedOperator>("*7 - 2", 1);

  assumeException<UnexpectedOperator>("3+*2", 3);
  assumeException<UnexpectedSymbol>("3.2.1", 4, '.');

  assumeException<UnexpectedExpressionEnd>("2+", 2);
  assumeException<UnexpectedExpressionEnd>("5+(2*)-1", 5);
  assumeException<UnexpectedExpressionEnd>("3()", 2);

  assumeException<UnexpectedSymbol>(")2+1", 1, ')');
  assumeException<UnexpectedSymbol>("(2+3)2)+1", 7, ')');

  assumeException<UnexpectedSymbol>(".", 1, '.');
}

TEST(randomized_tests) {
  for (int i = 0; i < 100; ++i) {    
    auto expr = generateRandomExpression();
    try {
      assumeResult(expr->serialize(), expr->evaluate());
    }
    catch (...) {
      std::cout << std::endl;
      throw;
    }
    std::cout << ".";
    std::cout.flush();
  }
  std::cout << std::endl;
}

int main() {
  RUNTEST(fundamental_equality);
  RUNTEST(operator_priorities);
  RUNTEST(braced_structures);
  RUNTEST(non_integers);
  RUNTEST(exceptional_cases);
  RUNTEST(bad_cases);

  RUNTEST(randomized_tests);

  return Tester::instance().statistics();
}
