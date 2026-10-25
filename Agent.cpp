//
// Created by Mina Armijo on 10/26/26.
//

#include "Agent.h"

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