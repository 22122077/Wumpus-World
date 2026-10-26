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

Agent::Agent(int worldSize)
    : knowledgeBase(worldSize) {

    knowledgeBase.markVisited(position);
    knowledgeBase.markSafe(position);
}

InferenceResult Agent::processPercept(Percept percept) {
    if (percept.scream) {
        knowledgeBase.markWumpusDead();
    }

    InferenceResult result = knowledgeBase.markVisited(position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    result = knowledgeBase.markSafe(position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    result = knowledgeBase.addPerceptFacts(percept, position);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    return knowledgeBase.runInference();
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

bool Agent::moveForward(World& world) {
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

