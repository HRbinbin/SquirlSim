//
// Created by Ray on 2020/4/7.
//

#ifndef SQUIRLSIM_CONTROLLERACTOR_H
#define SQUIRLSIM_CONTROLLERACTOR_H

int initialiseController();
int controllerAsk(int workerPid);
int controllerWorker();
void sendAllLandCell(int * sendBuffer, int count);
void sendRecvAllPopNInf(int * sendBuffer, int count);
void countSquirrels();
void print_log();

#endif //SQUIRLSIM_CONTROLLERACTOR_H
