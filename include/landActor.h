//
// Created by Ray on 2020/4/7.
//

#ifndef SQUIRLSIM_LANDACTOR_H
#define SQUIRLSIM_LANDACTOR_H

void landInitialiseMessage();
int initialiseLandCell();
int landAsk(int workerPid);
int landWorker();
void updateLand(int month, MPI_Status status);
void renewMonth(int month);
void terminateSquirrel(MPI_Status status);

#endif //SQUIRLSIM_LANDACTOR_H
