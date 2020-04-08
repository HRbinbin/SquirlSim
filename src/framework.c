//
// Created by Ruibin on 2020/2/28.
//
/*
 * Example code to run and test the process pool. To compile use something like mpicc -o test test.c pool.c
 */

#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "../include/pool.h"
#include "../include/framework.h"
#include "../include/actorConfig.h"
#include "../include/landActor.h"
#include "../include/squirrelActor.h"
#include "../include/controllerActor.h"

static int masterInitialiseWorker(int identity);
static void masterInitialiseWorkers(int identity, int count, int * workerPids);
static void masterInitialiseVariables();
static void masterSendWorkers(int count, int workerPids[], WorkerFunc workerAskFunc);
static void masterCode();
static void workerCode(WorkerFunc initialiseFunc, WorkerFunc workerFunc);

static void masterInitialiseVariables() {
    sickCount = 0;
}

int main(int argc, char* argv[]) {
    // Call MPI initialize first
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);

    /*
     * Initialise the process pool.
     * The return code is = 1 for worker to do some work, 0 for do nothing and stop and 2 for this is the master so call master poll
     * For workers this subroutine will block until the master has woken it up to do some work
     */
    int statusCode = processPoolInit();

    if (statusCode == 1) {
        int identity;
        MPI_Recv(&identity, 1, MPI_INT, MPI_ANY_SOURCE, IDENTITY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        switch (identity){
            case CONTROLLER_ACTOR:
                workerCode(initialiseController, controllerWorker);
                break;
            case LAND_ACTOR:
                workerCode(initialiseLandCell, landWorker);
                break;
            case SQUIRREL_ACTOR:
                workerCode(initialiseSquirrel, squirrelWorker);
                break;
        }

    } else if (statusCode == 2) {
        masterCode();
    }

    // Finalizes the process pool, call this before closing down MPI
    processPoolFinalise();
    // Finalize MPI, ensure you have closed the process pool first
    MPI_Finalize();
    return 0;
}

static void masterCode() {
    masterInitialiseVariables();
    // Initial controller
    masterInitialiseWorkers(CONTROLLER_ACTOR, CONTROLLER_NUMBER, controllers);
    // Initial land actors
    masterInitialiseWorkers(LAND_ACTOR, LENGTH_OF_LAND, cellWorkers);
    // Initial squirrel actors
    masterInitialiseWorkers(SQUIRREL_ACTOR, INITIAL_NUMBER_OF_SQUIRRELS, squirrelWorkers);
    // Response controller's ask
    masterSendWorkers(CONTROLLER_NUMBER, controllers, controllerAsk);
    // Response lands' ask
    masterSendWorkers(LENGTH_OF_LAND, cellWorkers, landAsk);
    // Response squirrels' ask
    masterSendWorkers(INITIAL_NUMBER_OF_SQUIRRELS, squirrelWorkers, squirrelAsk);

    double start, end;
    start = MPI_Wtime();

    int masterStatus = masterPoll();
    while (masterStatus) {
        masterStatus=masterPoll();
    }

    end = MPI_Wtime();
    printf("Master Quit. Runtime %f\n", end-start);
}

static int masterInitialiseWorker(int identity){
    // Initial controller
    int workerPid = startWorkerProcess();
    // Tell the process that it is a controller
    MPI_Send(&identity, 1, MPI_INT, workerPid, IDENTITY_TAG, MPI_COMM_WORLD);
    return workerPid;
}

static void masterInitialiseWorkers(int identity, int count, int workerPids[]){
    int i;
    for (i=0;i<count;i++) {
        int workerPid = masterInitialiseWorker(identity);
        workerPids[i] = workerPid;
    }
}

static void masterSendWorkers(int count, int workerPids[], WorkerFunc workerAskFunc){
    int i, workerPid;
    for (i=0;i<count;i++) {
        workerAskFunc(workerPids[i]);
    }
}

static void workerCode(WorkerFunc initialiseFunc, WorkerFunc workerFunc) {
    int workerStatus = 1;
    while (workerStatus) {
        // Initialise worker
        initialiseFunc();
        // Worker do work
        workerFunc();
        workerStatus=workerSleep();	// This MPI process will sleep, further workers may be run on this process now
    }
}
