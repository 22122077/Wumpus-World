//
// Created by Mina Armijo on 9/25/26.
//

#include "World.h"
#include <iostream>

World::World(int size)
    : n(size),
        grid(size, vector<Cell>(size)) {

}

void World::printWorld() const {
    for (int y = n - 1; y>= 0; y--) {
        for (int x = 0; x < n; x++) {

            const Cell& cell = grid[x][y];

            if (cell.hasGold) {
                cout << "G ";
            }
            else {
                cout << ". ";
            }
        }

        cout << endl;
    }
}

void World::placeGold(Position position) {
    grid[position.x - 1][position.y - 1].hasGold = true;
}
