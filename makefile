# Makefile for Writing Make Files Example
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -Wall -Ofast -pthread
PROFILING= #-pg

# ****************************************************
# Targets needed to bring the executable up to date
 
all: globals.o graph.o canonical_ordering.o combinations.o utils.o
	g++ ${CFLAGS} main.cpp -o calculate_for_level_opt globals.o graph.o canonical_ordering.o combinations.o utils.o $(PROFILING)
	
# The main.o target can be written more simply
# main.o: main.cpp graph.o canonical_ordering.o combinations.o utils.o
# 	g++ $(CFLAGS) -c main.cpp
 
globals.o: graph.o
	g++ $(CFLAGS) -c "./data-structures/globals.cpp" -o globals.o $(PROFILING)

clean: 
	rm *.o

graph.o: 
	g++ $(CFLAGS) -c "./data-structures/graph.cpp" -o graph.o $(PROFILING)

canonical_ordering.o:
	g++ $(CFLAGS) -c "./utils/canonical_ordering.cpp" -o canonical_ordering.o $(PROFILING)

combinations.o: 
	g++ $(CFLAGS) -c "./utils/combinations.cpp" -o combinations.o $(PROFILING)

utils.o: graph.o globals.o
	g++ $(CFLAGS) -c "./utils/utils.cpp" -o utils.o globals.o $(PROFILING)