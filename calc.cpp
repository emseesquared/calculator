#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "exceptions.h"
#include "parser.h"

static std::string formatDouble(const double val) {
  std::ostringstream stream;
  stream << std::setprecision(2) 
	 << std::fixed 
	 << val;

  std::string result = stream.str();
  static const char decPoint = std::use_facet<std::numpunct<char>>(stream.getloc()).decimal_point();

  // Remove trailing zeros
  auto rIt = result.rbegin();
  while (*rIt == '0') ++rIt;

  if (*rIt == decPoint) // for integers
    ++rIt;

  result.erase(rIt.base(), result.end());

  return result;
}

int main() {
  while (std::cin.good()) {
    try {
      auto parser = ExpressionParser::parseStream(std::cin);

      if (!parser.nothingRead())
	std::cout << formatDouble(parser.getTree().evaluate()) << std::endl;
    }
    catch (Exceptions::ParsingException& e) {
      std::cerr << e.what() << std::endl;

      // Prepare ourselves for the next expression
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }

  return 0;
}
