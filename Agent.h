//
// Created by Mina Armijo on 10/26/26.
//

#ifndef AGENT_H
#define AGENT_H

#include "World.h"
#include "KnowledgeBase.h"
#include <vector>

class Search;

class Agent {
private:
    // ==================== AGENT STATE ====================

    // Agent begins at [1,1] facing East as required by Wumpus World.
    // World size and hidden cell contents are NOT stored here.
    Position position{1, 1};
    Direction direction = Direction::East;

    bool hasArrow = true;
    bool hasGold = false;
    bool alive = true;
    int score = 0;

    // All facts learned from percepts and inference are stored separately
    // from real World state inside Agent's KB.
    KnowledgeBase knowledgeBase;

    // ==================== PRIVATE HELPERS ====================

    // Returns the next turn or Shoot action needed once Agent has
    // inferred Wumpus location and is aligned with it.
    Action getShootAction(Direction targetDirection) const;

    // Calculates the coordinate directly in front of Agent without
    // relying on World boundaries, which Agent does NOT know.
    Position getForwardPosition(
        Position position,
        Direction direction
    ) const;

    // These KB queries are only needed internally for action selection.
    std::vector<Position> getSafeUnvisitedPositions() const;

    bool getKnownWumpusPosition(Position &position) const;

    bool isWumpusAlive() const;

public:
    // ==================== AGENT SETUP / STATE ====================

    Agent();

    Position getPosition() const;

    Direction getDirection() const;

    bool isAlive() const;

    int getScore() const;

    bool hasArrowAvailable() const;

    bool hasGoldCollected() const;

    // ==================== PHYSICAL ACTIONS ====================
    void turnLeft();

    void turnRight();

    bool moveForward(World &world);

    void kill(); // Marks Agent as dead after entering a deadly cell.
    bool grab(World &world);

    bool shoot(World &world);

    bool climb(World &world) const;

    void updateScore(int amount);

    // ==================== KNOWLEDGE / PERCEPT PROCESSING ====================

    // Updates KB from current percept and runs inference before
    // Agent chooses its next action.
    InferenceResult processPercept(Percept percept);

    // Search uses these queries so it only plans movement through
    // positions Agent has logically established as safe/valid.
    bool knowsSafe(Position position) const;

    bool knowsValid(Position position) const;

    // ==================== ACTION SELECTION ====================

    // Chooses one action from current percept, KB and BFS search results.
    // Agent replans after each action as new percept info becomes available.
    Action chooseAction(
        Percept percept,
        const Search &search
    ) const;
};

#endif //AGENT_H
