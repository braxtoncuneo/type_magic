all:
	g++ module.cpp -Wall -DCOLOR_ASSERTS -DHARMONIZE_REORDER_STRUCT_MEMBERS=true -o mod_check.exe

