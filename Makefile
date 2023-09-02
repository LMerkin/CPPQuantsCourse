CXX = g++
#OPT     = -Ofast -march=native -mtune=native
OPT      = -O0 -g
CXXFLAGS = -Wall -Wextra -std=c++20 -I./3rdParty

VPATH = __BUILD__

all: HelloWorld OptionPricer HTTPClient1 HTTPServer1

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

TCP_Acceptor.o: TCP_Acceptor.cpp TCP_Acceptor.h
	$(CXX) $(OPT) $(CXXFLAGS) -c -o $(VPATH)/$@ TCP_Acceptor.cpp

HTTPServer1:    HTTPServer1.cpp HTTPProtoDialogue.hpp Echo.hpp Logger.hpp \
                TCP_Acceptor.o  error.o
	$(CXX) $(OPT) $(CXXFLAGS) -o $(VPATH)/$@ HTTPServer1.cpp $(VPATH)/TCP_Acceptor.o $(VPATH)/error.o

error.o: 3rdParty/utxx/error.cpp
	$(CXX) $(OPT) $(CXXFLAGS) -c -o $(VPATH)/$@ $<

clean:
	rm -f $(VPATH)/*

