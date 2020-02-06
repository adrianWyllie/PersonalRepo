%this is a maze solver in prolog!
  
offByOne(X,XA):- %This is true when X and XA are off by one.
    (N is X+1;
    N is X-1),
    XA = N.

isValid([X,Y],Maze):- %This is true when the given coordinates are in Maze.
    length(Maze,SizeX),
    X < SizeX,
    X >= 0,
    nth0(X,Maze,Row),
    length(Row,SizeY),
    Y < SizeY,
    Y >= 0.

isRoom([X,Y],Maze):- %This is true when the given spot isn't a wall.
    isValid([X,Y],Maze),
    nth0(X,Maze,Row),
    nth0(Y,Row,Elem),
    Elem = 0.

connected([X1,Y1],[X2,Y2],Maze):- %This is true when the given spots are adjacent
    isRoom([X1,Y1],Maze),
    isRoom([X2,Y2],Maze),
    (offByOne(X1,X2),Y1=Y2);
    (offByOne(Y1,Y2),X1=X2).

neighbor([X,Y],[XN,YN],Maze):- %This is true for a room having at least one neighbor. This is designed for getting a neighbor of a room, rather than checking if two rooms are adjacent.
    (XN is X+1,isRoom([XN,Y],Maze),YN is Y);
    (XN is X-1,isRoom([XN,Y],Maze),YN is Y);
    (YN is Y+1,isRoom([X,YN],Maze),XN is X);
    (YN is Y-1,isRoom([X,YN],Maze),XN is X).

path(A,B,Maze,Path):- %This is true when Path goes from A to B in the maze. This is here because travel creates Q going from B to A
    travel(A,B,[A],Maze,Q),
    reverse(Q,Path).

travel([X1,Y1],[X2,Y2],P,Maze,[[X2,Y2]|P]):- %Base case for tavel.
    connected([X1,Y1],[X2,Y2],Maze).
travel([X1,Y1],[X2,Y2],Visited,Maze,Path):- %This travels the Maze, and isn't true when it hits a dead end. It will not finish until a path is found
    [XN,YN] \== [X2,Y2],
    neighbor([X1,Y1],[XN,YN],Maze),
    not(member([XN,YN],Visited)),
    travel([XN,YN],[X2,Y2],[[XN,YN]|Visited],Maze,Path).

mazepath(X,Y,X,Y,Maze,0). %Base case for mazepath. If you are going from the current location to the current location, there is no need to drive around the block.
mazepath(X1,Y1,X2,Y2,Maze,P):- %This prints the Path, the Maze, and the length of the path. This also includes different test mazes, which are commented out.
    %Maze = [[0, 0, 0, 0],[0, 1, 1, 0],[0, 1, 0, 0],[0, 1, 0, 1],[0, 0, 0, 0]],
    Maze = [[0,0,1],[0,1,0],[1,0,0]],
    %Maze = [[0,1,0,0,0],[0,1,0,1,0],[0,0,0,1,1],[0,1,0,1,0],[0,1,0,0,0]],
    %Maze = [0,1,1,0,0,0,1,0],[0,1,0,0,1,0,1,0],[0,0,0,1,1,0,0,0],[0,1,0,1,0,1,1,0],[0,0,0,1,0,0,1,0],[0,1,1,0,1,0,0,0],[0,0,1,0,1,1,0,1],[1,0,0,0,0,0,0,0]],
	isRoom([X1,Y1],Maze),
	isRoom([X2,Y2],Maze),
	path([X1,Y1],[X2,Y2],Maze,Path),
	write(Path),
	length(Path,L),
	P is L-1.
