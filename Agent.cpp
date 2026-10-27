//
// Created by Mina Armijo on 10/26/26.
//

#include "Agent.h"
#include "Search.h"

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

Agent::Agent() {
    knowledgeBase.markCellValid(position);
    knowledgeBase.markVisited(position);
    knowledgeBase.markSafe(position);
}

InferenceResult Agent::processPercept(Percept percept) {
    bool changed = false;

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
    }
    else {
        InferenceResult result =
            knowledgeBase.markCellValid(position);

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    }

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

    result = knowledgeBase.markSafe(position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

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

bool Agent::moveForward(World &world) {
    return world.moveForward(position, direction);
}

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

bool Agent::knows(Proposition proposition) const {
    return knowledgeBase.containsProposition(proposition);
}

bool Agent::knowsSafe(Position position) const {
    return knowledgeBase.isKnownSafe(position);
}

bool Agent::hasVisited(Position position) const {
    return knowledgeBase.isVisited(position);
}

std::vector<Position> Agent::getSafeUnvisitedPositions() const {
    return knowledgeBase.getSafeUnvisitedPositions();
}

Action Agent::chooseAction(
    Percept percept,
    const Search &search
) const {
    if (percept.glitter) {
        return Action::Grab;
    }

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

    if (hasArrow && isWumpusAlive()) {
        Position wumpusPosition;

        if (getKnownWumpusPosition(wumpusPosition)) {
            if (wumpusPosition.y == position.y) {
                if (wumpusPosition.x > position.x) {
                    return turnToward(Direction::East);
                }

                if (wumpusPosition.x < position.x) {
                    return turnToward(Direction::West);
                }
            }

            if (wumpusPosition.x == position.x) {
                if (wumpusPosition.y > position.y) {
                    return turnToward(Direction::North);
                }

                if (wumpusPosition.y < position.y) {
                    return turnToward(Direction::South);
                }
            }
        }
    }

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

    if (position.x == home.x &&
        position.y == home.y) {
        return Action::Climb;
    }

    std::vector<Action> pathHome =
            search.findPathTo(*this, home);

    if (!pathHome.empty()) {
        return pathHome[0];
    }

    return Action::Climb;
}

bool Agent::getKnownWumpusPosition(Position &position) const {
    return knowledgeBase.getKnownWumpusPosition(position);
}

bool Agent::isWumpusAlive() const {
    return knowledgeBase.isWumpusAlive();
}

Action Agent::turnToward(Direction targetDirection) const {
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

bool Agent::knowsValid(Position position) const {
    return knowledgeBase.isKnownValid(position);
}