all:
	g++ module.cpp -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true -o mod_check.exe -ftemplate-backtrace-limit=0

perf:
	clang++ module.cpp -stdlib=libc++ -std=c++20 -Wall -ftime-trace -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true -o mod_check.exe


