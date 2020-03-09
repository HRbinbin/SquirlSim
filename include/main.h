//
// Created by Ruibin on 2020/2/28.
//

#ifndef SQUIRLSIM_MAIN_H
#define SQUIRLSIM_MAIN_H

#define CONTROLLER_ACTOR 0
#define LAND_ACTOR 1
#define SQUIRREL_ACTOR 2

#define NOT_EXIST 0
#define BORN 1
#define HEALTHY 2
#define SICK 3
#define CATCH_DISEASE 4
#define TERMINATE 5

#define SQ_GO 0
#define SQ_STOP 1

#define LENGTH_OF_LAND 16
#define MONTH_LIMIT 24
//#define LAND_RENEW_RATE 0.1
#define LAND_RENEW_RATE 0.00001
#define LAND_STOP_SIGNAL -1
#define SQUIRREL_STOP_SIGNAL -2

#define MAX_SQUIRREL_NUMBER 37
#define INITIAL_NUMBER_OF_SQUIRRELS 34
#define INITIAL_INFECTION_LEVEL 4
#define GIVE_BIRTH_STEPS 1

#define LAST_POPULATION_STEPS 50
#define LAST_INFECTION_STEPS 50
#define LAST_POPULATION_MONTHS 3
#define LAST_INFECTION_MONTHS 2

//#define LAND_ACTOR 1

// A structure record a squirrel's data
struct Squirrel {
    float x;
    float y;
    int state;  // 0: does not exist; 1: healthy; 2: sick;
    int steps;  // The total number of steps
    int sickSteps; // The steps after being sick
    int pop[LAST_POPULATION_STEPS];  // Last 50 population level
    int inf[LAST_INFECTION_STEPS];  // Last 50 infection level
};

// The land cell
struct LandCell {
    int population[LAST_POPULATION_MONTHS];
    int infection[LAST_INFECTION_MONTHS];
};

typedef int (*WorkerFunc)();

struct Squirrel initialiseSquirrel();
struct LandCell initialiseLandCell();
static void workerCode(WorkerFunc workerFunc);
int squirrelWorker();
int landWorker();
int controllerWorker();
struct LandCell updateLand(int month, int squirrelPid, struct LandCell cell);
struct LandCell renewMonth(int month, struct LandCell cell);
struct Squirrel squirlGo(int rank, struct Squirrel squirl);

#endif //SQUIRLSIM_MAIN_H
