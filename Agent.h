//
// Created by Mina Armijo on 10/26/26.
//

#ifndef AGENT_H
#define AGENT_H

#include "World.h"
#include "KnowledgeBase.h"

class Search;

class Agent {
    private:
    Position position{1, 1};
    Direction direction = Direction::East;
    bool hasArrow = true;
    bool hasGold = false;
    bool alive = true;
    int score = 0;
    KnowledgeBase knowledgeBase;
    Action turnToward(Direction targetDirection) const;
    Position getForwardPosition(
        Position position,
        Direction direction
    ) const;

    public:
    Agent();
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
    bool climb(World& world) const;
    void updateScore(int amount);
    InferenceResult processPercept(Percept percept);
    bool knows(Proposition proposition) const;
    bool knowsSafe(Position position) const;
    bool knowsValid(Position position) const;
    bool hasVisited(Position position) const;
    std::vector<Position> getSafeUnvisitedPositions() const;
    Action chooseAction(
        Percept percept,
        const Search& search
    ) const;
    bool getKnownWumpusPosition(Position& position) const;
    bool isWumpusAlive() const;
};

#endif //AGENT_H
