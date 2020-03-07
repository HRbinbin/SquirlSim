//
// Created by Ruibin on 2020/2/28.
//
/*
 * Example code to run and test the process pool. To compile use something like mpicc -o test test.c pool.c
 */

#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "pool.h"
#include "main.h"

static long seed;

struct Squirrel initialiseSquirrel();
struct LandCells initialiseLandCells();
static void workerCode(WorkerFunc workerFunc);
int squirrelWorker();
struct LandCells updateLand(int month, int squirrelPid, struct LandCells cells, int* squirlSignal);
struct LandCells renewMonth(int month, struct LandCells cells);
struct Squirrel squirlGo(int rank, struct Squirrel squirl);


int main(int argc, char* argv[]) {
    // Call MPI initialize first
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    seed = -1-rank;
    initialiseRNG(&seed);

    /*
     * Initialise the process pool.
     * The return code is = 1 for worker to do some work, 0 for do nothing and stop and 2 for this is the master so call master poll
     * For workers this subroutine will block until the master has woken it up to do some work
     */
    int statusCode = processPoolInit();

    if (statusCode == 1) {
        // A worker so do the worker tasks
        WorkerFunc wf;
        if (rank == LAND_ACTOR){
            wf = landWorker;
        } else {
            wf = squirrelWorker;
        }

        workerCode(wf);

    } else if (statusCode == 2) {
        /*
         * This is the master, each call to master poll will block until a message is received and then will handle it and return
         * 1 to continue polling and running the pool and 0 to quit.
         * Basically it just starts 10 workers and then registers when each one has completed. When they have all completed it
         * shuts the entire pool down
         */
        int i, activeWorkers, sick_count = 0;

        // Initialise squirrels
        for (i=0;i<INITIAL_NUMBER_OF_SQUIRRELS + 1;i++) {
            int workerPid = startWorkerProcess();
            if (workerPid==LAND_ACTOR){
                continue;
            }

            int SquirlState;
            if (sick_count < INITIAL_INFECTION_LEVEL){
                SquirlState = SICK;
                sick_count++;
            } else if (workerPid < INITIAL_NUMBER_OF_SQUIRRELS) {
                SquirlState = HEALTHY;
            }
            MPI_Send(&SquirlState, 1, MPI_INT, workerPid, 1, MPI_COMM_WORLD);
            activeWorkers++;
        }

        int masterStatus = masterPoll();
        while (masterStatus) {
            masterStatus=masterPoll();
        }
    }

    // Finalizes the process pool, call this before closing down MPI
    processPoolFinalise();
    // Finalize MPI, ensure you have closed the process pool first
    MPI_Finalize();
    return 0;
}

static void workerCode(WorkerFunc workerFunc) {
    int workerStatus = 1;
    while (workerStatus) {
        workerFunc();
        workerStatus=workerSleep();	// This MPI process will sleep, further workers may be run on this process now
    }
}

int squirrelWorker(){
    struct Squirrel squirl = initialiseSquirrel();
    int rank, terminateSignal;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (squirl.state){
        // Send the rank to land to tell who I am
        MPI_Ssend(&rank, 1, MPI_INT, LAND_ACTOR, 0, MPI_COMM_WORLD);
        // Recv the permission signal
        MPI_Recv(&terminateSignal, 1, MPI_INT, LAND_ACTOR, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (terminateSignal) {
//            printf("I am Squirrel %d, I am alive but I should terminate now.\n", rank);
            break;
        } else {
            squirl = squirlGo(rank, squirl);
            if (squirl.state == NOT_EXIST){
//                printf("I am Squirrel %d, I walked %d steps, I am dead now.\n", rank, squirl.steps);
            } else if (squirl.state == BORN) {
                printf("I am Squirrel %d, I just born.\n", rank);
                squirl.state = HEALTHY;
            } else if (squirl.state == CATCH_DISEASE) {
//                printf("I am Squirrel %d, I catch disease.\n", rank);
                squirl.state = SICK;
            }
        }
    }

    return 0;
}

int landWorker(){

    struct LandCells cells = initialiseLandCells();
    double duration, wait_time, start, start_for_wait, end;
    int count, month, i, j, remainSquirrel, infectedSquirrel, totalDeadSquirrel, squirrelPid, squirlSignal, terminateSignal, activeWorkers;
    count = 0;
    month = 1;
    duration = 0;
    terminateSignal = 0;
    remainSquirrel = INITIAL_NUMBER_OF_SQUIRRELS;
    activeWorkers = INITIAL_NUMBER_OF_SQUIRRELS;
    infectedSquirrel = INITIAL_INFECTION_LEVEL;
    totalDeadSquirrel = 0;
    start = MPI_Wtime();
    start_for_wait = MPI_Wtime();

    for (i=0; i<LENGTH_OF_LAND; i++)
    do {
        if (remainSquirrel > MAX_SQUIRREL_NUMBER || terminateSignal || month >= 24) {
            terminateSignal = SQ_STOP;
        } else {
            terminateSignal = SQ_GO;
        }
        MPI_Status status;
        // Recv the request from a squirrel that want to take action
        MPI_Recv(&squirrelPid, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Send the permission to order the squirrel whether it can move or not.
        MPI_Ssend(&terminateSignal, 1, MPI_INT, squirrelPid, 2, MPI_COMM_WORLD);

        if (terminateSignal) {
            activeWorkers--;
        } else {
            cells = updateLand(month, squirrelPid, cells, &squirlSignal);
            if (squirlSignal == NOT_EXIST) {
                remainSquirrel--;
                infectedSquirrel--;
                totalDeadSquirrel++;
                activeWorkers--;
            } else if (squirlSignal == BORN){
                remainSquirrel++;
                activeWorkers--;
            } else if (squirlSignal == CATCH_DISEASE) {
                infectedSquirrel++;
            }

            end = MPI_Wtime();
            duration = end - start;
            if (duration > LAND_RENEW_RATE) {
                printf("The %d month:\n", month);
                printf("Population:\t[\t"); for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", cells.population[month % LAST_POPULATION_MONTHS][i]); printf("]\n");
                printf("Infection:\t[\t"); for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", cells.infection[month % LAST_INFECTION_MONTHS][i]); printf("]\n");
                printf("Alive: %d\tSick: %d\tDead: %d\n\n", remainSquirrel, infectedSquirrel, totalDeadSquirrel);
                month++;
                cells = renewMonth(month, cells);
                start = MPI_Wtime();
            }
            count++;
        }
    } while (activeWorkers);

    printf("The %d month:\n", month);
    printf("Population:\t[\t"); for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", cells.population[month % LAST_POPULATION_MONTHS][i]); printf("]\n");
    printf("Infection:\t[\t"); for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", cells.infection[month % LAST_INFECTION_MONTHS][i]); printf("]\n");
    printf("Alive: %d\tSick: %d\tDead: %d\n\n", remainSquirrel, infectedSquirrel, totalDeadSquirrel);
    printf("The loop has executed %d times, it is %d months\n", count, month);
    shutdownPool();

    return 0;
}

struct Squirrel initialiseSquirrel(){

    int i, state, position, parentId;
    struct Squirrel squirl;

    parentId = getCommandData();
    MPI_Recv(&state, 1, MPI_INT, parentId, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    squirl.state = state;
    squirl.steps = 0;
    squirl.sick_steps = 0;
    squirl.x = 0;
    squirl.y = 0;

    float x_new, y_new;
    squirrelStep(&squirl.x, &squirl.y, &x_new, &y_new, &seed);
    squirl.x = x_new;
    squirl.y = y_new;

    for (i=0; i<LAST_POPULATION_STEPS; i++)
        squirl.pop[i] = 0;

    for (i=0; i<LAST_INFECTION_STEPS; i++)
        squirl.inf[i] = 0;

    return squirl;
}

struct LandCells initialiseLandCells(){
    struct LandCells cells;
    int i, j, position_buffer, state_buffer;
    // Initialise population
    for (i=0; i<LAST_POPULATION_MONTHS; i++) {
        for (j=0; j<LENGTH_OF_LAND; j++){
            cells.population[i][j] = 0;
        }
    }

    // Initialise infection
    for (i=0; i<LAST_INFECTION_MONTHS; i++) {
        for (j=0; j<LENGTH_OF_LAND; j++){
            cells.infection[i][j] = 0;
        }
    }

    return cells;
}

struct Squirrel squirlGo(int rank, struct Squirrel squirl) {
    int i, position;

    // The squirrel will catches disease
    if (squirl.state == HEALTHY) {
        float avg_inf_level;
        for (i=0; i<LAST_INFECTION_STEPS; i++) {
            avg_inf_level += squirl.inf[i];
        }
        avg_inf_level /= LAST_INFECTION_STEPS;
        if (willCatchDisease(avg_inf_level, &seed)){
            squirl.state = CATCH_DISEASE;
        }
    }

    float x_new, y_new;
    squirrelStep(&squirl.x, &squirl.y, &x_new, &y_new, &seed);
    squirl.x = x_new;
    squirl.y = y_new;

    position = getCellFromPosition(&squirl.x, &squirl.y);
    // Send the squirrel's position
    int message[2], recv_buffer[2], returnCode;
    message[0] = squirl.state;
    message[1] = position;

    // Send new pos to Land Actor
    MPI_Send(message, 2, MPI_INT, LAND_ACTOR, 3, MPI_COMM_WORLD);
    // Recv the population and infection level at this position
    MPI_Recv(recv_buffer, 2, MPI_INT, LAND_ACTOR, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // Update population and infection level
    squirl.pop[squirl.steps % LAST_POPULATION_STEPS] = recv_buffer[0];
    squirl.inf[squirl.steps % LAST_INFECTION_STEPS] = recv_buffer[1];

    squirl.steps++;
    if (squirl.state == SICK){
        squirl.sick_steps++;
    }

    // The squirrel will give birth
    if (squirl.steps % GIVE_BIRTH_STEPS == 0) {
        float avg_pop;
        for (i=0; i<LAST_POPULATION_STEPS; i++) {
            avg_pop += squirl.pop[i];
        }
        avg_pop /= LAST_POPULATION_STEPS;
        if (willGiveBirth(avg_pop, &seed)){
            /* Create a new process and squirrel */
            int childPid, childState;
            childPid = startWorkerProcess();
            childState = BORN;
            MPI_Send(&childState, 1, MPI_INT, childPid, 1, MPI_COMM_WORLD);
        }
    }

    // The squirrel will die
    if (squirl.sick_steps>50) {
        if (willDie(&seed)) {
            squirl.state = NOT_EXIST;
            message[0] = squirl.state;
            int terminateSignal;
            // Send the rank to land to tell who I am
            MPI_Ssend(&rank, 1, MPI_INT, LAND_ACTOR, 0, MPI_COMM_WORLD);
            // Recv the permission signal
            MPI_Recv(&terminateSignal, 1, MPI_INT, LAND_ACTOR, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (!terminateSignal)
                MPI_Send(message, 2, MPI_INT, LAND_ACTOR, 3, MPI_COMM_WORLD);
        }
    }

    return squirl;
}

struct LandCells updateLand(int month, int squirrelPid, struct LandCells cells, int* squirlSignal){
    int returnCode, i, squirlState, recv_message_buffer[2], send_message_buffer[2];
    MPI_Status status;
    MPI_Recv(recv_message_buffer, 2, MPI_INT, squirrelPid, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    squirlState = recv_message_buffer[0];
    if (squirlState != NOT_EXIST) {
        cells.population[month % LAST_POPULATION_MONTHS][recv_message_buffer[1]]+=1;
        if (squirlState == SICK)
            cells.infection[month % LAST_INFECTION_MONTHS][recv_message_buffer[1]]+=1;

        // According to the recv position, send the population and infection level back
        send_message_buffer[0] = 0;
        send_message_buffer[1] = 0;
        for (i=0; i<LAST_POPULATION_MONTHS; i++) send_message_buffer[0]+=cells.population[i][recv_message_buffer[1]];
        for (i=0; i<LAST_INFECTION_MONTHS; i++) send_message_buffer[1]=cells.infection[month % LAST_INFECTION_MONTHS][recv_message_buffer[1]];
        MPI_Send(send_message_buffer, 2, MPI_INT, squirrelPid, 6, MPI_COMM_WORLD);
    }
    *squirlSignal = squirlState;

    return cells;
}

struct LandCells renewMonth(int month, struct LandCells cells){
    int i;
    for (i=0; i<LENGTH_OF_LAND; i++){
        cells.population[month % LAST_POPULATION_MONTHS][i] = 0;
    }

    for (i=0; i<LENGTH_OF_LAND; i++){
        cells.infection[month % LAST_INFECTION_MONTHS][i] = 0;
    }

    return cells;
}
