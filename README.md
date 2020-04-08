# Squirrels disease simulation

This is a simulation of British red squirrels infecting parapoxvirus.

The infrastructure is **Master actor + Controller actor + N land actors + N squirrel actors**  

The infrastructure is **Master actor + land actor + N squirrel actors**

## Author
Ray Huang <br>
s1905483@ed.ac.uk <br>

## Environment setup

After entering directory `SquirlSim`, run
```$xslt
$ module load intel-compilers-18
$ module load mpt
```
to setup the compiler environment in order to compile the code

## Quick started

After setup, the the src code can be compiled

```
$ make
mpicc -cc=icc -I include -c -o build/squirl.o src/squirl.c
mpicc -cc=icc -I include -c -o build/ran2.o src/ran2.c
mpicc -cc=icc -I include -c -o build/test.o src/test.c
mpicc -cc=icc -I include -c -o build/pool.o src/pool.c
mpicc -cc=icc build/squirl.o build/ran2.o build/test.o build/pool.o -o bin/run -lm
```

The auto-make script generates a directory `build` with the object files. The executable file is in `bin`

Use `mpirun` to run the code. We should assign **218** processes since there are **16 land actors, 
up to 200 squirrel actors, a controller actor and a master actor**. We can change the parameters in `main.h`. 

After running `mpirun -n 218 bin/run`, we can see the simulation output of every month.

```$xslt
$ mpirun -n 218 bin/run
Month  1        alive 35        infected 4      dead 4
POPULATION INFLUX       [       157     169     177     185     180     191     179     190     166     217     173     208     194     180     173     176     ]
INFECTION  LEVEL        [       17      15      16      15      13      23      23      17      18      29      18      34      17      16      31      16      ]

Month  2        alive 36        infected 3      dead 6
POPULATION INFLUX       [       40      41      55      45      54      50      41      38      41      41      42      52      46      49      42      51      ]
INFECTION  LEVEL        [       5       4       7       4       3       7       8       5       7       6       3       11      11      8       9       8       ]

Month  3        alive 38        infected 7      dead 9
POPULATION INFLUX       [       104     99      127     91      123     90      105     94      113     125     92      107     110     117     106     121     ]
INFECTION  LEVEL        [       17      14      19      17      15      15      21      14      15      13      17      17      21      17      11      19      ]

Month  4        alive 35        infected 9      dead 15
POPULATION INFLUX       [       103     82      97      95      93      82      73      89      82      88      83      74      97      85      81      86      ]
INFECTION  LEVEL        [       26      21      23      23      22      18      18      26      24      24      17      11      17      20      17      22      ]

Month  5        alive 39        infected 9      dead 19
POPULATION INFLUX       [       64      63      67      67      69      74      57      63      69      61      70      61      71      72      46      69      ]
INFECTION  LEVEL        [       16      12      18      18      17      21      24      23      21      18      23      18      22      26      14      17      ]

Month  6        alive 34        infected 6      dead 27
POPULATION INFLUX       [       84      109     88      113     83      99      115     100     104     119     95      96      105     83      118     105     ]
INFECTION  LEVEL        [       14      25      19      26      11      17      23      18      20      19      17      21      19      21      30      18      ]

Month  7        alive 37        infected 11     dead 30
POPULATION INFLUX       [       86      93      86      106     101     79      84      67      77      79      67      87      84      81      97      85      ]
INFECTION  LEVEL        [       17      23      20      33      21      14      16      13      18      23      13      26      18      17      17      14      ]

Month  8        alive 40        infected 11     dead 32
POPULATION INFLUX       [       24      24      19      23      25      25      14      17      35      20      25      17      19      19      23      17      ]
INFECTION  LEVEL        [       7       5       6       7       6       9       4       6       13      6       14      4       9       3       12      1       ]

Month  9        alive 41        infected 6      dead 39
POPULATION INFLUX       [       108     104     96      90      130     98      101     102     92      90      25      110     121     19      93      112     ]
INFECTION  LEVEL        [       25      24      15      19      24      20      21      20      11      22      14      23      28      3       26      26      ]

Month 10        alive 40        infected 11     dead 44
POPULATION INFLUX       [       84      73      92      92      97      84      92      93      93      92      86      86      86      78      84      83      ]
INFECTION  LEVEL        [       15      19      19      15      17      19      16      16      17      19      20      7       19      13      20      15      ]

Month 11        alive 39        infected 12     dead 50
POPULATION INFLUX       [       91      105     81      85      103     98      121     103     114     88      106     108     109     75      111     102     ]
INFECTION  LEVEL        [       39      35      24      32      34      34      45      42      43      34      37      42      33      26      43      30      ]

Month 12        alive 35        infected 12     dead 57
POPULATION INFLUX       [       62      42      43      54      60      53      50      58      54      67      38      68      70      41      48      45      ]
INFECTION  LEVEL        [       17      10      11      16      21      16      14      6       13      17      7       19      23      12      9       8       ]

Month 13        alive 39        infected 16     dead 60
POPULATION INFLUX       [       74      71      66      63      76      77      70      71      67      61      65      79      74      71      56      80      ]
INFECTION  LEVEL        [       26      30      17      20      32      28      25      26      19      20      17      30      29      23      16      28      ]

Month 14        alive 32        infected 11     dead 70
POPULATION INFLUX       [       76      75      68      62      59      77      75      75      85      63      74      63      74      67      69      77      ]
INFECTION  LEVEL        [       40      42      32      28      25      30      27      36      33      27      35      27      31      24      31      31      ]

Month 15        alive 27        infected 4      dead 80
POPULATION INFLUX       [       82      70      68      72      68      65      64      70      85      63      68      68      74      91      69      63      ]
INFECTION  LEVEL        [       20      26      14      22      28      21      17      14      33      27      11      16      31      27      31      18      ]

Month 16        alive 25        infected 9      dead 85
POPULATION INFLUX       [       92      84      96      72      93      83      92      84      88      103     84      70      92      104     83      94      ]
INFECTION  LEVEL        [       32      29      32      24      22      24      28      23      24      34      35      16      27      28      20      28      ]

Month 17        alive 23        infected 5      dead 92
POPULATION INFLUX       [       52      59      31      44      37      46      63      46      41      54      46      57      54      51      61      58      ]
INFECTION  LEVEL        [       19      16      9       12      12      13      21      13      19      15      10      23      19      17      19      14      ]

Month 18        alive 23        infected 4      dead 98
POPULATION INFLUX       [       78      59      83      83      81      73      85      48      74      76      73      62      57      72      86      77      ]
INFECTION  LEVEL        [       23      15      20      29      26      24      20      13      18      28      16      17      14      22      19      14      ]

Month 19        alive 25        infected 12     dead 101
POPULATION INFLUX       [       55      63      61      63      66      73      64      61      52      68      59      57      83      58      63      53      ]
INFECTION  LEVEL        [       21      36      23      25      29      32      28      25      26      29      28      28      37      20      25      24      ]

Month 20        alive 19        infected 6      dead 110
POPULATION INFLUX       [       44      65      50      45      50      47      55      61      44      51      49      52      52      51      49      52      ]
INFECTION  LEVEL        [       21      34      21      16      27      21      22      31      20      26      25      26      18      21      20      27      ]

Month 21        alive 20        infected 8      dead 115
POPULATION INFLUX       [       40      53      55      47      48      66      53      43      49      42      56      46      47      48      54      43      ]
INFECTION  LEVEL        [       14      16      20      20      24      17      15      18      11      19      22      14      20      17      20      18      ]

Month 22        alive 17        infected 6      dead 122
POPULATION INFLUX       [       50      57      59      54      46      46      60      49      47      57      58      55      56      60      52      61      ]
INFECTION  LEVEL        [       17      12      19      16      13      21      21      20      16      23      18      13      17      15      13      22      ]

Month 23        alive 18        infected 5      dead 128
POPULATION INFLUX       [       87      88      80      96      100     80      70      84      89      79      101     93      89      63      86      92      ]
INFECTION  LEVEL        [       27      32      26      24      28      19      20      18      21      26      26      29      32      13      26      29      ]

Month 24        alive 19        infected 8      dead 132
POPULATION INFLUX       [       61      56      46      60      42      50      55      62      63      54      73      39      57      66      77      62      ]
INFECTION  LEVEL        [       18      21      15      16      12      18      15      21      20      20      25      14      21      22      26      20      ]

Controller Stop
Master Quit. Runtime 10.474666
```

or submit the batch job to backend. There are `.pbs` file for Cirrus and Archer

For example, submit the batch job in Cirrus
```
$ qsub simulation_cirrus.pbs
```

## Simulation Parameters setting

The parameters in simulation can be modified in `include/config.h`.

```
/** Controller parameters **/
#define CONTROLLER_NUMBER 1

/** Land parameters **/
#define LENGTH_OF_LAND 16
#define MONTH_LIMIT 24
#define LAND_RENEW_RATE 0.000002
#define LAST_POPULATION_MONTHS 3
#define LAST_INFECTION_MONTHS 2

/** Squirrel parameters **/
#define MAX_SQUIRREL_NUMBER 200
#define INITIAL_NUMBER_OF_SQUIRRELS 34
#define INITIAL_INFECTION_LEVEL 4
#define GIVE_BIRTH_STEPS 50
#define CATCH_DISEASE_STEPS 50
#define LAST_POPULATION_STEPS 50
#define LAST_INFECTION_STEPS 50
```

`CONTROLLER_NUMBER` is the number of controller actor. In this simulation, **we are against 
that the user set it more than 1.**<br>
`LENGTH_OF_LAND` is the number of land actors. <br>
`MONTH_LIMIT` is the number of land actors. <br>
`LAND_RENEW_RATE` is the time of a month that the land update the population influx and infection level. <br>
`LAST_POPULATION_MONTHS` Land update the population influx after this number of months. <br>
`LAST_INFECTION_MONTHS` Land update the infection level after this number of months. <br>

`MAX_SQUIRREL_NUMBER` is the maximum number of Squirrels. The controller terminate the simulation if the 
number of active squirrels is over this number<br>
`INITIAL_NUMBER_OF_SQUIRRELS` is the number of initial squirrels. <br>
`INITIAL_INFECTION_LEVEL` is the number of initial sick squirrels. <br>
`LAST_POPULATION_STEPS` Squirrels are going to try give birth in every this number steps. <br>
`GIVE_BIRTH_STEPS` Squirrels try to give birth in every this number steps. <br>
`CATCH_DISEASE_STEPS` Squirrels try to catch disease after this number steps. <br>
`CATCH_DISEASE_STEPS` Squirrels try to catch disease after this number steps. <br>
`LAST_POPULATION_STEPS` Squirrels try to give birth according to the average of this number steps population influx. <br>
`LAST_INFECTION_STEPS` Squirrels try to catch disease according to the average of this number steps infection level. <br>