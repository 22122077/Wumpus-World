//
// Created by Mina Armijo on 10/27/26.
//

#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include "World.h"

class Agent;


// ==================== SEARCH REPRESENTATION ====================


// A search state includes both position and facing direction.
// Reaching the same cell facing a different direction is a different
// state because TurnLeft and TurnRight each count as actions.
struct SearchState {
    Position position;
    Direction direction;
};

// Stores one BFS state together with the action sequence used to reach it.
struct SearchNode {
    SearchState state;
    std::vector<Action> actions;
};

class Search {
public:
    // ==================== BFS PATH SEARCH ====================

    // Finds a shortest action sequence from Agent's current state
    // to one specific goal using only knowledge available to Agent.
    std::vector<Action> findPathTo(
        const Agent &agent,
        Position goal
    ) const;

    // Finds a shortest action sequence to any available goal.
    // This is used when Agent has multiple safe unexplored cells to choose from.
    std::vector<Action> findPathToAny(
        const Agent &agent,
        const std::vector<Position> &goals
    ) const;

private:
    // ==================== SEARCH HELPERS ====================

    // Search tracks direction as well as position so the same state
    // is not repeatedly added to the BFS frontier.
    bool containsState(
        const std::vector<SearchState> &states,
        SearchState state
    ) const;

    Direction turnLeft(Direction direction) const;

    Direction turnRight(Direction direction) const;

    Position getForwardPosition(
        Position position,
        Direction direction
    ) const;

    // Checks whether a position matches one of the possible BFS goals.
    bool containsPosition(
        const std::vector<Position> &positions,
        Position position
    ) const;
};

#endif //SEARCH_H
