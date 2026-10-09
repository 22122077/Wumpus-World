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
            else if (cell.hasWumpus && cell.hasPit) {
                cout << "WP ";
            }
            else if (cell.hasGold) {
                cout << "G  ";
            }
            else if (cell.hasWumpus) {
                cout << "W  ";
            }
            else if (cell.hasPit) {
                cout << "P  ";
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

void World::placePits() {
    bernoulli_distribution pitChance(0.2);

    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            Position position{x + 1, y + 1};

            if (isProtectedStartSquare(position) || (grid[x][y].hasGold) ){
                continue;
            }

            if (pitChance(gen)) {
                grid[x][y].hasPit = true;
            }
        }
    }
}

Position World::randPosition() {
    uniform_int_distribution<int> dist(1, n);

    Position pos;

    pos.x = dist(gen);
    pos.y = dist(gen);

    return pos;
}

bool World::isInsideWorld(Position position) const {
    return  position.x >= 1 && position.x <= n &&
            position.y >= 1 && position.y <= n;
}

vector<Position> World::getNeighbors(Position position) const {
    vector<Position> neighbors;

    Position left{position.x - 1, position.y};
    Position right{position.x + 1, position.y};
    Position top{position.x, position.y + 1};
    Position bottom{position.x, position.y - 1};

    if (isInsideWorld(left)) {
        neighbors.push_back(left);
    }
    if (isInsideWorld(right)) {
        neighbors.push_back(right);
    }
    if (isInsideWorld(top)) {
        neighbors.push_back(top);
    }
    if (isInsideWorld(bottom)) {
        neighbors.push_back(bottom);
    }

    return neighbors;
}

bool World::hasBreeze(Position position) const {
    vector<Position> neighbors = getNeighbors(position);

    for (Position neighbor : neighbors) {
        if (grid[neighbor.x - 1][neighbor.y -1].hasPit) {
            return true;
        }
    }

    return false;
}

bool World::hasStench(Position position) const {
    vector<Position> neighbors = getNeighbors(position);

    for (Position neighbor : neighbors) {
        if (grid[neighbor.x - 1][neighbor.y -1].hasWumpus) {
            return true;
        }
    }

    return false;
}

Percept World::getPercept(Position position, bool bump, bool scream) const {
    Percept percept;

    percept.stench = hasStench(position);
    percept.breeze = hasBreeze(position);
    percept.glitter = grid[position.x - 1][position.y].hasGold;
    percept.bump = bump;
    percept.scream = scream;

    return percept;
}

Position World::getForwardPosition(Position position, Direction direction) const {
    switch (direction) {
        case Direction::North:
            position.y += 1;
            break;

        case Direction::East:
            position.x += 1;
            break;
        case Direction::South:
            position.y -= 1;
            break;
        case Direction::West:
            position.x -= 1;
            break;
    }
    return position;
}

bool World::moveForward(Position& position, Direction direction) const {
    Position attemptedPosition = getForwardPosition(position, direction);

    if (!isInsideWorld(attemptedPosition)) {
        return false;
    }

    position = attemptedPosition;
    return true;
}

