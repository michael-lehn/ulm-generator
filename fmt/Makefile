CC := $(CXX)
DEBUG := -g
CXXFLAGS := -Wall -std=c++11 $(DEBUG)
LDFLAGS := $(DEBUG)
.PHONY:		all clean depend
all:		test_suite
test_suite.o:	test_suite.cpp printf.hpp

clean:
		rm -f test_suite test_suite.o *.gcov gmon.out *.gcno *.gcda core
