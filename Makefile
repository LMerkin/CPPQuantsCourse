CXX = g++
OPT      = -Ofast -march=native -mtune=native
CXXFLAGS = -Wall  -std=c++20

VPATH = __BUILD__

all: HelloWorld OptionPricer HTTPClient1

# HelloWorld executable depends directly on HelloWorld.cpp:
HelloWorld: HelloWorld.cpp
	$(CXX) $(OPT) $(CXXFLAGS) -o $(VPATH)/$@ $<

HTTPClient1: HTTPClient1.cpp
	$(CXX) $(OPT) $(CXXFLAGS) -o $(VPATH)/$@ $<

OptionPricer: OptionPricer.cpp BSM.o
	$(CXX) $(OPT) $(CXXFLAGS) -o $(VPATH)/$@ OptionPricer.cpp $(VPATH)/BSM.o

# Separate compilation of BSM.o:
BSM.o: BSM.cpp BSM.h
	$(CXX) $(OPT) $(CXXFLAGS) -c -o $(VPATH)/$@ BSM.cpp

clean:
	rm -f $(VPATH)/*

