//
// Created by Mina Armijo on 9/25/26.
//

#include "World.h"
#include <iostream>

World::World(int size)
    : n(size),
        grid(size, vector<Cell>(size)),
        gen(random_device{}()) {

}

void World::printWorld() const {
    for (int y = n - 1; y>= 0; y--) {
        for (int x = 0; x < n; x++) {

            const Cell& cell = grid[x][y];

            if (cell.hasGold && cell.hasWumpus) {
                cout << "GW ";
            }
            else if (cell.hasGold) {
                cout << "G  ";
            }
            else if (cell.hasWumpus) {
                cout << "W  ";
            }
            else {
                cout << ".  ";
            }
        }

        cout << endl;
    }
}

void World::placeGold() {
    Position position;

    do {
        position = randPosition();
    }
    while (position.x == 1 && position.y == 1);


    grid[position.x - 1][position.y - 1].hasGold = true;
}

bool World::isProtectedStartSquare(Position position) const {
    return  (position.x == 1 && position.y == 1) ||
            (position.x == 2 && position.y == 1) ||
            (position.x == 1 && position.y == 2);
}

void World::placeWumpus() {
    Position position;

    do {
        position = randPosition();
    }
    while (isProtectedStartSquare(position));

    grid[position.x - 1][position.y - 1].hasWumpus = true;

}

Position World::randPosition() {
    uniform_int_distribution<int> dist(1, n);

    Position pos;

    pos.x = dist(gen);
    pos.y = dist(gen);

    return pos;
}