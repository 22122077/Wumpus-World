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

struct Cell {
    bool hasPit = false;
    bool hasWumpus = false;
    bool hasGold = false;
};

class World {

    private:
        int n;
        vector<vector<Cell>> grid; //grid[x][y]
        bool isProtectedStartSquare(Position position) const;
        Position randPosition();
        mt19937 gen;

    public:
        World(int size);
        void printWorld() const;
        void placeGold();
        void placeWumpus();
};



#endif //WORLD_H
