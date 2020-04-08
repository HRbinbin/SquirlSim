//
// Created by Ray on 2020/4/7.
//

#include <stdio.h>
#include <mpi.h>
#include "../include/framework.h"
#include "../include/controllerActor.h"
#include "../include/config.h"
#include "../include/actorConfig.h"

int cellWorkers[LENGTH_OF_LAND];
int controllerWorkerPid;
int popNInf[LENGTH_OF_LAND * 2];

int month;
int remainSquirrel;
int infectedSquirrel;
int totalDeadSquirrel;
int activeSquirrelWorkers;
int stopSignal;

MPI_Status status;

int initialiseController();
int controllerAsk(int workerPid);
int controllerWorker();
void sendAllLandCell(int * sendBuffer, int count);
void sendRecvAllPopNInf(int * sendBuffer, int count);
void countSquirrels();
void print_log();

int initialiseController(){
    MPI_Recv(cellWorkers, LENGTH_OF_LAND, MPI_INT, 0, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return 0;
}

int controllerAsk(int workerPid){
    MPI_Send(cellWorkers, LENGTH_OF_LAND, MPI_INT, workerPid, INITIAL_TAG, MPI_COMM_WORLD);
    return workerPid;
}

int controllerWorker(){
    int i, squirlSignal;

    remainSquirrel = INITIAL_NUMBER_OF_SQUIRRELS;
    activeSquirrelWorkers = INITIAL_NUMBER_OF_SQUIRRELS;
    infectedSquirrel = INITIAL_INFECTION_LEVEL;
    totalDeadSquirrel = 0;
    month = 0;
    stopSignal = 0;

    double start, end, duration, commStart, commEnd, commDuration;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    start = MPI_Wtime();
    while(month < MONTH_LIMIT && activeSquirrelWorkers > 0 && activeSquirrelWorkers < MAX_SQUIRREL_NUMBER){
        end = MPI_Wtime();
        duration = end - start;
        if (duration > LAND_RENEW_RATE) {
            month++;

            sendRecvAllPopNInf(&month, 1);
            print_log();
            start = MPI_Wtime();
            continue;
        }

        commStart = MPI_Wtime();
        countSquirrels();
        commEnd = MPI_Wtime();
        commDuration = commEnd - commStart;
        start += commDuration;
    }

    stopSignal = SQUIRREL_STOP_SIGNAL;  // Let the land actor tell squirrels to stop
    sendAllLandCell(&stopSignal, 1);

    while (activeSquirrelWorkers) {
        countSquirrels();
    }

    stopSignal = LAND_STOP_SIGNAL;
    sendRecvAllPopNInf(&stopSignal, 1);

    if (month < 24) {
        // Print the last output if there is no enough 24 months
        printf("[Last output]");
        print_log();
    }

    printf("Controller Stop\n");
    shutdownPool();
    return 0;
}

void sendAllLandCell(int * sendBuffer, int count){
    int i;
    MPI_Request requestList[LENGTH_OF_LAND];
    MPI_Status statusList[LENGTH_OF_LAND];

    for (i=0; i<LENGTH_OF_LAND; i++) {
        MPI_Isend(sendBuffer, count, MPI_INT, cellWorkers[i], LAND_RECV_TAG, MPI_COMM_WORLD, &requestList[i]);
    }

    MPI_Waitall(LENGTH_OF_LAND, requestList, statusList);
}

void sendRecvAllPopNInf(int * sendBuffer, int count){
    int i;
    MPI_Request requestList[LENGTH_OF_LAND * 2];
    MPI_Status statusList[LENGTH_OF_LAND * 2];
    for (i=0; i<LENGTH_OF_LAND; i++) {
        MPI_Isend(sendBuffer, count, MPI_INT, cellWorkers[i], LAND_RECV_TAG, MPI_COMM_WORLD, &requestList[i*2]);
        MPI_Irecv(&popNInf[i*2], 2, MPI_INT, cellWorkers[i], CONTROLLER_RECV_TAG, MPI_COMM_WORLD, &requestList[i*2+1]);
    }

    MPI_Waitall(LENGTH_OF_LAND, requestList, statusList);
}

void countSquirrels(){
    int squirlSignal;

    MPI_Recv(&squirlSignal, 1, MPI_INT, MPI_ANY_SOURCE, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD, &status);
    if (squirlSignal == NOT_EXIST) {
        remainSquirrel--;
        infectedSquirrel--;
        activeSquirrelWorkers--;
        if (stopSignal != SQUIRREL_STOP_SIGNAL)
            // The death is counted if the squirrel is dead before being terminated
            totalDeadSquirrel++;
    } else if (squirlSignal == BORN){
        // Check if the number of squirrel out of limit,
        // then decide the squirrel can give birth or not
        if (remainSquirrel < MAX_SQUIRREL_NUMBER && stopSignal != SQUIRREL_STOP_SIGNAL) {
            squirlSignal = HEALTHY;
            remainSquirrel++;
            activeSquirrelWorkers++;
        } else {
            squirlSignal = NOT_EXIST;
        }
        MPI_Send(&squirlSignal, 1, MPI_INT, status.MPI_SOURCE, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD);
    } else if (squirlSignal == CATCH_DISEASE) {
        infectedSquirrel++;
    } else if (squirlSignal == TERMINATE) {
        activeSquirrelWorkers--;
    }
}

void print_log(){
    int i;
    printf("Month %2d\talive %d\tinfected %d\tdead %d\n", month, remainSquirrel, infectedSquirrel, totalDeadSquirrel);
    printf("POPULATION INFLUX\t[\t");
    for (i=0; i<LENGTH_OF_LAND; i++)
        printf("%d\t", popNInf[i*2]);
    printf("]\n");

    printf("INFECTION  LEVEL \t[\t");
    for (i=0; i<LENGTH_OF_LAND; i++)
        printf("%d\t", popNInf[i*2+1]);
    printf("]\n\n");
}
