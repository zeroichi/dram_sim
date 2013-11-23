CXXFLAGS=-g

all: proctest dramtest

proctest: proctest.o ../model.o ../proc.o
	$(CXX) $(CXXFLAGS) -o $@ $^

dramtest: dramtest.o ../model.o ../proc.o ../dram.o ../addr.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o proctest dramtest

.PHONY: all clean
