#include <iostream>
#include "World.h"
using namespace std;

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

    Position position{1, 1};
    Direction direction = Direction::West;

    bool moved = world.moveForward(position, direction);

    Percept percept = world.getPercept(position, !moved, false);

    cout << "Position: [" << position.x << "," << position.y << "]" << endl;
    cout << "Bump: " << percept.bump << endl;

    return 0;
}