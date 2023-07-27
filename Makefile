CXX = g++
CXXFLAGS = -Wall -std=c++20

VPATH = __BUILD__

all: HelloWorld OptionPricer

# HelloWorld executable depends directly on HelloWorld.cpp:
HelloWorld: HelloWorld.cpp
	# Building HelloWorld...
	$(CXX) $(CXXFLAGS) -o $(VPATH)/$@ $<

OptionPricer: OptionPricer.cpp BSM.o
	$(CXX) $(CXXFLAGS) -o $(VPATH)/$@ OptionPricer.cpp $(VPATH)/BSM.o -lboost_program_options

# Separate compilation of BSM.o:
BSM.o: BSM.cpp BSM.h
	$(CXX) $(CXXFLAGS) -c -o $(VPATH)/$@ BSM.cpp

clean:
	rm -f $(VPATH)/*

