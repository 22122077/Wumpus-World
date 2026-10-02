#include <iostream>
#include "World.h"
using namespace std;

enum class Direction {
    North,
    East,
    South,
    West
};

int main() {
    Position start{1, 1};
    Cell cell;
    Direction startDirection = Direction::East;

    int n;
    cout << "Enter world size: ";
    cin >> n;

    World world(n);

    world.placeWumpus();
    world.placeGold();
    world.placePits();
    world.printWorld();

    return 0;
}