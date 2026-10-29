//
// Created by Mina Armijo on 9/25/26.
//

#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <random>

// Coordinates used by Agent and World 1-based
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

// Results produced by executing action that may affect next percept/end the game
struct ActionResult {
    bool bump = false;
    bool scream = false;
    bool gameOver = false;
};

// Represents true contents of a cell in hidden world
struct Cell {
    bool hasPit = false;
    bool hasWumpus = false;
    bool hasGold = false;
};

// Info available to Agent after an action
struct Percept {
    bool stench = false;
    bool breeze = false;
    bool glitter = false;
    bool bump = false;
    bool scream = false;
};

// Stores true state of environment
// Agent does NOT directly access hidden grid/world size
class World {
private:
    int n;
    // Position coordinates are 1-based, grid indexes are 0-based
    std::vector<std::vector<Cell> > grid; //grid[x][y]
    bool isProtectedStartSquare(Position position) const;

    Position randPosition();

    std::mt19937 gen;

    bool isInsideWorld(Position position) const;

    std::vector<Position> getNeighbors(Position position) const;

    bool hasBreeze(Position position) const;

    bool hasStench(Position position) const;

    Position getForwardPosition(Position position, Direction direction) const;

    // Tracks whether Wumpus is still dangerous after being shot
    bool wumpusAlive = false;

public:
    explicit World(int size);

    // Prints hidden map for testing/observer only!
    void printWorld() const;

    void placeGold();

    void placeWumpus();

    void placePits();

    Percept getPercept(Position position, bool bump, bool scream) const;

    bool moveForward(Position &position, Direction direction) const;

    bool grabGold(Position position);

    bool shoot(Position position, Direction direction);

    bool isDeadly(Position position) const;

    bool canClimb(Position position) const;
};


#endif //WORLD_H
