CXX = g++
FLAGS = -g -std=c++11 -Wall

all: calc test

tree.o: tree.cpp tree.h exceptions.h
	$(CXX) -c $< $(FLAGS) -o $@

parser.o: parser.cpp parser.h tree.h exceptions.h
	$(CXX) -c $< $(FLAGS) -o $@

exceptions.o: exceptions_ru.cpp exceptions.h
	$(CXX) -c $< $(FLAGS) -o $@

calc: calc.cpp tree.o parser.o exceptions.o
	$(CXX) $< tree.o parser.o exceptions.o -o $@ $(FLAGS)

test:
	$(CXX) tests.cpp tree.o parser.o exceptions.o -o tests
	@echo '--- Running tests ---'
	@./tests

clean:
	rm -f parser.o tree.o exceptions.o calc tests