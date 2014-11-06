CXX ?= g++
CXXFLAGS = -Wall -std=c++11 -O2
SOURCES = regexppars.cpp regexpcmp.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLES = $(OBJECTS:.o=)

all : $(EXECUTABLES)

.PHONY : clean

clean : 
	rm -f $(OBJECTS) $(EXECUTABLES)
