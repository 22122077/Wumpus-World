#include <iostream>

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

    cout << "[" << start.x << "," << start.y << "]" << endl;
    cout << cell.hasPit << endl;

    return 0;
}