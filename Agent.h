//
// Created by Mina Armijo on 10/26/26.
//

#ifndef AGENT_H
#define AGENT_H

#include "World.h"

class Agent {
    private:
        Position position{1, 1};
        Direction direction = Direction::East;
        bool hasArrow = true;
        bool hasGold = false;
        bool alive = true;
        int score = 0;

    public:
    Position getPosition() const;
    Direction getDirection() const;
    bool isAlive() const;
    int getScore() const;
    bool hasArrowAvailable() const;
    bool hasGoldCollected() const;
    void turnLeft();
    void turnRight();
    bool moveForward(World& world);
    void kill();
    bool grab(World& world);
    bool shoot(World& world);
};

#endif //AGENT_H
