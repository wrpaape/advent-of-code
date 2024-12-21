.PHONY: all clean

SRCS=$(wildcard *.cpp)
BINS=$(SRCS:%.cpp=%)

all: $(BINS)

%: %.cpp
	g++ $< -std=c++23 -g -fsanitize=undefined,address -Wall -o $@

clean:
	$(RM) $(BINS)
