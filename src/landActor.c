//
// Created by Ray on 2020/4/7.
//

#include <stdio.h>
#include <mpi.h>
#include "../include/framework.h"
#include "../include/landActor.h"
#include "../include/config.h"
#include "../include/actorConfig.h"

int controllerWorkerPid;
int cellWorkers[LENGTH_OF_LAND];
int population[LAST_POPULATION_MONTHS];
int infection[LAST_INFECTION_MONTHS];
int squirlState;
int sendBuffer[2];
MPI_Group landGroup;
MPI_Comm landComm;
MPI_Status status;

int initialiseLandCell();
int landAsk(int workerPid);
int landWorker();
void updateLand(int month, MPI_Status status);
void renewMonth(int month);
void terminateSquirrel(MPI_Status status);
void landInitialiseMessage();

void landInitialiseMessage(){
    MPI_Recv(&controllerWorkerPid, 1, MPI_INT, 0, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(cellWorkers, LENGTH_OF_LAND, MPI_INT, 0, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // Create a communicator for land actors group
    MPI_Group_incl(worldGroup, LENGTH_OF_LAND, cellWorkers, &landGroup);
    MPI_Comm_create(MPI_COMM_WORLD, landGroup, &landComm);
}

int landAsk(int workerPid) {
    MPI_Send(&controllers[0], 1, MPI_INT, workerPid, INITIAL_TAG, MPI_COMM_WORLD);
    // Send the message to tell the land's pids to all land actors for create group communicators.
    MPI_Send(cellWorkers, LENGTH_OF_LAND, MPI_INT, workerPid, INITIAL_TAG, MPI_COMM_WORLD);
    return workerPid;
}

int landWorker(){
    int permissionSignal, probeFlag, receiveMonth, month;

    permissionSignal = 1;
    month = 0;
    receiveMonth = 0;

    while (1){
        // Recv the request from other workers
        MPI_Iprobe(MPI_ANY_SOURCE, LAND_RECV_TAG, MPI_COMM_WORLD, &probeFlag, &status);
        if (probeFlag) {
            if (status.MPI_SOURCE == controllerWorkerPid) {
                // This is the message from controller for update month
                MPI_Recv(&receiveMonth, 1, MPI_INT, status.MPI_SOURCE, LAND_RECV_TAG, MPI_COMM_WORLD, &status);

                if (receiveMonth == LAND_STOP_SIGNAL) {
                    sendBuffer[0] = population[month % LAST_POPULATION_MONTHS];
                    sendBuffer[1] = infection[month % LAST_INFECTION_MONTHS];
                    MPI_Send(sendBuffer, 2, MPI_INT, status.MPI_SOURCE, CONTROLLER_RECV_TAG, MPI_COMM_WORLD);
                    break;
                } else if (receiveMonth == SQUIRREL_STOP_SIGNAL) {
                    permissionSignal = 0;
                    continue;
                }

                month = receiveMonth;

                sendBuffer[0] = population[(month - 1) % LAST_POPULATION_MONTHS];
                sendBuffer[1] = infection[(month - 1) % LAST_INFECTION_MONTHS];
                MPI_Send(sendBuffer, 2, MPI_INT, status.MPI_SOURCE, CONTROLLER_RECV_TAG, MPI_COMM_WORLD);
                renewMonth(month);
                MPI_Barrier(landComm);
            } else {
                // This is the message from squirrels for update cell
                if (permissionSignal)
                    updateLand(month, status);
                else
                    terminateSquirrel(status);
            }
        }
    }

    return 0;
}

int initialiseLandCell(){
    landInitialiseMessage();

    int i;
    // Initialise population
    for (i=0; i<LAST_POPULATION_MONTHS; i++)
        population[i] = 0;

    // Initialise infection
    for (i=0; i<LAST_INFECTION_MONTHS; i++)
        infection[i] = 0;

    return 0;
}

void updateLand(int month, MPI_Status status){
    MPI_Recv(&squirlState, 1, MPI_INT, status.MPI_SOURCE, LAND_RECV_TAG, MPI_COMM_WORLD, &status);

    population[month % LAST_POPULATION_MONTHS]+=1;
    if (squirlState == SICK)
        infection[month % LAST_INFECTION_MONTHS]+=1;

    // According to the recv position, send the population and infection level back
    sendBuffer[0]=population[month % LAST_POPULATION_MONTHS];
    sendBuffer[1]=infection[month % LAST_INFECTION_MONTHS];
    MPI_Send(sendBuffer, 2, MPI_INT, status.MPI_SOURCE, SQUIRREL_RECV_TAG, MPI_COMM_WORLD);
}

void terminateSquirrel(MPI_Status status){
    MPI_Recv(&squirlState, 1, MPI_INT, status.MPI_SOURCE, LAND_RECV_TAG, MPI_COMM_WORLD, &status);
    MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, SQUIRREL_RECV_TAG, MPI_COMM_WORLD);
}

void renewMonth(int month){
    population[month % LAST_POPULATION_MONTHS] = 0;
    infection[month % LAST_INFECTION_MONTHS] = 0;
}