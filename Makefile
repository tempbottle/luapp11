INCLUDE = $(shell pkg-config --cflags luajit)
LIBS = $(shell pkg-config --libs luajit)

HEADERS  = $(shell find . -name *.h) $(shell find . -name *.hpp)
TEST_CPP = $(shell ls test/*.cpp)

bin/test: $(HEADERS) $(TEST_CPP)
	@mkdir -p bin
	clang++ -g --std=c++11 $(TEST_CPP) -o $@ $(INCLUDE) -I./ $(LIBS)