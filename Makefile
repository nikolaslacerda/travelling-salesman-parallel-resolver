compile: tsp_omp.cpp
	g++ -fopenmp tsp_omp.cpp -o tsp_omp

all: compile