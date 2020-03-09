# Squirrels disease simulation

This is a simulation of British red squirrels infecting parapoxvirus. 

The infrastructure is **Master actor + land actor + N squirrel actors**

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
