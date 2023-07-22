CXX = g++
CXXFLAGS = -Wall -std=c++20

VPATH = __BUILD__

all: HelloWorld BSM.o

# HelloWorld executable depends directly on HelloWorld.cpp:
HelloWorld: HelloWorld.cpp
	# Building HelloWorld...
	$(CXX) $(CXXFLAGS) -o $(VPATH)/$@ $<

BSM.o: BSM.cpp BSM.h
	$(CXX) $(CXXFLAGS) -c -o $(VPATH)/$@ $<

clean:
	rm -f $(VPATH)/*

