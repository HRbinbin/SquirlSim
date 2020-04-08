//
// Created by Ray on 2020/2/28.
//
#include "../include/config.h"
#include "../include/actorConfig.h"

#ifndef SQUIRLSIM_MAIN_H
#define SQUIRLSIM_MAIN_H

// Worker require variables
int controllers[CONTROLLER_NUMBER];
int cellWorkers[LENGTH_OF_LAND];
int squirrelWorkers[INITIAL_NUMBER_OF_SQUIRRELS];
int sickCount;

MPI_Group worldGroup;
typedef int (*WorkerFunc)();

static int masterInitialiseWorker(int identity);
static void masterInitialiseWorkers(int identity, int count, int * workerPids);
static void masterInitialiseVariables();
static void masterSendWorkers(int count, int workerPids[], WorkerFunc workerAskFunc);
static void masterCode();
static void workerCode(WorkerFunc initialiseFunc, WorkerFunc workerFunc);

#endif //SQUIRLSIM_MAIN_H
