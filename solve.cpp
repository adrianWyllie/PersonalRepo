#include <vector>
#include <iostream>
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>

/*  CS 463
    Project 2 - A* on the Megaminx
*/
//Deallocation on program completion takes its time.
using namespace std;

//Colour, Rotation, edge, corner, and side were copied from my code for the megaminx. No changes were made
enum Colour { white, blue, yellow, purple, green, red, grey, lblue, lyellow, pink, lgreen, orange};
enum Rotation { clockwise, counterclockwise};

struct edge {
	int eid[2];
	int nextTo[2];
};

struct corner {
	int cid[3];
	int nextTo[3];
};

struct side {
	int colour;
	struct side * nextTo[5];
	struct edge edges[5];
	struct corner corners[5];
};

//This struct acts as my node for A*.
struct config {
	uint8_t h; //Component of node's f value, representing depth
	uint8_t g; //COmponent of node's f value, representing the estimated distance to solved
	vector<uint8_t> moves; //this stores the moves used to generate the node's configuration
	bool operator<(const struct config &o) const { //This overrides the < operator for priority_queue
		return (float)1/(h+g) < (float)1/(o.h+o.g); //priority_queue is backwards. 1/f fixes that.
		//There's technically a bug when g + h = 0, but that won't happen. Period.
	}
};
// These distance tables are a component of my heuristic.
const int edgeDistTable[12][12] = {{0,1,1,1,1,1,4,2,2,2,2,2},{1,0,1,2,2,1,2,4,2,1,1,2},{1,1,0,1,2,2,2,2,4,2,1,1},
                {1,2,1,0,1,2,2,1,2,4,2,1},{1,2,2,1,0,1,2,1,1,2,4,2},{1,1,2,2,1,0,2,2,1,1,2,4},
                {4,2,2,2,2,2,0,1,1,1,1,1},{2,4,2,1,1,2,1,0,1,2,2,1},{2,2,4,2,1,1,1,1,0,1,2,2},
                {2,1,2,4,2,1,1,2,1,0,1,2},{2,1,1,2,4,2,1,2,2,1,0,1},{2,2,1,1,2,4,1,1,2,2,1,0}};
const int cornerDistTable[12][12] = {{0,1,1,1,1,1,3,2,2,2,2,2},{1,0,1,2,2,1,2,3,2,1,1,2},{1,1,0,1,2,2,2,2,3,2,1,1},
                {1,2,1,0,1,2,2,1,2,3,2,1},{1,2,2,1,0,1,2,1,1,2,3,2},{1,1,2,2,1,0,2,2,1,1,2,3},
                {3,2,2,2,2,2,0,1,1,1,1,1},{2,3,2,1,1,2,1,0,1,2,2,1},{2,2,3,2,1,1,1,1,0,1,2,2},
                {2,1,2,3,2,1,1,2,1,0,1,2},{2,1,1,2,3,2,1,2,2,1,0,1},{2,2,1,1,2,3,1,1,2,2,1,0}};
static struct side * sides; //This is used to copy iSides and move to a node's configuration to find children
static struct side * iSides; //This is scrambled and kept as a scrambled configuration of the puzzle
static priority_queue<struct config> q; //This priority_queue contains the search frontier

struct config input();
void rotateC(struct side *rSide);
void initializePuzzle();
void scramble(int numMoves);

void colour(int col);
void printMoves(struct config node);
void copySides();
void moveSeq(vector<uint8_t> moves);
void rotate(int rSide, int dir);
void rotateCC(struct side *rSide);
void findChildren(struct config node);
void aStar(int max);
void initDistTables();
bool isSolved();
int heuristic();
int dist();

int mod (int a, int b) { //This acts as a better modulus function than %. It helps.
	return ((b+(a%b))%b);
}

int main(int argc, char** argv) { //Hey it's my main function!
	sides = (struct side *)malloc(sizeof(struct side) * 12); //Initialization is nice.
	iSides = (struct side *)malloc(sizeof(struct side) * 12); //Doubly so.
	int numMoves = 0; //Remember when I said g+h=0 wouldn't happen? It only happens when numMoves = 0.
	if (argc == 1) {
		numMoves = 0;
	}
	else if (argc == 2) {
		numMoves = atoi(argv[1]);
	}
	else {
		return 1;
	}
	initializePuzzle(); //The good initialization.
	if (numMoves > 0) { //I don't want numMoves to be 0.
		scramble(numMoves); //Now with 100% less butterknives. Not that there were any.
		auto sTime = chrono::high_resolution_clock::now(); //I keep time.
		aStar(numMoves); //Here's the part this project is about.
		auto eTime = chrono::high_resolution_clock::now(); //I'm still keeping time.
		auto duration = chrono::duration_cast<chrono::milliseconds>(eTime - sTime);
		cout << "Time: " << duration.count() << " milliseconds" << endl;
	}
	//Welcome to TemporaryLand < I wrote this comment when I first started this. I'm leaving it in. Why not?
	return 1;
}

bool isSolved(int h) { //The easiest function. Iff the puzzle is solved, h is 0. Ergo if h is 0, the puzzle is solved.
	return !h;
}
int heuristic() { //This heuristic function uses the distance tables from above. It's not the greatest, but it works
	int dists = dist(); //Get that distance.
	int h = dists/15 + (dists%15 != 0); //Here's a bit of code for implementing an efficient ceiling division
	return h;
}

int dist() { //This is basically manhattan distance using the tables up above.
	int sumEdgeDists = 0; //This code checks each edge twice,
	int sumCornerDists = 0; // and each corner 3 times, because of how I set up my data structure
	int sumDists = 0; //So a little extra math is needed here.
	for (int i = 0; i < 12; i += 1) {
		for (int j = 0; j < 5; j += 1) { //This block finds the distance between each sticker and where it
				//wants to be. It's the part where we use the tables.
			sumEdgeDists += edgeDistTable[sides[i].edges[j].eid[0]][sides[i].edges[j].nextTo[0]];
			sumEdgeDists += edgeDistTable[sides[i].edges[j].eid[1]][sides[i].edges[j].nextTo[1]];
			sumCornerDists += cornerDistTable[sides[i].corners[j].cid[0]][sides[i].corners[j].nextTo[0]];
			sumCornerDists += cornerDistTable[sides[i].corners[j].cid[1]][sides[i].corners[j].nextTo[1]];
			sumCornerDists += cornerDistTable[sides[i].corners[j].cid[2]][sides[i].corners[j].nextTo[2]];
		}
	}
	//I mentioned needing a little more math here. I'm taking the ceiling of the two sums divided by the number of times
	//I access each type of sticker and adding them together.
	sumDists = ((sumEdgeDists/2 + (sumEdgeDists%2 != 0)) + (sumCornerDists/3 + (sumCornerDists%3 != 0)));
	return sumDists;
}

struct config input() {
	copySides(); //make sides look like iSides.
	//Now we generate the initial config
	struct config initial;
	initial.h = (uint8_t)heuristic();
	initial.g = 0;
	return initial;
}

void aStar(int max) {
	struct config node; //Here, have a node. It's like a cookie, but it's not.
	int expanded = 0; //An iterator
	bool solved = false; //For when the puzzle is solved
	q.push(input()); //Put the first node in the queue
	while (!solved) {
		node = q.top(); //removes the top node from the stack
		q.pop();
		if (node.g <= max) { //The optimal solution should have a g less than max, but I have this here anyways
			expanded += 1; //It's a counter, not an iterator!
			solved = isSolved(node.h); //checks to see if the node is solved
			if (solved) {
				cout << "Expanded: " << expanded << endl;
				cout << "Size of Frontier: " << q.size() << endl;
				cout << "Size of Solution: " << (int)node.g << endl;
				printMoves(node);
				break;
			}
			else {
				findChildren(node); //finds children and adds them to the queue.
			}				// ^ is really weird outside context.
		}
	}
	return;
}

void findChildren(struct config node) {
	moveSeq(node.moves); //Get a puzzle to match the state of the node
	int last = 25;
	if (!node.moves.empty()) {
		unsigned char back = node.moves[node.moves.size()-1];
		last = mod(back+12, 24);
	}
	for (int i = 0; i < 24; i += 1) { //There are 24 possible moves. Cycle through them all.
		if (i != last) {
			int dir;
			int side;
			if (i >= 12) {
				dir = 1;
				side = i - 12;
			}
			else {
				dir = 0;
				side = i;
			}
			struct config child; // create a child node
			for (vector<uint8_t>::iterator j = node.moves.begin(); j != node.moves.end(); j += 1) {
				child.moves.push_back(*j); //copy the parent's moves into the child
			}
			child.moves.push_back((uint8_t)i); //add the current move to the child
			child.g = node.g;
			child.g += 1; //set the child's g to 1 + the parent's g
			rotate(side, dir); //move from the parent state to the child's state
			child.h = (uint8_t)heuristic(); //calculate the h value
			rotate(side, (dir == 0)); //the (dir == 0) basically returns the other direction.
				//Gets back to the parent state
			q.push(child); //add the child node to the queue
		}
	}
	return;
}

void moveSeq(vector<uint8_t> moves) { //this does kinda what it says it does
	//It rotates the puzzle a sequence of moves from the initial scrambled state
	copySides(); //copies iSides onto sides
	for (vector<uint8_t>::iterator i = moves.begin(); i != moves.end(); i += 1) {
		int dir;
		int rSide;
		if (*i < 24) {
			if (*i >= 12) {
				dir = 1;
				rSide = *i - 12;
			}
			else {
				dir = 0;
				rSide = *i;
			}
			rotate(rSide, dir);
		}
	}
}

void rotate(int rSide, int dir) { //A rotate function. Sorta unnecessary but it's here to stay.
	if (dir == 1) {
		rotateCC(&sides[rSide]);
	}
	else {
		rotateC(&sides[rSide]);
	}
}

void rotateC(struct side *rSide) { //The lazy approach to clockwise.
	for (int i = 0; i < 4; i += 1) {
		rotateCC(rSide);
	}
	return;
}

void rotateCC(struct side * rSide) { //The not lazy approach to counterclockwise
	struct side *tempSide = rSide->nextTo[0];
	for (int i = 0; i < 4; i += 1) { //Update the adjacent sides to reflect their new positions
		rSide->nextTo[i] = rSide->nextTo[mod(i+1,5)];
	}
	rSide->nextTo[4] = tempSide;

	for (int i = 0; i < 5; i += 1) {//update the edges to reflect their new positions, while preserving correct orientation
	        if (rSide->edges[i].nextTo[0] == rSide->colour)
                        rSide->edges[i].nextTo[1] = rSide->nextTo[i]->colour;
                else if (rSide->edges[i].nextTo[1] == rSide->colour)
                        rSide->edges[i].nextTo[0] = rSide->nextTo[i]->colour;
        }
        for (int i = 0; i < 5; i += 1) {//update the corners to reflect their new positions, while preserving correct orientation
                if (rSide->corners[i].nextTo[0] == rSide->colour) {
                        rSide->corners[i].nextTo[1] = rSide->nextTo[i]->colour;
                        rSide->corners[i].nextTo[2] = rSide->nextTo[mod(i+1,5)]->colour;
                }
                else if (rSide->corners[i].nextTo[1] == rSide->colour) {
                        rSide->corners[i].nextTo[2] = rSide->nextTo[i]->colour;
                        rSide->corners[i].nextTo[0] = rSide->nextTo[mod(i+1,5)]->colour;
                }
                else if (rSide->corners[i].nextTo[2] == rSide->colour) {
                        rSide->corners[i].nextTo[0] = rSide->nextTo[i]->colour;
                        rSide->corners[i].nextTo[1] = rSide->nextTo[mod(i+1,5)]->colour;
                }
        }
        for (int i = 0; i < 5; i += 1) { //move the edges to their new positions
                for (int j = 0; j < 5; j += 1) {
                        if (rSide->nextTo[i]->edges[j].eid[0] == rSide->edges[mod(i+1,5)].eid[0]
            && rSide->nextTo[i]->edges[j].eid[1] == rSide->edges[mod(i+1,5)].eid[1])
                                rSide->nextTo[i]->edges[j] = rSide->edges[i];
        }
        for (int j= 0; j < 5; j += 1) { //move the corners to their new positions
            if (rSide->nextTo[i]->corners[j].cid[0] == rSide->corners[mod(i+1,5)].cid[0]
                        && rSide->nextTo[i]->corners[j].cid[1] == rSide->corners[mod(i+1,5)].cid[1]
             		&& rSide->nextTo[i]->corners[j].cid[2] == rSide->corners[mod(i+1,5)].cid[2]) {
                        	if (rSide->nextTo[i]->corners[mod(j+1,5)].cid[0] == rSide->corners[i].cid[0]
               	    && rSide->nextTo[i]->corners[mod(j+1,5)].cid[1] == rSide->corners[i].cid[1]
                    && rSide->nextTo[i]->corners[mod(j+1,5)].cid[2] == rSide->corners[i].cid[2]) {
                    rSide->nextTo[i]->corners[j] = rSide->corners[i];
                    rSide->nextTo[i]->corners[mod(j+1,5)] = rSide->corners[mod(i-1,5)];
                    break;
                }
                        }
                }
        }
        return;
}

void initializePuzzle() {
    //I really didn't like debugging this the first time around.
    //   assigns each side a number and corresponding colour
        for (int i = 0; i < 12; i += 1) {
                iSides[i].colour = i;
        }

    //   this is a list of all side neighbors for each side in counterclockwise order
        int tempNextTo[12][5] = {{ 1, 2, 3, 4, 5}, { 0, 5, 9, 10, 2}, { 0, 1, 10, 11, 3},
        { 0, 2, 11, 7, 4}, { 0, 3, 7, 8, 5}, { 0, 4, 8, 9, 1},
        { 7, 11, 10, 9, 8}, { 6, 8, 4, 3, 11}, { 6, 9, 5, 4, 7},
        { 6, 10, 1, 5, 8}, { 6, 11, 2, 1, 9}, { 6, 7, 3, 2, 10}};
        for (int i = 0; i < 12; i += 1) {
                for (int j = 0; j < 5; j += 1) {
                        iSides[i].nextTo[j] = &iSides[tempNextTo[i][j]];
                }
        }

    //   this is a list of all of the edges
    //   the edges are in ascending order with the smaller number of each edge listed first
        int le[30][2] = {{0,1},{0,2},{0,3},{0,4},{0,5},{1,2},{1,5},{1,9},{1,10},{2,3},
        {2,10},{2,11},{3,4},{3,7},{3,11},{4,5},{4,7},{4,8},{5,8},{5,9},
        {6,7},{6,8},{6,9},{6,10},{6,11},{7,8},{7,11},{8,9},{9,10},{10,11}};

    //   This is a list of edges for each side in counterclockwise order
        int *tempEdges[12][5] = {{le[0],le[1],le[2],le[3],le[4]},{le[0],le[6],le[7],le[8],le[5]},
        {le[1],le[5],le[10],le[11],le[9]},{le[2],le[9],le[14],le[13],le[12]},
        {le[3],le[12],le[16],le[17],le[15]},{le[4],le[15],le[18],le[19],le[6]},
        {le[20],le[24],le[23],le[22],le[21]},{le[20],le[25],le[16],le[13],le[26]},
        {le[21],le[27],le[18],le[17],le[25]},{le[22],le[28],le[7],le[19],le[27]},
        {le[23],le[29],le[10],le[8],le[28]},{le[24],le[26],le[14],le[11],le[29]}};
        for (int i = 0; i < 12; i += 1) {
                for (int j = 0; j < 5; j += 1) {
                        iSides[i].edges[j].eid[0] = tempEdges[i][j][0];
                        iSides[i].edges[j].eid[1] = tempEdges[i][j][1];
                        iSides[i].edges[j].nextTo[0] = tempEdges[i][j][0];
                        iSides[i].edges[j].nextTo[1] = tempEdges[i][j][1];
                }
        }

    //   these are all the corners
    //   each corner is listed with it's smallest number face first, then in counterclockwise order
    //   the list is in acsending order
        int lc[20][3] = {{0,1,2},{0,2,3},{0,3,4},{0,4,5},{0,5,1},{1,5,9},{1,9,10},{1,10,2},{2,10,11},
        {2,11,3},{3,7,4},{3,11,7},{4,7,8},{4,8,5},{5,8,9},{6,7,11},{6,8,7},{6,9,8},{6,10,9},{6,11,10}};

   //   this lists the corners for each side in counterclockwise order
        int *tempCorners[12][5] = {{lc[0],lc[1],lc[2],lc[3],lc[4]},{lc[4],lc[5],lc[6],lc[7],lc[0]},
        {lc[0],lc[7],lc[8],lc[9],lc[1]},{lc[1],lc[9],lc[11],lc[10],lc[2]},
        {lc[2],lc[10],lc[12],lc[13],lc[3]},{lc[3],lc[13],lc[14],lc[5],lc[4]},
        {lc[15],lc[19],lc[18],lc[17],lc[16]},{lc[16],lc[12],lc[10],lc[11],lc[15]},
        {lc[17],lc[14],lc[13],lc[12],lc[16]},{lc[18],lc[6],lc[5],lc[14],lc[17]},
        {lc[19],lc[8],lc[7],lc[6],lc[18]},{lc[15],lc[11],lc[9],lc[8],lc[19]}};
        for (int i = 0; i < 12; i += 1) {
                for (int j = 0; j < 5; j += 1) {
                        iSides[i].corners[j].cid[0] = tempCorners[i][j][0];
                        iSides[i].corners[j].cid[1] = tempCorners[i][j][1];
                        iSides[i].corners[j].cid[2] = tempCorners[i][j][2];
                        iSides[i].corners[j].nextTo[0] = tempCorners[i][j][0];
                        iSides[i].corners[j].nextTo[1] = tempCorners[i][j][1];
                        iSides[i].corners[j].nextTo[2] = tempCorners[i][j][2];
                }
        }
        return;
}
    //   Hey look! It's a mess of colours. That's what this does.
void scramble(int numMoves) {
        srand(time(NULL));
    //   lists of moves for move verification
        int ls[numMoves];
        int lr[numMoves];
        int i = 0; // I HAZ ITERATOR
        while (i < numMoves) {

    //   Get a random move by picking a side and rotation randomly
                ls[i] = rand() % 12;
                lr[i] = rand() % 2;

    //   Here's the part where we get rid of invalid moves that undo previous moves
    //   by setting check to 1, i will not increment, so invalid moves do not count towards the total
                int check = 0;
    //   This finds the last move that touched this side. If that move was a move on an adjacent side
    //   or there was no such prior move, the move is valid. If the last move was on the same side in
    //   the opposite direction, the current move is invalid
                for (int j = i-1; j >= 0; j -= 1) {
                        if (iSides[ls[j]].colour == iSides[ls[i]].nextTo[0]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[1]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[2]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[3]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[4]->colour)
                                break;
                        if (ls[i] == ls[j] && lr[i] != lr[j]) {
                                check = 1;
                                break;
                        }
                }
    //   this next code checks if the same side has moved in the same direction five times
    //   without an adjacent side moving. If this move makes it so it has, this move is invalid
    //   On a side note, this effectively locks the side from being rotated until an adjacent side
    //   is rotated.
                int check5 = 1;
                for (int j = i-1; j > 0; j -= 1) {
                        if (check == 1) {
                                break;
                        }
                        if (iSides[ls[j]].colour == iSides[ls[i]].nextTo[0]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[1]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[2]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[3]->colour
                        || iSides[ls[j]].colour == iSides[ls[i]].nextTo[4]->colour) {
                                check5 = 1;
                                break;
                        }
                        if (ls[i] == ls[j] && lr[i] == lr[j]) {
                                check5 += 1;
                        }
                        if (check5 == 5) {
                                check = 1;
                                break;
                        }
                }
                if (check == 0) {
                        if (lr[i] == clockwise) {
                                rotateC(&iSides[ls[i]]);
                                //cout << "Rotate "; CPr(sides[ls[i]].colour); cout << " clockwise" << endl;
                        }
                        else {
                                rotateCC(&iSides[ls[i]]);
                                //cout << "Rotate "; CPr(sides[ls[i]].colour); cout << " counterclockwise" << endl;
                        }
                        i++;
                }
        }
        return;
}

void copySides() { //Here's that copy function. I had to write this because pointers.
	for (int i = 0; i < 12; i += 1) {
		sides[i].colour = iSides[i].colour;
		for (int j = 0; j < 5; j += 1) {
			sides[i].edges[j].eid[0] = iSides[i].edges[j].eid[0];
			sides[i].edges[j].eid[1] = iSides[i].edges[j].eid[1];
			sides[i].edges[j].nextTo[0] = iSides[i].edges[j].nextTo[0];
			sides[i].edges[j].nextTo[1] = iSides[i].edges[j].nextTo[1];
			sides[i].corners[j].cid[0] = iSides[i].corners[j].cid[0];
			sides[i].corners[j].cid[1] = iSides[i].corners[j].cid[1];
			sides[i].corners[j].cid[2] = iSides[i].corners[j].cid[2];
			sides[i].corners[j].nextTo[0] = iSides[i].corners[j].nextTo[0];
			sides[i].corners[j].nextTo[1] = iSides[i].corners[j].nextTo[1];
			sides[i].corners[j].nextTo[2] = iSides[i].corners[j].nextTo[2];
			sides[i].nextTo[j] = &sides[iSides[i].nextTo[j]->colour];
		}
	}
}

void printMoves(struct config node) {
	for (vector<uint8_t>::iterator i = node.moves.begin(); i != node.moves.end(); i += 1) {
		if (*i < 24) {
			if (*i >= 12) {
				cout << "rotate "; colour((int)*i-12); cout << " Counterclockwise" << endl;
			}
			else {
				cout << "rotate "; colour((int)*i); cout << " Clockwise" << endl;
			}
		}
	}
}

void colour(int col) {
	switch (col) {
		case white: cout << "White"; break;
		case blue: cout << "Blue"; break;
		case yellow: cout << "Yellow"; break;
		case purple: cout << "Purple"; break;
		case green: cout << "Green"; break;
		case red: cout << "Red"; break;
		case grey: cout << "Grey"; break;
		case lblue: cout << "Light Blue"; break;
		case lyellow: cout << "Light Yellow"; break;
		case pink: cout << "Pink"; break;
		case lgreen: cout << "Light Green"; break;
		case orange: cout << "Orange"; break;
	}
}
