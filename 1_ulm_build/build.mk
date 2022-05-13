# $(this).CC := gcc-11
# $(this).CXX := g++-11
$(this).CC := gcc
$(this).CXX := g++
$(this).CFLAGS := $(CFLAGS) -Wall -DDEBUG
$(this).CXXFLAGS := -Wall -std=c++17
$(this).CPPFLAGS := $(CPPFLAGS)
