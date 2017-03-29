//
// Created by geoffrey on 26/03/17.
//

#ifndef HILLCLIMBERFI_FUNCTIONS_H
#define HILLCLIMBERFI_FUNCTIONS_H
#define BUFFER_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Initialisation of variables & test program arguments */
int initParameters(int argc, char * argv[], int * sizeOfSolution, int * numberOfEvaluation);

/* Listen the server & load the initial solution */
void loadInitialSolution(int *solution, int size);

/* Send a solution to the server */
void askFitness(const int *solution, int size);

#endif