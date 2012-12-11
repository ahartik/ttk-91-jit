
CXXFLAGS=-I$(HOME)/local/include/ -g -Wall -std=c++0x
LDFLAGS=$(HOME)/local/lib/libjit.a $(HOME)/local/lib/libjitplus.a -lpthread -lboost_regex

default: test

interp: interp.cpp
	$(CXX) interp.cpp -o $@ $(CXXFLAGS) $(LDFLAGS)

asm.o: asm.cpp asm.hpp
	$(CXX) asm.cpp -c -o $@ $(CXXFLAGS)
compiler.o: compiler.cpp compiler.hpp asm.hpp
	$(CXX) compiler.cpp -c -o $@ $(CXXFLAGS)
test: test.cpp compiler.o asm.o asm.hpp compiler.hpp
	$(CXX) asm.o compiler.o test.cpp -o $@ $(CXXFLAGS) $(LDFLAGS)
