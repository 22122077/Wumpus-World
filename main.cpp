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

    world.placeGold({2,2});
    world.printWorld();

    return 0;
}