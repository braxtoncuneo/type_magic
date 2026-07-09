all:
	g++  main.cpp -o main.exe -std=c++20 -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true  -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10

perf:
	clang++ -x c++ main.cpp -std=c++20 -Wall -ftime-trace -DHARMONIZE_REORDER_STRUCT_MEMBERS=true -o mod_check.exe


