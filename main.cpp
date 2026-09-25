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

int main() {
    Position start{1, 1};
    Cell cell;

    cout << "[" << start.x << "," << start.y << "]" << endl;
    cout << cell.hasPit << endl;

    return 0;
}