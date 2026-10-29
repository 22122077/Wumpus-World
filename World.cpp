//
// Created by Mina Armijo on 9/25/26.
//

#include "World.h"
#include <iostream>


// ==================== WORLD SETUP ====================


// The World stores real environment state, including the full map size
// and hidden cell contents. This info is NOT exposed directly to Agent.
World::World(int size)
    : n(size),
      grid(size, std::vector<Cell>(size)),
      gen(std::random_device{}()) {
}

// Prints the complete hidden map for observer/testing purposes.
// This does NOT represent info available to the Agent.
void World::printWorld() const {
    for (int y = n - 1; y >= 0; y--) {
        for (int x = 0; x < n; x++) {
            const Cell &cell = grid[x][y];

            // Combined symbols are shown where multiple objects occupy a cell
            if (cell.hasGold && cell.hasWumpus) {
                std::cout << "GW ";
            } else if (cell.hasWumpus && cell.hasPit) {
                std::cout << "WP ";
            } else if (cell.hasGold) {
                std::cout << "G  ";
            } else if (cell.hasWumpus) {
                std::cout << "W  ";
            } else if (cell.hasPit) {
                std::cout << "P  ";
            } else {
                std::cout << ".  ";
            }
        }

        std::cout << std::endl;
    }
}


// ==================== WORLD GENERATION ====================


// Places one Gold in random cell other than the starting position.
// Agent must discover Gold through the Glitter percept rather than
// receiving location directly.
void World::placeGold() {
    Position position = randPosition();


    while (position.x == 1 && position.y == 1) {
        position = randPosition();
    }

    grid[position.x - 1][position.y - 1].hasGold = true;
}

// The initial position and its two possible neighboring cells are protected.
// This ensures required initial state at [1,1] has no Breeze or Stench.
bool World::isProtectedStartSquare(Position position) const {
    return (position.x == 1 && position.y == 1) ||
           (position.x == 2 && position.y == 1) ||
           (position.x == 1 && position.y == 2);
}

// Places 1 Wumpus outside the protected starting area.
// The wumpusAlive flag allows Wumpus location to remain part
// of the physical world even after Agent successfully shoots it.
void World::placeWumpus() {
    Position position = randPosition();

    while (isProtectedStartSquare(position)) {
        position = randPosition();
    }

    grid[position.x - 1][position.y - 1].hasWumpus = true;
    wumpusAlive = true;
}

// Each eligible cell independently has a 20% chance of containing a pit.
// Starting area is protected so Agent begins safely and the Gold
// cell is excluded from pit generation.
void World::placePits() {
    std::bernoulli_distribution pitChance(0.2);

    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            Position position{x + 1, y + 1};

            if (isProtectedStartSquare(position) || (grid[x][y].hasGold)) {
                continue;
            }

            if (pitChance(gen)) {
                grid[x][y].hasPit = true;
            }
        }
    }
}

// Generates a valid 1-based coordinate within the world dimensions.
// I kept World coordinates 1-based to match the assignment representation.
Position World::randPosition() {
    std::uniform_int_distribution<int> dist(1, n);

    Position pos{
        dist(gen),
        dist(gen)
    };

    return pos;
}


// ==================== WORLD BOUNDARIES / POSITIONING ====================


// Boundary checking is performed by World because World knows n.
// Agent does NOT call this directly and instead discovers boundaries
// when attempted Forward action produces a Bump.
bool World::isInsideWorld(Position position) const {
    return position.x >= 1 && position.x <= n &&
           position.y >= 1 && position.y <= n;
}

// Returns the physically valid (top, bottom, left, right) neighbors of a cell.
// Diagonal cells are NOT included because Wumpus World percepts only apply
// to directly adjacent squares.
std::vector<Position> World::getNeighbors(Position position) const {
    std::vector<Position> neighbors;

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

// Calculates coordinate directly in front of a position without
// changing the original Agent state. This is reused by movement and shooting.
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


// ==================== PERCEPTS ====================


// Breeze is perceived when at least one adjacent cell
// contains a pit. World determines the percept from the true map.
// Agent must reason about which neighboring cell caused it.
bool World::hasBreeze(Position position) const {
    std::vector<Position> neighbors = getNeighbors(position);

    for (Position neighbor: neighbors) {
        if (grid[neighbor.x - 1][neighbor.y - 1].hasPit) {
            return true;
        }
    }

    return false;
}

// Stench is perceived when Wumpus occupies an adjacent cell.
// Wumpus remains at its original location after death, allowing Agent
// to retain previously observed location evidence while wumpusAlive separately
// determines whether the Wumpus is still dangerous.
bool World::hasStench(Position position) const {
    std::vector<Position> neighbors = getNeighbors(position);

    for (Position neighbor: neighbors) {
        if (grid[neighbor.x - 1][neighbor.y - 1].hasWumpus) {
            return true;
        }
    }

    return false;
}

// Constructs Agent's percept for its current position.
// Stench, Breeze and Glitter depend on the current world state.
// Bump and Scream describe the result of the immediately previous action.
Percept World::getPercept(Position position, bool bump, bool scream) const {
    Percept percept;

    percept.stench = hasStench(position);
    percept.breeze = hasBreeze(position);
    percept.glitter = grid[position.x - 1][position.y - 1].hasGold;
    percept.bump = bump;
    percept.scream = scream;

    return percept;
}


// ==================== ACTIONS / WORLD INTERACTIONS ====================


// Attempts to move Agent one square forward.
// Returning false represents hitting a wall, so the Agent remains in its
// current square and later receives Bump percept.
bool World::moveForward(Position &position, Direction direction) const {
    Position attemptedPosition = getForwardPosition(position, direction);

    if (!isInsideWorld(attemptedPosition)) {
        return false;
    }

    position = attemptedPosition;
    return true;
}

// Gold can only be collected when Agent is standing in Gold cell.
// Removing Gold from the world also means Glitter will no longer be perceived.
bool World::grabGold(Position position) {
    Cell &cell = grid[position.x - 1][position.y - 1];

    if (!cell.hasGold) {
        return false;
    }

    cell.hasGold = false;
    return true;
}

// The arrow travels in straight line in Agent's facing direction.
// It continues until it either reaches the live Wumpus or exits the world.
// A successful hit changes only the Wumpus's alive state, preserving its
// physical location for previously established logical knowledge.
bool World::shoot(Position position, Direction direction) {
    Position arrowPosition = getForwardPosition(position, direction);

    while (isInsideWorld(arrowPosition)) {
        const Cell &cell = grid[arrowPosition.x - 1][arrowPosition.y - 1];
        if (cell.hasWumpus && wumpusAlive) {
            wumpusAlive = false;
            return true;
        }
        arrowPosition = getForwardPosition(arrowPosition, direction);
    }
    return false;
}

// Pit cell is always fatal, while Wumpus cell is only fatal while
// Wumpus is alive. This allows Agent to safely enter the Wumpus's
// former location after hearing Scream.
bool World::isDeadly(Position position) const {
    const Cell &cell = grid[position.x - 1][position.y - 1];

    return cell.hasPit || (cell.hasWumpus && wumpusAlive);
}

// Climbing is only valid from the starting square [1,1].
// Whether Agent has Gold is handled separately by the game scoring logic.
bool World::canClimb(Position position) const {
    return position.x == 1 && position.y == 1;
}
