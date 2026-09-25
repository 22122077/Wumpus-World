//
// Created by Mina Armijo on 9/25/26.
//

#pragma once
#include <vector>
using namespace std;

#ifndef WORLD_H
#define WORLD_H

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

public:
    World(int size);
    void printWorld() const;
    void placeGold(Position position);
};



#endif //WORLD_H
