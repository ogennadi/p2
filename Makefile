CXXFLAGS := -g -Wall -std=c++0x -lm
CXX=g++

all: procsim

procsim: procsim.o procsim_driver.o
	$(CXX) -o procsim procsim.o procsim_driver.o

clean:
	rm -f procsim *.o
