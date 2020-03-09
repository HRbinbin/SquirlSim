# Squirrels disease simulation

This is a simulation of British red squirrels infecting parapoxvirus.

The infrastructure is **Master actor + Controller actor + N land actors + N squirrel actors**  

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
mpicc -cc=icc build/squirl.o build/ran2.o build/test.o build/pool.o -o bin/test -lm
```

The auto-make script generates a directory `build` with the object files. The executable file is in `bin`

Use `mpirun` to run the code. We should assign 218 processes since there are 16 land actors, 
up to 200 squirrel actors, a controller actor and a master actor. We can change the parameters in `main.h`. 

After running `mpirun -n 218 bin/test`, we can see the simulation output of every month.

```$xslt
$ mpirun -n 218 bin/test
Month 1 alive 34        infected 6      dead 0
[       14      17      15      11      17      20      21      15      22      16      19      15      15      28      20      16      ]
[       8       3       8       5       4       7       4       5       7       6       2       3       5       6       8       5       ]

Month 2 alive 34        infected 8      dead 0
[       0       6       5       7       4       9       8       5       8       6       5       7       4       2       6       9       ]
[       0       2       1       2       1       2       4       0       3       4       1       3       2       0       1       3       ]

Month 3 alive 34        infected 10     dead 0
[       23      14      14      12      20      14      11      5       14      15      12      17      10      19      24      16      ]
[       5       7       4       5       8       12      5       1       7       6       4       10      4       9       11      7       ]

Month 4 alive 34        infected 12     dead 0
[       0       3       1       2       2       4       2       6       1       3       8       2       2       4       4       4       ]
[       0       1       0       0       2       0       0       1       1       2       2       0       2       2       0       1       ]

Month 5 alive 34        infected 15     dead 0
[       20      8       14      9       10      9       10      11      15      11      9       16      5       8       8       16      ]
[       10      6       5       8       7       5       8       9       10      9       8       11      4       5       7       14      ]

Month 6 alive 34        infected 18     dead 0
[       17      23      28      17      21      7       15      19      11      19      23      27      17      12      8       13      ]
[       10      13      16      13      15      5       10      13      6       13      13      20      10      9       4       10      ]

Month 7 alive 34        infected 20     dead 0
[       16      10      12      8       9       8       6       8       10      11      14      12      22      16      11      19      ]
[       12      8       10      6       9       8       5       5       9       8       11      10      17      13      9       18      ]

Month 8 alive 34        infected 22     dead 0
[       13      8       9       11      10      7       16      11      15      9       8       7       9       5       8       10      ]
[       12      7       7       9       10      7       13      10      13      9       8       6       8       5       8       9       ]

Month 9 alive 34        infected 24     dead 0
[       12      9       6       8       3       5       5       6       10      7       3       4       2       3       8       7       ]
[       10      8       6       6       2       4       4       6       9       6       1       4       2       3       7       6       ]

Month 10        alive 33        infected 25     dead 1
[       10      8       8       12      6       11      11      9       13      7       7       8       10      8       11      11      ]
[       9       8       8       12      6       10      9       9       13      5       6       7       10      7       10      10      ]

Month 11        alive 33        infected 27     dead 1
[       8       8       11      5       5       2       7       3       6       11      2       6       13      5       9       9       ]
[       8       8       10      5       5       2       6       3       6       10      2       6       13      4       9       8       ]

Month 12        alive 32        infected 25     dead 3
[       2       2       6       4       7       2       5       8       3       6       7       7       6       4       4       7       ]
[       2       1       6       4       6       2       5       7       3       6       7       7       6       4       4       7       ]

Month 13        alive 30        infected 23     dead 5
[       10      5       4       2       1       5       2       3       3       2       6       6       3       2       2       1       ]
[       8       5       4       2       1       5       2       2       3       2       6       4       3       2       2       1       ]

Month 14        alive 28        infected 22     dead 7
[       1       1       3       4       3       3       0       3       3       5       8       0       4       2       2       5       ]
[       0       1       3       4       3       3       0       3       3       3       6       0       4       2       2       5       ]

Month 15        alive 27        infected 22     dead 8
[       3       4       3       3       8       13      2       4       1       4       5       2       3       1       4       8       ]
[       3       4       3       3       7       12      2       4       1       4       4       2       3       1       3       7       ]

Month 16        alive 26        infected 22     dead 9
[       6       3       6       7       7       5       5       4       2       12      6       3       6       11      2       4       ]
[       6       3       6       7       7       5       5       4       2       12      6       3       6       11      2       4       ]

Month 17        alive 25        infected 22     dead 10
[       4       6       8       7       4       3       4       7       4       3       7       7       7       5       4       7       ]
[       4       6       8       7       4       3       4       7       4       3       7       7       7       5       4       7       ]

Month 18        alive 23        infected 20     dead 12
[       3       1       6       4       5       5       7       4       9       3       5       2       3       8       3       3       ]
[       3       1       6       4       5       5       7       4       9       3       5       2       3       8       3       3       ]

Month 19        alive 21        infected 18     dead 14
[       3       4       3       3       0       1       1       3       5       4       2       0       2       3       2       2       ]
[       3       4       3       3       0       1       1       3       5       4       2       0       2       3       2       2       ]

Month 20        alive 19        infected 16     dead 16
[       2       0       2       2       1       1       0       1       1       2       1       1       0       1       0       0       ]
[       2       0       2       2       1       1       0       1       1       2       1       1       0       1       0       0       ]

Month 21        alive 17        infected 15     dead 18
[       1       1       0       1       2       1       0       4       3       1       2       2       1       2       2       1       ]
[       1       1       0       1       2       1       0       4       3       1       2       2       1       2       2       1       ]

Month 22        alive 15        infected 13     dead 20
[       4       1       2       2       2       2       2       3       3       3       4       6       1       0       3       1       ]
[       4       1       2       2       2       2       2       3       3       3       4       6       1       0       3       1       ]

Month 23        alive 12        infected 10     dead 23
[       0       1       1       2       0       1       1       1       1       0       0       0       0       0       0       0       ]
[       0       1       1       2       0       1       1       1       1       0       0       0       0       0       0       0       ]

Month 24        alive 10        infected 8      dead 25
[       3       1       0       0       0       1       0       0       0       0       0       1       0       0       0       1       ]
[       3       1       0       0       0       1       0       0       0       0       0       1       0       0       0       1       ]

[Last Output] Month 24  alive 10        infected 9      dead 25
[       3       1       0       0       0       1       0       0       0       0       0       1       0       0       0       1       ]
[       3       1       0       0       0       1       0       0       0       0       0       1       0       0       0       1       ]

```