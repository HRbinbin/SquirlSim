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
int cellWorkers[LENGTH_OF_LAND];
int controllerWorkerPid;
MPI_Group worldGroup, landGroup;
MPI_Comm landComm;

struct Squirrel initialiseSquirrel();
struct LandCell initialiseLandCell();
static void workerCode(WorkerFunc workerFunc);
int squirrelWorker();
int landWorker();
int controllerWorker();
struct LandCell updateLand(int month, int squirrelPid, struct LandCell cell);
struct LandCell renewMonth(int month, struct LandCell cell);
struct Squirrel squirlGo(int rank, struct Squirrel squirl);


int main(int argc, char* argv[]) {
    // Call MPI initialize first
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    seed = -1-rank;
    initialiseRNG(&seed);

    MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);

    /*
     * Initialise the process pool.
     * The return code is = 1 for worker to do some work, 0 for do nothing and stop and 2 for this is the master so call master poll
     * For workers this subroutine will block until the master has woken it up to do some work
     */
    int statusCode = processPoolInit();

    if (statusCode == 1) {
        int identity;
        MPI_Recv(&identity, 1, MPI_INT, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//        printf("I am rank %d, I am a %d\n", rank, identity);
        // A worker so do the worker tasks
        WorkerFunc wf;

        if (identity == CONTROLLER_ACTOR) {
            MPI_Recv(cellWorkers, LENGTH_OF_LAND, MPI_INT, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            wf = controllerWorker;
        } else if (identity == LAND_ACTOR){
            MPI_Recv(&controllerWorkerPid, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(cellWorkers, LENGTH_OF_LAND, MPI_INT, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Create a communicator for land actors group
            MPI_Group_incl(worldGroup, LENGTH_OF_LAND, cellWorkers, &landGroup);
            MPI_Comm_create(MPI_COMM_WORLD, landGroup, &landComm);

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
        int i, sickCount = 0, identity;

        // Initial controller
        int workerPid = startWorkerProcess();
        identity = CONTROLLER_ACTOR;
        // Tell the process that it is a controller
        MPI_Ssend(&identity, 1, MPI_INT, workerPid, 10, MPI_COMM_WORLD);
        controllerWorkerPid = workerPid;

        // Initial land actors
        for (i=0;i<LENGTH_OF_LAND;i++) {
            int workerPid = startWorkerProcess();
            identity = LAND_ACTOR;
            // Tell processes that they are land actors
            MPI_Ssend(&identity, 1, MPI_INT, workerPid, 10, MPI_COMM_WORLD);
            MPI_Ssend(&controllerWorkerPid, 1, MPI_INT, workerPid, 10, MPI_COMM_WORLD);
            cellWorkers[i] = workerPid;
        }

        MPI_Ssend(cellWorkers, LENGTH_OF_LAND, MPI_INT, controllerWorkerPid, 10, MPI_COMM_WORLD);

        for (i=0;i<LENGTH_OF_LAND;i++) {
            // Send the message to tell the land's pids to all land actors.
            MPI_Ssend(cellWorkers, LENGTH_OF_LAND, MPI_INT, cellWorkers[i], 10, MPI_COMM_WORLD);
        }

        // Initial squirrel actors
        for (i=0;i<INITIAL_NUMBER_OF_SQUIRRELS;i++) {
            int workerPid = startWorkerProcess();
            identity = SQUIRREL_ACTOR;
            // Tell processes that they are squirrel actors
            MPI_Ssend(&identity, 1, MPI_INT, workerPid, 10, MPI_COMM_WORLD);
            int SquirlState;
            if (sickCount < INITIAL_INFECTION_LEVEL){
                SquirlState = SICK;
                sickCount++;
            } else  {
                SquirlState = HEALTHY;
            }
            MPI_Send(&SquirlState, 1, MPI_INT, workerPid, 1, MPI_COMM_WORLD);
            // Tell Squirrels who is controller
            MPI_Ssend(&controllerWorkerPid, 1, MPI_INT, workerPid, 11, MPI_COMM_WORLD);
            // Tell Squirrels who are land actors
            MPI_Ssend(&cellWorkers, LENGTH_OF_LAND, MPI_INT, workerPid, 12, MPI_COMM_WORLD);
        }


        int masterStatus = masterPoll();
        while (masterStatus) {
            masterStatus=masterPoll();
        }
        printf("Master Quit\n");
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
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (squirl.state != NOT_EXIST && squirl.state != TERMINATE){
        squirl = squirlGo(rank, squirl);
        if (squirl.state == NOT_EXIST){
            // Tell controller I am dead.
//            printf("I am Squirrel %d, I walked %d steps, I am dead now.\n", rank, squirl.steps);
            MPI_Ssend(&squirl.state, 1, MPI_INT, controllerWorkerPid, 99, MPI_COMM_WORLD);

        } else if (squirl.state == CATCH_DISEASE) {
            // Tell controller I am sick.
            MPI_Ssend(&squirl.state, 1, MPI_INT, controllerWorkerPid, 99, MPI_COMM_WORLD);
//            printf("I am Squirrel %d, I catch disease.\n", rank);
            squirl.state = SICK;
        } else if (squirl.state == TERMINATE) {
            MPI_Ssend(&squirl.state, 1, MPI_INT, controllerWorkerPid, 99, MPI_COMM_WORLD);
//            printf("I am Squirrel %d, I am still alive. But I should stop.\n", rank);
        }
    }
//    printf("I am Squirrel %d, I quit, my state is %d\n", rank, squirl.state);
    return 0;
}

int landWorker(){
    struct LandCell cell = initialiseLandCell();
    int permissionSignal, terminateSignal, squirrelPid, activeSignal, month, flag;
    MPI_Request request;
    MPI_Status status;

    permissionSignal = 1;
    terminateSignal = 1;
    month = 0;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (1){
        if (month == SQUIRREL_STOP_SIGNAL) {
//            MPI_Barrier(landComm);
            permissionSignal = 0;
        }

        // Recv the request from other workers
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        if (status.MPI_SOURCE == controllerWorkerPid) {
            // This is the message from controller for update month
            MPI_Recv(&month, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);

            if (month == LAND_STOP_SIGNAL) {
                break;
            }
            else if (month == SQUIRREL_STOP_SIGNAL) {
//                printf("I am land %d, I know squirrel permission signal, ", rank);
                continue;
            }

            MPI_Ssend(&cell.population[(month-1) % LAST_POPULATION_MONTHS], 1, MPI_INT, status.MPI_SOURCE, 199, MPI_COMM_WORLD);
            MPI_Ssend(&cell.infection[(month-1) % LAST_INFECTION_MONTHS], 1, MPI_INT, status.MPI_SOURCE, 199, MPI_COMM_WORLD);
            cell = renewMonth(month, cell);
            MPI_Barrier(landComm);
        } else {
            // This is the message from squirrels for update cell
            MPI_Recv(&squirrelPid, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
            MPI_Ssend(&permissionSignal, 1, MPI_INT, squirrelPid, 2, MPI_COMM_WORLD);
            if (permissionSignal) {
                cell = updateLand(month, squirrelPid, cell);
            } else {
//                printf("squirrel permission signal sent to sq %d\n", status.MPI_SOURCE);
            }
        }
    }

//    printf("I am rank %d, I am stop, permission %d, month %d\n", rank, permissionSignal, month);

    return 0;
}

int controllerWorker(){
    int i, flag, activeLandWorkers, activeSquirrelWorkers, month, permissionSignal, stopSignal,
    remainSquirrel, infectedSquirrel, totalDeadSquirrel, squirlSignal,
    populationBuffer[LENGTH_OF_LAND], infectionBuffer[LENGTH_OF_LAND];
    MPI_Status status;
    permissionSignal = 1;
    remainSquirrel = INITIAL_NUMBER_OF_SQUIRRELS;
    activeSquirrelWorkers = INITIAL_NUMBER_OF_SQUIRRELS;
    activeLandWorkers = LENGTH_OF_LAND;
    infectedSquirrel = INITIAL_INFECTION_LEVEL;
    totalDeadSquirrel = 0;
    month = 0;

    double start, end, duration, commStart, commEnd, commDuration, printStart, printEnd, printDuration;
    commDuration = 0;
    printDuration = 0;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    start = MPI_Wtime();
    while(activeLandWorkers){
        if (month < MONTH_LIMIT && activeSquirrelWorkers > 0 && activeSquirrelWorkers < MAX_SQUIRREL_NUMBER) {
            end = MPI_Wtime();
            duration = end - start - commDuration - printDuration;
            if (duration > LAND_RENEW_RATE) {
                month++;
                for (i=0; i<LENGTH_OF_LAND; i++) {
                    MPI_Ssend(&month, 1, MPI_INT, cellWorkers[i], 0, MPI_COMM_WORLD);
                    MPI_Recv(&populationBuffer[i], 1, MPI_INT, cellWorkers[i], 199, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(&infectionBuffer[i], 1, MPI_INT, cellWorkers[i], 199, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                start = MPI_Wtime();
                printStart = MPI_Wtime();
                printf("Month %d\talive %d\tinfected %d\tdead %d\n[\t", month, remainSquirrel, infectedSquirrel, totalDeadSquirrel);
                for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", populationBuffer[i]); printf("]\n[\t");
                for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", infectionBuffer[i]); printf("]\n\n");
                printEnd = MPI_Wtime();
                printDuration = printEnd - printStart;
            }

            commStart = MPI_Wtime();
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == 99) {  // This is a message from squirrel actor
                MPI_Recv(&squirlSignal, 1, MPI_INT, status.MPI_SOURCE, 99, MPI_COMM_WORLD, &status);
                if (squirlSignal == NOT_EXIST) {
                    remainSquirrel--;
                    infectedSquirrel--;
                    totalDeadSquirrel++;
                    activeSquirrelWorkers--;
                } else if (squirlSignal == BORN){
                    // Check if the number of squirrel out of limit,
                    // then decide the squirrel can give birth or not
                    if (remainSquirrel < MAX_SQUIRREL_NUMBER) {
                        squirlSignal = HEALTHY;
                        remainSquirrel++;
                        activeSquirrelWorkers++;
                    } else {
                        squirlSignal = NOT_EXIST;
                        activeSquirrelWorkers++;
                    }
                    MPI_Ssend(&squirlSignal, 1, MPI_INT, status.MPI_SOURCE, 100, MPI_COMM_WORLD);
                } else if (squirlSignal == CATCH_DISEASE) {
                    infectedSquirrel++;
                } else if (squirlSignal == TERMINATE) {
                    activeSquirrelWorkers--;
                }
            }

            commEnd = MPI_Wtime();
            commDuration = commEnd - commStart;
        } else if (activeSquirrelWorkers > 0) {
            stopSignal = SQUIRREL_STOP_SIGNAL;  // Let the land actor tell squirrels to stop
            for (i=0; i<LENGTH_OF_LAND; i++) {
                MPI_Ssend(&stopSignal, 1, MPI_INT, cellWorkers[i], 0, MPI_COMM_WORLD);
            }

            while (activeSquirrelWorkers) {
                MPI_Recv(&squirlSignal, 1, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &status);
//                printf("squirlSignal %d, from sq %d ", squirlSignal, status.MPI_SOURCE);
                if (squirlSignal == NOT_EXIST || squirlSignal == TERMINATE)
                    activeSquirrelWorkers--;
                else if (squirlSignal == BORN){
                    // Check if the number of squirrel out of limit,
                    // then decide the squirrel can give birth or not
                    squirlSignal = NOT_EXIST;
                    MPI_Ssend(&squirlSignal, 1, MPI_INT, status.MPI_SOURCE, 100, MPI_COMM_WORLD);
//                    activeSquirrelWorkers--;
                }
//                printf("remain squirrel actor %d\n", activeSquirrelWorkers);
            }

        } else {
            stopSignal = LAND_STOP_SIGNAL;
            for (i=0; i<LENGTH_OF_LAND; i++) {
                MPI_Ssend(&stopSignal, 1, MPI_INT, cellWorkers[i], 0, MPI_COMM_WORLD);
                activeLandWorkers--;
//                printf("remain land actor %d\n", activeLandWorkers);
            }
        }
    }

    printf("[Last Output] Month %d\talive %d\tinfected %d\tdead %d\n[\t", month, remainSquirrel, infectedSquirrel, totalDeadSquirrel);
    for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", populationBuffer[i]); printf("]\n[\t");
    for (i=0; i<LENGTH_OF_LAND; i++) printf("%d\t", infectionBuffer[i]); printf("]\n\n");
    printf("Controller Stop\n");
    shutdownPool();
    return 0;
}

struct Squirrel initialiseSquirrel(){

    int i, state, position, parentId;
    struct Squirrel squirl;
    MPI_Status status;

    parentId = getCommandData();

    MPI_Probe(parentId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == 10) {
        MPI_Recv(&state, 1, MPI_INT, parentId, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&state, 1, MPI_INT, parentId, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&controllerWorkerPid, 1, MPI_INT, parentId, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&cellWorkers, LENGTH_OF_LAND, MPI_INT, parentId, 12, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        MPI_Recv(&state, 1, MPI_INT, parentId, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&controllerWorkerPid, 1, MPI_INT, parentId, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&cellWorkers, LENGTH_OF_LAND, MPI_INT, parentId, 12, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    squirl.state = state;
    squirl.steps = 0;
    squirl.sickSteps = 0;
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

struct LandCell initialiseLandCell(){
    struct LandCell cell;
    int i, j;
    // Initialise population
    for (i=0; i<LAST_POPULATION_MONTHS; i++) {
        cell.population[i] = 0;
    }

    // Initialise infection
    for (i=0; i<LAST_INFECTION_MONTHS; i++) {
        cell.infection[i] = 0;
    }

    return cell;
}

struct Squirrel squirlGo(int rank, struct Squirrel squirl) {
    int i, position;
    float x_new, y_new;
    squirrelStep(&squirl.x, &squirl.y, &x_new, &y_new, &seed);
    squirl.x = x_new;
    squirl.y = y_new;

    position = getCellFromPosition(&squirl.x, &squirl.y);
    // Send the squirrel's position
    int recvBuffer[2], permissionSignal;

    // Send the rank to land to tell who I am
    MPI_Ssend(&rank, 1, MPI_INT, cellWorkers[position], 0, MPI_COMM_WORLD);
    // Recv the permission signal
    MPI_Recv(&permissionSignal, 1, MPI_INT, cellWorkers[position], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (!permissionSignal) {
        squirl.state = TERMINATE;
        return squirl;
    }

    // Send squirrel state to Land Actor
    MPI_Send(&squirl.state, 1, MPI_INT, cellWorkers[position], 3, MPI_COMM_WORLD);
    // Recv the population and infection level at this position
    MPI_Recv(recvBuffer, 2, MPI_INT, cellWorkers[position], 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // Update population and infection level
    squirl.pop[squirl.steps % LAST_POPULATION_STEPS] = recvBuffer[0];
    squirl.inf[squirl.steps % LAST_INFECTION_STEPS] = recvBuffer[1];

    squirl.steps++;
    if (squirl.state == SICK){
        squirl.sickSteps++;
    }

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

    // The squirrel will give birth
    if (squirl.steps % GIVE_BIRTH_STEPS == 0) {
        float avg_pop;
        for (i=0; i<LAST_POPULATION_STEPS; i++) {
            avg_pop += squirl.pop[i];
        }
        avg_pop /= LAST_POPULATION_STEPS;
        if (willGiveBirth(avg_pop, &seed)){
            /* Create a new process and squirrel */
            int childPid, childState, identity;
            childState = BORN;
            // Enquiry controller whether I can give birth
            MPI_Ssend(&childState, 1, MPI_INT, controllerWorkerPid, 99, MPI_COMM_WORLD);
            MPI_Recv(&childState, 1, MPI_INT, controllerWorkerPid, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // If it does not recv the BORN signal, it means the number of squirrels out of limit.
            if (childState == HEALTHY) {
                childPid = startWorkerProcess();

                identity = SQUIRREL_ACTOR;
                MPI_Ssend(&identity, 1, MPI_INT, childPid, 10, MPI_COMM_WORLD);

                MPI_Ssend(&childState, 1, MPI_INT, childPid, 1, MPI_COMM_WORLD);
                // Tell Squirrels who is controller

                MPI_Ssend(&controllerWorkerPid, 1, MPI_INT, childPid, 11, MPI_COMM_WORLD);
                // Tell Squirrels who are land actors
                MPI_Ssend(&cellWorkers, LENGTH_OF_LAND, MPI_INT, childPid, 12, MPI_COMM_WORLD);
            }
        }
    }

    // The squirrel will die
    if (squirl.sickSteps>50) {
        if (willDie(&seed)) {
            squirl.state = NOT_EXIST;
        }
    }

    return squirl;
}

struct LandCell updateLand(int month, int squirrelPid, struct LandCell cell){
    int i, squirlState, sendBuffer[2];
    MPI_Status status;
    MPI_Recv(&squirlState, 1, MPI_INT, squirrelPid, 3, MPI_COMM_WORLD, &status);

    cell.population[month % LAST_POPULATION_MONTHS]+=1;
    if (squirlState == SICK)
        cell.infection[month % LAST_INFECTION_MONTHS]+=1;

    // According to the recv position, send the population and infection level back
    sendBuffer[0] = 0;
    sendBuffer[1] = 0;
    for (i=0; i<LAST_POPULATION_MONTHS; i++) sendBuffer[0]+=cell.population[i];
    for (i=0; i<LAST_INFECTION_MONTHS; i++) sendBuffer[1]=cell.infection[month % LAST_INFECTION_MONTHS];
    MPI_Ssend(sendBuffer, 2, MPI_INT, squirrelPid, 6, MPI_COMM_WORLD);

    return cell;
}

struct LandCell renewMonth(int month, struct LandCell cell){
    cell.population[month % LAST_POPULATION_MONTHS] = 0;
    cell.infection[month % LAST_INFECTION_MONTHS] = 0;
    return cell;
}
