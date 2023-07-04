# Makefile for Writing Make Files Example
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -pthread -O3
PROFILING= #-pg

# ****************************************************
# Targets needed to bring the executable up to date
 
all: globals.o graph.o canonical_ordering.o combinations.o utils.o
	g++ ${CFLAGS} $(PROFILING) main.cpp -o calculate_for_level_opt globals.o graph.o canonical_ordering.o combinations.o utils.o
	
# The main.o target can be written more simply
# main.o: main.cpp graph.o canonical_ordering.o combinations.o utils.o
# 	g++ $(CFLAGS) -c main.cpp
 
globals.o: graph.o
	g++ $(CFLAGS) $(PROFILING) -c "./data-structures/globals.cpp" -o globals.o

clean: 
	rm *.o

graph.o: 
	g++ $(CFLAGS) $(PROFILING) -c "./data-structures/graph.cpp" -o graph.o 

canonical_ordering.o:
	g++ $(CFLAGS) $(PROFILING) -c "./utils/canonical_ordering.cpp" -o canonical_ordering.o 

combinations.o: 
	g++ $(CFLAGS) $(PROFILING) -c "./utils/combinations.cpp" -o combinations.o 

utils.o: graph.o globals.o
	g++ $(CFLAGS) $(PROFILING) -c "./utils/utils.cpp" -o utils.o globals.o 