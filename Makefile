CXX = g++
CXXFLAGS = -Wall -std=c++2a

VPATH = __BUILD__

all: HelloWorld OptionPricer

# HelloWorld executable depends directly on HelloWorld.cpp:
HelloWorld: HelloWorld.cpp
	# Building HelloWorld...
	$(CXX) $(CXXFLAGS) -o $(VPATH)/$@ $<

OptionPricer: OptionPricer.cpp BSM.o
	$(CXX) $(CXXFLAGS) -o $(VPATH)/$@ OptionPricer.cpp $(VPATH)/BSM.o

# Separate compilation of BSM.o:
BSM.o: BSM.cpp BSM.h
	$(CXX) $(CXXFLAGS) -c -o $(VPATH)/$@ BSM.cpp

clean:
	rm -f $(VPATH)/*

