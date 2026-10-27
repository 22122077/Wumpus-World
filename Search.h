//
// Created by Mina Armijo on 10/27/26.
//

#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include "World.h"
#include "Agent.h"

struct SearchState {
    Position position;
    Direction direction;
};

struct SearchNode {
    SearchState state;
    std::vector<Action> actions;
};

class Search {
public:
    std::vector<Action> findPathTo(
        const Agent& agent,
        Position goal
    ) const;
    std::vector<Action> findPathToAny(
        const Agent& agent,
        const std::vector<Position>& goals
    ) const;

private:
    bool containsState(
        const std::vector<SearchState>& states,
        SearchState state
    ) const;
    Direction turnLeft(Direction direction) const;
    Direction turnRight(Direction direction) const;
    Position getForwardPosition(
        Position position,
        Direction direction
    ) const;
    bool containsPosition(
        const std::vector<Position>& positions,
        Position position
    ) const;
};

#endif //SEARCH_H
