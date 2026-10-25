#include <iostream>
#include "World.h"
using namespace std;

int main() {
    int n;
    do {
        cout << "Enter world size: ";
        cin >> n;
    }
    while (n < 2);

    World world(n);

    world.placeWumpus();
    world.placeGold();
    world.placePits();
    world.printWorld();

    return 0;
}