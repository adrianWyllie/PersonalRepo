# Personal Ropsitory
This contains an example selection of programs I've written and worked on.

### solve.cpp
This program was written as part of an intro to artificial intelligence class at the University of Kentucky. It was the follow-up to another program that generated a virtual model of a megaminx puzzle, a more complex version of the rubiks' cube, which could also generated a scrambled configuration. This program can generate a scrambled configuration based on the number of moves entered as a command line arguement when running the program, and it will also solve it using the A* algorithm.
Please note that this program is memory hungry, and may take a while to run with more scrambled configurations, so I don't recommend running it with more than 10 steps in the scramble process.

### mazepath.pl
This program was written as part of an intro to artificial intelligence class at the University of Kentucky. It is a maze solver developed in Prolog, which is a declarative language. The algorithm I'm using to solve these mazes has automatic loop detection, and will return false if no path is possible from the start to end point.
