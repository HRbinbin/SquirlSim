//
// Created by Ray on 2020/4/7.
//

#ifndef SQUIRLSIM_SQUIRRELACTOR_H
#define SQUIRLSIM_SQUIRRELACTOR_H

int initialiseSquirrel();
int squirrelAsk(int workerPid);
int squirrelWorker();
int squirlGo();
void reproduce();
float get_avg_inf_level();
float get_avg_pop();

#endif //SQUIRLSIM_SQUIRRELACTOR_H
