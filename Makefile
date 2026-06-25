all:
	#nvcc  module.cpp -o mod_check.exe --compiler-options "-std=c++20 -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true  -ftemplate-backtrace-limit=0"
	g++  module.cpp -o mod_check.exe -std=c++20 -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true  -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=10 -fdiagnostics-all-candidates

perf:
	clang++ module.cpp -stdlib=libc++ -std=c++20 -Wall -ftime-trace -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true -o mod_check.exe


