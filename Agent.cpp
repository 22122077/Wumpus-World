//
// Created by Mina Armijo on 10/26/26.
//

#include "Agent.h"
#include "Search.h"


// ==================== AGENT SETUP / STATE ====================


// Agent begins with only knowledge of its initial safe position.
// World size and hidden cell contents are NOT provided to Agent.
Agent::Agent() {
    knowledgeBase.markCellValid(position);
    knowledgeBase.markVisited(position);
    knowledgeBase.markSafe(position);
}

Position Agent::getPosition() const {
    return position;
}

Direction Agent::getDirection() const {
    return direction;
}

bool Agent::isAlive() const {
    return alive;
}

int Agent::getScore() const {
    return score;
}

bool Agent::hasArrowAvailable() const {
    return hasArrow;
}

bool Agent::hasGoldCollected() const {
    return hasGold;
}


// ==================== PERCEPT PROCESSING ====================


// Updates KB after every action using only Agent's current percept.
// Bump/Scream update world-state knowledge first, then current position
// facts are stored before running inference to a fixed point.
InferenceResult Agent::processPercept(Percept percept) {
    bool changed = false;

    // A failed Forward action leaves Agent in its current position.
    // The coordinate directly ahead can therefore be marked invalid.
    if (percept.bump) {
        Position attemptedPosition =
                getForwardPosition(position, direction);

        InferenceResult result =
                knowledgeBase.markCellInvalid(
                    attemptedPosition
                );

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    } else {
        // Successful occupancy confirms that current coordinate exists.
        InferenceResult result =
                knowledgeBase.markCellValid(position);

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    }

    // Scream confirms Wumpus is dead without removing previously
    // inferred info about its physical location.
    if (percept.scream) {
        knowledgeBase.markWumpusDead();
        changed = true;
    }

    InferenceResult result = knowledgeBase.markVisited(position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

    // Since Agent is still alive at its current position, that position
    // can safely be recorded as containing no active hazard.
    result = knowledgeBase.markSafe(position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

    // Breeze and Stench are converted into positive or negative
    // propositional facts before inference rules are applied.
    result = knowledgeBase.addPerceptFacts(percept, position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

    result = knowledgeBase.runInference();

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}


// ==================== PHYSICAL ACTIONS ====================


void Agent::turnLeft() {
    switch (direction) {
        case Direction::North:
            direction = Direction::West;
            break;

        case Direction::West:
            direction = Direction::South;
            break;

        case Direction::South:
            direction = Direction::East;
            break;

        case Direction::East:
            direction = Direction::North;
            break;
    }
}

void Agent::turnRight() {
    switch (direction) {
        case Direction::North:
            direction = Direction::East;
            break;

        case Direction::East:
            direction = Direction::South;
            break;

        case Direction::South:
            direction = Direction::West;
            break;

        case Direction::West:
            direction = Direction::North;
            break;
    }
}

// World performs actual boundary checking because Agent does NOT know n.
// False means movement failed and next percept will contain Bump.
bool Agent::moveForward(World &world) {
    return world.moveForward(position, direction);
}

// Marks Agent as dead after Game detects a deadly position.
void Agent::kill() {
    alive = false;
}

bool Agent::grab(World &world) {
    bool grabbed = world.grabGold(position);
    if (grabbed) {
        hasGold = true;
    }
    return grabbed;
}

// Agent has only one arrow. It is consumed whether or not Wumpus is hit.
bool Agent::shoot(World &world) {
    if (!hasArrow) {
        return false;
    }

    hasArrow = false;

    return world.shoot(position, direction);
}

bool Agent::climb(World &world) const {
    return world.canClimb(position);
}

void Agent::updateScore(int amount) {
    score += amount;
}


// ==================== KNOWLEDGE QUERIES ====================


// Search uses these public queries instead of accessing KB directly.
bool Agent::knowsSafe(Position position) const {
    return knowledgeBase.isKnownSafe(position);
}

bool Agent::knowsValid(Position position) const {
    return knowledgeBase.isKnownValid(position);
}

// Internal query used to choose unexplored targets that KB has proven safe.
std::vector<Position> Agent::getSafeUnvisitedPositions() const {
    return knowledgeBase.getSafeUnvisitedPositions();
}

bool Agent::getKnownWumpusPosition(Position &position) const {
    return knowledgeBase.getKnownWumpusPosition(position);
}

bool Agent::isWumpusAlive() const {
    return knowledgeBase.isWumpusAlive();
}


// ==================== ACTION SELECTION ====================

// Action priority:
// 1. Grab visible Gold.
// 2. Return home and Climb once Gold is collected.
// 3. Shoot a definitely known Wumpus when Agent is aligned with it.
// 4. BFS toward a safe unvisited position.
// 5. Return home and Climb when no safe exploration remains.
//
// Agent never deliberately enters a position unless KB has proven it Safe.
Action Agent::chooseAction(
    Percept percept,
    const Search &search
) const {
    if (percept.glitter) {
        return Action::Grab;
    }

    // Once Gold is collected, exploration stops and Agent uses BFS
    // through known safe cells to return to [1,1].
    if (hasGold) {
        Position home{1, 1};

        if (position.x == home.x &&
            position.y == home.y) {
            return Action::Climb;
        }

        std::vector<Action> pathHome =
                search.findPathTo(*this, home);

        if (!pathHome.empty()) {
            return pathHome[0];
        }
    }

    // Agent only shoots after KB has inferred one definite Wumpus position.
    // Shooting is attempted when Wumpus shares Agent's row or column.
    if (hasArrow && isWumpusAlive()) {
        Position wumpusPosition;

        if (getKnownWumpusPosition(wumpusPosition)) {
            if (wumpusPosition.y == position.y) {
                if (wumpusPosition.x > position.x) {
                    return getShootAction(Direction::East);
                }

                if (wumpusPosition.x < position.x) {
                    return getShootAction(Direction::West);
                }
            }

            if (wumpusPosition.x == position.x) {
                if (wumpusPosition.y > position.y) {
                    return getShootAction(Direction::North);
                }

                if (wumpusPosition.y < position.y) {
                    return getShootAction(Direction::South);
                }
            }
        }
    }

    // Safe unexplored positions become BFS goals.
    // Search returns primitive movement/turn actions and Agent executes
    // only the first action before processing a new percept and replanning.
    std::vector<Position> safeUnvisited =
            getSafeUnvisitedPositions();

    if (!safeUnvisited.empty()) {
        std::vector<Action> path =
                search.findPathToAny(*this, safeUnvisited);

        if (!path.empty()) {
            return path[0];
        }
    }

    Position home{1, 1};

    // If no provably safe exploration remains, Agent exits rather
    // than deliberately taking a risk through an unknown cell.
    if (position.x == home.x &&
        position.y == home.y) {
        return Action::Climb;
    }

    std::vector<Action> pathHome =
            search.findPathTo(*this, home);

    if (!pathHome.empty()) {
        return pathHome[0];
    }

    // Defensive fallback. Under normal operation a path home should
    // exist because Agent only travels through proven safe positions.
    return Action::Climb;
}


// ==================== PRIVATE HELPERS ====================


// Returns Shoot when Agent already faces Wumpus.
// Otherwise returns one turn toward required shooting direction.
// If direction is opposite, repeated replanning completes second turn.
Action Agent::getShootAction(Direction targetDirection) const {
    if (direction == targetDirection) {
        return Action::Shoot;
    }

    if ((direction == Direction::North && targetDirection == Direction::West) ||
        (direction == Direction::West && targetDirection == Direction::South) ||
        (direction == Direction::South && targetDirection == Direction::East) ||
        (direction == Direction::East && targetDirection == Direction::North)) {
        return Action::TurnLeft;
    }

    return Action::TurnRight;
}

// Calculates coordinate directly ahead without checking world boundaries.
// This is needed after Bump because Agent must identify which attempted
// coordinate has just been proven invalid.
Position Agent::getForwardPosition(
    Position position,
    Direction direction
) const {
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
