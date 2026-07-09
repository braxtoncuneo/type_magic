CXX ?= g++
CXXFLAGS := -std=c++20 -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10

.PHONY: all clean

all: init_check.exe existing_checks.exe
	./init_check.exe
	./existing_checks.exe

init_check.exe: init_check.cpp ../include/include.h
	$(CXX) init_check.cpp -o init_check.exe $(CXXFLAGS)

existing_checks.exe: sanity_check.cpp module_check.cpp ../include/include.h
	printf '%s\n' \
		'#include "../include/include.h"' \
		'#include "sanity_check.cpp"' \
		'#include "module_check.cpp"' \
		'int main() { return 0; }' \
		| $(CXX) -x c++ - -o existing_checks.exe $(CXXFLAGS)

clean:
	rm -f init_check.exe existing_checks.exe
