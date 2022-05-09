# $(this).CC := gcc-11
# $(this).CXX := g++-11
$(this).CC := gcc
$(this).CXX := g++
$(this).CFLAGS := $(CFLAGS) -Wall -Werror -DDEBUG
$(this).CXXFLAGS := -Wall -Werror -std=c++17
$(this).CPPFLAGS := $(CPPFLAGS)
