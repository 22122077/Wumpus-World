//
// Created by Mina Armijo on 9/25/26.
//

#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <random>
using namespace std;

struct Position {
    int x;
    int y;
};

enum class Direction {
    North,
    East,
    South,
    West
};

enum class Action {
    Forward,
    TurnLeft,
    TurnRight,
    Grab,
    Shoot,
    Climb
};

struct ActionResult {
    bool bump = false;
    bool scream = false;
    bool gameOver = false;
};

struct Cell {
    bool hasPit = false;
    bool hasWumpus = false;
    bool hasGold = false;
};

struct Percept {
    bool stench = false;
    bool breeze = false;
    bool glitter = false;
    bool bump = false;
    bool scream = false;
};

class World {

    private:
        int n;
        vector<vector<Cell>> grid; //grid[x][y]
        bool isProtectedStartSquare(Position position) const;
        Position randPosition();
        mt19937 gen;
        bool isInsideWorld(Position position) const;
        vector<Position> getNeighbors(Position position) const;
        bool hasBreeze(Position position) const;
        bool hasStench(Position position) const;
        Position getForwardPosition(Position position, Direction direction) const;
        bool wumpusAlive = false;


    public:
        explicit World(int size);
        void printWorld() const;
        void placeGold();
        void placeWumpus();
        void placePits();
        Percept getPercept(Position position, bool bump, bool scream) const;
        bool moveForward(Position& position, Direction direction) const;
        bool grabGold(Position position);
        bool shoot(Position position, Direction direction);
        bool isDeadly(Position position) const;
        bool canClimb(Position position) const;
};



#endif //WORLD_H
