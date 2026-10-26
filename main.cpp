#include <iostream>
#include "World.h"
#include "Agent.h"

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

    Agent agent(n);

    Percept initialPercept = world.getPercept(
        agent.getPosition(),
        false,
        false
    );

    InferenceResult result = agent.processPercept(initialPercept);

    Proposition safeEast{
        PropositionType::Safe,
        {2, 1},
        false
    };

    cout << "Agent knows [2,1] is safe: "
         << agent.knows(safeEast)
         << endl;

    Proposition safeNorth{
        PropositionType::Safe,
        {1, 2},
        false
    };

    cout << "Agent knows [1,2] is safe: "
         << agent.knows(safeNorth)
         << endl;

    return 0;
}