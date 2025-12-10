.PHONY: all clean

SRCS=$(wildcard *.cpp)
BINS=$(SRCS:%.cpp=%)
CXXFLAGS = -std=c++23 -g -fsanitize=undefined,address -Wall $(shell pkg-config --cflags cbc)
LDFLAGS = $(shell pkg-config --libs cbc)

all: $(BINS)

%: %.cpp
	g++ $< $(CXXFLAGS) $(LDFLAGS) -o $@

clean:
	$(RM) $(BINS)
