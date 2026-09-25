#include <iostream>

using namespace std;

struct Position {
    int x;
    int y;
};
int main() {
    Position start{1, 1};

    cout << "[" << start.x << "," << start.y << "]" << endl;
    return 0;
}