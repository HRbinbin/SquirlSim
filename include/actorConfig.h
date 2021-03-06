//
// Created by Ray on 2020/4/7.
//

#ifndef SQUIRLSIM_ACTORCONFIG_H
#define SQUIRLSIM_ACTORCONFIG_H
/** Stop signal **/
#define LAND_STOP_SIGNAL -1
#define SQUIRREL_STOP_SIGNAL -2

/** Actor identity **/
#define CONTROLLER_ACTOR 0
#define LAND_ACTOR 1
#define SQUIRREL_ACTOR 2

/** Squirrel state **/
#define NOT_EXIST 0
#define BORN 1
#define HEALTHY 2
#define SICK 3
#define CATCH_DISEASE 4
#define TERMINATE 5

/** Communication tag **/
#define IDENTITY_TAG 1024
#define INITIAL_TAG 1025
#define SQUIRREL_RECV_TAG 1026
#define SQUIRREL_CONTROLLER_TAG 1027
#define LAND_RECV_TAG 1028
#define CONTROLLER_RECV_TAG 1029

#endif //SQUIRLSIM_ACTORCONFIG_H
