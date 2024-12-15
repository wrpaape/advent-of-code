.PHONY: all clean

SRCS=$(wildcard *.cpp)
BINS=$(SRCS:%.cpp=%)

all: $(BINS)

%: %.cpp
	g++ $< -std=c++23 -g -fsanitize=undefined -o $@

clean:
	$(RM) $(BINS)
