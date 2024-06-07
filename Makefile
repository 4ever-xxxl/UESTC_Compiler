# Compiler
CXX = clang++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -O3

# Source files
SRC = $(wildcard *.cc)

# Object files
OBJ = $(SRC:.cc=.o)

# Executable name
EXEC = program

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXEC)
	./$(EXEC)

clean:
	rm -f $(EXEC) $(OBJ)