CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
SOURCES = orderbook.cpp
HEADERS = orderbook.hpp

# Targets
all: main test test_advanced

main: main.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o main main.cpp $(SOURCES)

test: test.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o test test.cpp $(SOURCES)

# Run programs
run-main: main
	./main

run-test: test
	./test

run-all-tests: test
	@echo "Running basic tests..."
	./test

# Clean build artifacts
clean:
	rm -f main test lob

# Debug build
debug: CXXFLAGS = -std=c++17 -g -Wall -Wextra -DDEBUG
debug: all

# Check for memory leaks with valgrind (if available)
valgrind: test
	valgrind --leak-check=full ./test

.PHONY: all clean run-main run-test debug valgrind
