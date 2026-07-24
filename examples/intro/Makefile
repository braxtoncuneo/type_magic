all:
	g++  main.cpp -o main.exe -std=c++20 -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true  -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10

perf:
	clang++-15 -x c++ main.cpp -std=c++20 -Wall -frelaxed-template-template-args -ftime-trace -DHARMONIZE_REORDER_STRUCT_MEMBERS=true  -c -o main.exe

debug:
	g++  main.cpp -o main.exe -std=c++20 -Wall -DCOLOR_ASSERTS -DHARMONIZE_TRACK_SEQUENCE -DHARMONIZE_REORDER_STRUCT_MEMBERS=true  -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10

