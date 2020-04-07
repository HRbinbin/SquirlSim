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
mpicc -cc=icc build/squirl.o build/ran2.o build/test.o build/pool.o -o bin/run -lm
```

The auto-make script generates a directory `build` with the object files. The executable file is in `bin`

Use `mpirun` to run the code. We should assign **218** processes since there are **16 land actors, 
up to 200 squirrel actors, a controller actor and a master actor**. We can change the parameters in `main.h`. 

After running `mpirun -n 218 bin/run`, we can see the simulation output of every month.

```$xslt
$ mpirun -n 218 bin/run
Month 1 alive 40        infected 5      dead 6
POP     [       263     262     294     269     303     291     267     266     258     323     275     316     319     302     271     288     ]
INF     [       28      31      24      24      20      31      34      29      26      40      31      43      37      27      40      30      ]

Month 2 alive 41        infected 9      dead 9
POP     [       96      78      115     91      86      87      102     73      103     94      70      100     84      85      94      105     ]
INF     [       23      8       28      15      13      17      24      10      16      16      11      20      21      20      20      23      ]

Month 3 alive 40        infected 7      dead 21
POP     [       233     224     192     226     227     195     193     196     220     233     198     228     214     236     209     232     ]
INF     [       37      37      28      49      33      28      26      37      45      36      41      42      41      36      33      39      ]

Month 4 alive 46        infected 5      dead 32
POP     [       245     243     220     234     244     242     241     226     249     208     226     228     263     213     233     244     ]
INF     [       46      32      35      44      39      39      32      36      34      27      38      38      40      29      32      35      ]

Month 5 alive 54        infected 11     dead 40
POP     [       222     226     231     223     227     246     237     218     217     213     209     224     210     208     194     184     ]
INF     [       43      41      32      53      41      44      39      47      48      45      41      35      39      40      41      45      ]

Month 6 alive 56        infected 11     dead 52
POP     [       221     254     227     209     256     237     220     237     218     205     241     240     232     234     225     242     ]
INF     [       35      36      37      30      48      40      27      46      31      35      32      37      41      43      31      26      ]

Month 7 alive 55        infected 11     dead 65
POP     [       266     287     285     239     295     268     275     275     283     291     309     300     252     272     287     256     ]
INF     [       53      45      40      43      60      50      54      47      53      56      61      39      38      56      45      52      ]

Month 8 alive 54        infected 17     dead 75
POP     [       130     170     163     153     143     142     133     166     148     154     150     146     152     163     144     166     ]
INF     [       31      68      52      43      38      43      39      38      53      41      37      39      42      44      46      47      ]

Month 9 alive 45        infected 8      dead 94
POP     [       221     218     199     210     211     203     217     181     199     224     204     184     209     230     213     231     ]
INF     [       66      47      51      51      50      54      46      43      48      47      55      43      64      53      51      54      ]

Month 10        alive 44        infected 6      dead 107
POP     [       199     220     228     202     233     225     206     204     214     223     220     217     223     216     199     206     ]
INF     [       39      40      46      33      51      50      34      41      38      45      43      36      49      36      47      35      ]

Month 11        alive 49        infected 10     dead 117
POP     [       223     224     212     206     218     187     211     205     203     214     240     200     200     247     202     240     ]
INF     [       46      48      45      29      33      25      39      54      35      40      47      42      44      48      36      42      ]

Month 12        alive 54        infected 10     dead 127
POP     [       186     208     200     205     199     225     195     172     215     180     204     190     182     203     202     200     ]
INF     [       38      47      43      33      32      52      34      44      40      36      32      40      37      49      43      34      ]

Month 13        alive 55        infected 13     dead 139
POP     [       205     211     232     228     263     228     231     194     207     226     239     229     217     225     213     212     ]
INF     [       42      32      42      44      54      50      42      42      44      55      52      53      45      36      45      46      ]

Month 14        alive 53        infected 12     dead 153
POP     [       203     183     200     214     189     206     190     191     183     202     204     204     182     203     200     180     ]
INF     [       59      40      56      61      31      48      53      55      57      50      53      57      44      50      50      51      ]

Month 15        alive 51        infected 18     dead 163
POP     [       188     182     175     208     181     160     184     183     177     172     156     179     151     164     174     169     ]
INF     [       65      60      55      69      66      53      67      43      62      52      47      68      58      52      47      58      ]

Month 16        alive 44        infected 12     dead 178
POP     [       122     137     101     111     120     115     115     110     93      114     96      117     129     125     111     116     ]
INF     [       34      29      26      29      30      32      27      29      24      36      26      32      34      31      21      31      ]

Month 17        alive 33        infected 6      dead 192
POP     [       191     162     198     184     205     162     187     187     184     175     185     183     196     177     183     171     ]
INF     [       38      29      36      27      28      29      27      38      40      34      33      37      36      37      28      31      ]

Month 18        alive 29        infected 7      dead 200
POP     [       121     125     132     118     120     128     129     103     107     123     133     119     116     141     114     133     ]
INF     [       35      29      39      36      31      39      45      21      32      31      42      35      30      34      34      40      ]

Month 19        alive 25        infected 2      dead 209
POP     [       106     119     112     124     91      120     89      109     98      116     101     112     106     122     89      109     ]
INF     [       21      29      25      26      20      26      17      19      17      21      19      18      18      21      18      14      ]

Month 20        alive 30        infected 5      dead 215
POP     [       302     348     341     346     344     329     282     335     383     339     309     350     331     358     335     329     ]
INF     [       27      20      28      35      30      31      24      34      17      24      30      28      15      29      26      23      ]

Month 21        alive 28        infected 13     dead 223
POP     [       262     263     257     241     265     255     259     297     302     283     284     287     268     247     278     282     ]
INF     [       35      42      48      37      48      37      42      54      51      48      57      51      51      50      52      51      ]

Month 22        alive 20        infected 9      dead 238
POP     [       119     90      110     103     100     108     112     111     111     97      99      108     102     108     117     120     ]
INF     [       41      39      37      40      33      48      44      32      34      38      36      40      42      46      38      45      ]

Month 23        alive 14        infected 1      dead 253
POP     [       244     217     218     163     220     213     228     212     229     212     219     202     229     203     196     217     ]
INF     [       36      39      38      32      39      27      48      30      28      40      44      30      42      37      36      36      ]

Month 24        alive 15        infected 6      dead 262
POP     [       162     177     195     187     171     181     197     201     173     187     220     189     197     189     181     167     ]
INF     [       35      27      46      40      34      35      44      52      40      34      47      51      49      40      46      36      ]

[Last output] Month 24  alive 15        infected 6      dead 262
POP     [       1       4       1       2       3       2       0       2       0       0       3       0       0       0       0       1       ]
INF     [       0       2       1       1       1       0       0       2       0       0       2       0       0       0       0       1       ]
```