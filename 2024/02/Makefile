.PHONY: all clean

SRCS=$(wildcard *.cpp)
BINS=$(SRCS:%.cpp=%)

all: $(BINS)

%: %.cpp
	g++ $< -g -fsanitize=undefined -o $@

clean:
	$(RM) $(BINS)
