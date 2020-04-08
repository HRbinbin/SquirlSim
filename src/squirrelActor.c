//
// Created by Ray on 2020/4/7.
//

#include <stdio.h>
#include <mpi.h>
#include "../include/squirrelActor.h"
#include "../include/squirrel-functions.h"
#include "../include/framework.h"
#include "../include/config.h"
#include "../include/actorConfig.h"

static long seed;
int cellWorkers[LENGTH_OF_LAND];
int controllerWorkerPid;
int rank;
float x;
float y;
int state;  // 0: does not exist; 1: healthy; 2: sick;
int steps;  // The total number of steps
int sickSteps; // The steps after being sick
int pop[LAST_POPULATION_STEPS];  // Last 50 population level
int inf[LAST_INFECTION_STEPS];  // Last 50 infection level
MPI_Status status;

int count;
int position;
int recvBuffer[2];

/** ========= The functions blow from actor framework, they will be called in framework.c ========= **/
int squirrelAsk(int workerPid);
int initialiseSquirrel();
int squirrelWorker();
/** ========= The functions blow belong to this actor ========= **/
int squirlGo();
void reproduce();
float get_avg_inf_level();
float get_avg_pop();

/**
 * @brief The function for worker asking message from the master.
 * @param[in] workerPid
 * The workers' pids.
 *
 */
int squirrelAsk(int workerPid){
    int SquirlState;
    if (sickCount < INITIAL_INFECTION_LEVEL){
        SquirlState = SICK;
        sickCount++;
    } else {
        SquirlState = HEALTHY;
    }

    MPI_Send(&SquirlState, 1, MPI_INT, workerPid, INITIAL_TAG, MPI_COMM_WORLD);
    // Tell Squirrels who is controller
    MPI_Send(&controllers[0], 1, MPI_INT, workerPid, INITIAL_TAG, MPI_COMM_WORLD);
    // Tell Squirrels who are land actors
    MPI_Send(cellWorkers, LENGTH_OF_LAND, MPI_INT, workerPid, INITIAL_TAG, MPI_COMM_WORLD);
    return workerPid;
}

/**
 * @brief The function for worker initialising after recv the message from the master.
 *
 */
int initialiseSquirrel(){
    int i, parentId;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    seed = -1-rank;
    initialiseRNG(&seed);

    x = 0;
    y = 0;

    parentId = getCommandData();

    MPI_Probe(parentId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == 10) {  // This means the process was a squirrel but dead. Now restart it.
        // This is a redundant identity, will be covered
        MPI_Recv(&state, 1, MPI_INT, parentId, IDENTITY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Recv(&state, 1, MPI_INT, parentId, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&controllerWorkerPid, 1, MPI_INT, parentId, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&cellWorkers, LENGTH_OF_LAND, MPI_INT, parentId, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (parentId == 0) {  // This means the squirrel is created by master, therefore, the x and y are randomised
        squirrelStep(x, y, &x, &y, &seed);
    } else {  // This means the squirrel is birthed by a existed squirrel, therefore, inherit parent's x and y
        float coord[2];
        MPI_Recv(&coord, 2, MPI_FLOAT, parentId, INITIAL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        x = coord[0];
        y = coord[1];
    }

    steps = 0;
    sickSteps = 0;

    for (i=0; i<LAST_POPULATION_STEPS; i++)
        pop[i] = 0;

    for (i=0; i<LAST_INFECTION_STEPS; i++)
        inf[i] = 0;

    return 0;
}

/**
 * @brief The actor work code.
 *
 */
int squirrelWorker(){
    while (state != NOT_EXIST && state != TERMINATE){
        squirlGo();

        if (state == NOT_EXIST){
            // Tell controller I am dead.
            MPI_Send(&state, 1, MPI_INT, controllerWorkerPid, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD);
        } else if (state == CATCH_DISEASE) {
            // Tell controller I am sick.
            MPI_Send(&state, 1, MPI_INT, controllerWorkerPid, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD);
            state = SICK;
        } else if (state == TERMINATE) {
            MPI_Send(&state, 1, MPI_INT, controllerWorkerPid, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD);
        }
    }
    return 0;
}

/**
 * @brief The squirrel move and try to catch disease, reproduce and die.
 *
 */
int squirlGo() {
    squirrelStep(x, y, &x, &y, &seed);
    position = getCellFromPosition(x, y);

    // Send squirrel state to Land Actor
    MPI_Send(&state, 1, MPI_INT, cellWorkers[position], LAND_RECV_TAG, MPI_COMM_WORLD);

    // Recv the population and infection level at this position
    MPI_Probe(cellWorkers[position], SQUIRREL_RECV_TAG, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &count);

    if (count == 0) {
        // Terminate signal, the squirrel should stop
        MPI_Recv(NULL, 0, MPI_INT, cellWorkers[position], SQUIRREL_RECV_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        state = TERMINATE;
        return 0;
    } else {
        // The squirrel can proceed
        MPI_Recv(recvBuffer, 2, MPI_INT, cellWorkers[position], SQUIRREL_RECV_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Update population and infection level
        pop[steps % LAST_POPULATION_STEPS] = recvBuffer[0];
        inf[steps % LAST_INFECTION_STEPS] = recvBuffer[1];

        steps++;

        if (state == SICK)
            sickSteps++;

        // The squirrel will catches disease
        if (steps > CATCH_DISEASE_STEPS && state == HEALTHY && willCatchDisease(get_avg_inf_level(), &seed))
            state = CATCH_DISEASE;

        // The squirrel will give birth
        if (steps % GIVE_BIRTH_STEPS == 0 && willGiveBirth(get_avg_pop(), &seed))
            reproduce();

        // The squirrel will die
        if (sickSteps > 50 && willDie(&seed))
            state = NOT_EXIST;

        return 1;
    }
}

/**
 * @brief The squirrel reproduce need to enquiry the controller, if the
 * controller permit then the squirrel can give birth
 *
 */
void reproduce(){
    /* Create a new process and squirrel */
    int childPid, childState, identity;
    childState = BORN;
    // Enquiry controller whether I can give birth
    MPI_Send(&childState, 1, MPI_INT, controllerWorkerPid, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD);
    MPI_Recv(&childState, 1, MPI_INT, controllerWorkerPid, SQUIRREL_CONTROLLER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // If it does not recv the BORN signal, it means the number of squirrels out of limit.
    if (childState == HEALTHY) {
        childPid = startWorkerProcess();
        identity = SQUIRREL_ACTOR;

        float coord[2];
        coord[0] = x;
        coord[1] = y;

        MPI_Send(&identity, 1, MPI_INT, childPid, IDENTITY_TAG, MPI_COMM_WORLD);

        MPI_Send(&childState, 1, MPI_INT, childPid, INITIAL_TAG, MPI_COMM_WORLD);
        // Tell baby squirrel who is controller
        MPI_Send(&controllerWorkerPid, 1, MPI_INT, childPid, INITIAL_TAG, MPI_COMM_WORLD);
        // Tell baby squirrel who are land actors
        MPI_Send(&cellWorkers, LENGTH_OF_LAND, MPI_INT, childPid, INITIAL_TAG, MPI_COMM_WORLD);

        MPI_Send(coord, 2, MPI_FLOAT, childPid, INITIAL_TAG, MPI_COMM_WORLD);
    }
}

/**
 * @brief Get the average infection level
 *
 */
float get_avg_inf_level(){
    int i;
    float avg_inf_level;
    avg_inf_level = 0.0;
    for (i = 0; i < LAST_INFECTION_STEPS; i++)
        avg_inf_level += inf[i];

    avg_inf_level /= LAST_INFECTION_STEPS;
    return avg_inf_level;
}

/**
 * @brief Get the average population influx
 *
 */
float get_avg_pop(){
    int i;
    float avg_pop;
    avg_pop = 0.0;
    for (i=0; i<LAST_POPULATION_STEPS; i++)
        avg_pop += pop[i];

    avg_pop /= LAST_POPULATION_STEPS;
    return avg_pop;
}
