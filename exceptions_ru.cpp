#include <sstream>

#include "exceptions.h"

static void printToStream(std::ostringstream& stream) { }
template <typename T, typename... Args>
static void printToStream(std::ostringstream& stream, T firstArg, Args... args) {
  stream << firstArg;
  printToStream(stream, args...);
}

template <typename... Args>
static std::string makeExceptionText(Args... args) {
  std::ostringstream stream;
  stream << "некорректный ввод, ";
  printToStream(stream, args...);
  return stream.str();
} 

namespace Exceptions {

std::string UnexpectedOperator::what() const { 
  return makeExceptionText("оператор неуместен в данном контексте (символ ", pos_, ")"); 
}

std::string UnexpectedOperand::what() const { 
  return makeExceptionText("число неуместно в данном контексте (символ ", pos_, ")"); 
}

std::string UnexpectedExpressionEnd::what() const {
  return makeExceptionText("выражение неожиданно обрывается после ", pos_, " символа"); 
}

std::string BadSymbols::what() const {
  return makeExceptionText("строка содержит недопустимое выражение ", symbols_.c_str());
}

std::string UnexpectedSymbol::what() const {
  return makeExceptionText("символ '", symbol_, "' неуместен на позиции ", pos_); 
}

}
