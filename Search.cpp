//
// Created by Mina Armijo on 10/27/26.
//

#include "Search.h"
#include <queue>

std::vector<Action> Search::findPathTo(
    const Agent &agent,
    Position goal
) const {
    std::queue<SearchNode> frontier;
    std::vector<SearchState> visitedStates;

    SearchState startState{
        agent.getPosition(),
        agent.getDirection()
    };

    SearchNode startNode{
        startState,
        {}
    };

    frontier.push(startNode);
    visitedStates.push_back(startState);

    while (!frontier.empty()) {
        SearchNode current = frontier.front();
        frontier.pop();

        if (current.state.position.x == goal.x &&
            current.state.position.y == goal.y) {
            return current.actions;
        }

        SearchState leftState{
            current.state.position,
            turnLeft(current.state.direction)
        };

        if (!containsState(visitedStates, leftState)) {
            SearchNode leftNode{
                leftState,
                current.actions
            };

            leftNode.actions.push_back(Action::TurnLeft);

            frontier.push(leftNode);
            visitedStates.push_back(leftState);
        }

        SearchState rightState{
            current.state.position,
            turnRight(current.state.direction)
        };

        if (!containsState(visitedStates, rightState)) {
            SearchNode rightNode{
                rightState,
                current.actions
            };

            rightNode.actions.push_back(Action::TurnRight);

            frontier.push(rightNode);
            visitedStates.push_back(rightState);
        }

        Position forwardPosition = getForwardPosition(
            current.state.position,
            current.state.direction
        );

        SearchState forwardState{
            forwardPosition,
            current.state.direction
        };

        if (agent.knowsSafe(forwardPosition) &&
            !containsState(visitedStates, forwardState)) {
            SearchNode forwardNode{
                forwardState,
                current.actions
            };

            forwardNode.actions.push_back(Action::Forward);

            frontier.push(forwardNode);
            visitedStates.push_back(forwardState);
        }
    }

    return {};
}

bool Search::containsState(
    const std::vector<SearchState> &states,
    SearchState state
) const {
    for (const SearchState &existing: states) {
        if (existing.position.x == state.position.x &&
            existing.position.y == state.position.y &&
            existing.direction == state.direction) {
            return true;
        }
    }

    return false;
}

Direction Search::turnLeft(Direction direction) const {
    switch (direction) {
        case Direction::North:
            return Direction::West;

        case Direction::West:
            return Direction::South;

        case Direction::South:
            return Direction::East;

        case Direction::East:
            return Direction::North;
    }

    return direction;
}

Direction Search::turnRight(Direction direction) const {
    switch (direction) {
        case Direction::North:
            return Direction::East;

        case Direction::East:
            return Direction::South;

        case Direction::South:
            return Direction::West;

        case Direction::West:
            return Direction::North;
    }

    return direction;
}

Position Search::getForwardPosition(
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

bool Search::containsPosition(
    const std::vector<Position>& positions,
    Position position
) const {
    for (const Position& existing : positions) {
        if (existing.x == position.x &&
            existing.y == position.y) {
            return true;
            }
    }

    return false;
}

std::vector<Action> Search::findPathToAny(
    const Agent& agent,
    const std::vector<Position>& goals
) const {
    std::queue<SearchNode> frontier;
    std::vector<SearchState> visitedStates;

    SearchState startState{
        agent.getPosition(),
        agent.getDirection()
    };

    SearchNode startNode{
        startState,
        {}
    };

    frontier.push(startNode);
    visitedStates.push_back(startState);

    while (!frontier.empty()) {
        SearchNode current = frontier.front();
        frontier.pop();

        if (containsPosition(
                goals,
                current.state.position)) {
            return current.actions;
        }

        SearchState leftState{
            current.state.position,
            turnLeft(current.state.direction)
        };

        if (!containsState(visitedStates, leftState)) {
            SearchNode leftNode{
                leftState,
                current.actions
            };

            leftNode.actions.push_back(Action::TurnLeft);

            frontier.push(leftNode);
            visitedStates.push_back(leftState);
        }

        SearchState rightState{
            current.state.position,
            turnRight(current.state.direction)
        };

        if (!containsState(visitedStates, rightState)) {
            SearchNode rightNode{
                rightState,
                current.actions
            };

            rightNode.actions.push_back(Action::TurnRight);

            frontier.push(rightNode);
            visitedStates.push_back(rightState);
        }

        Position forwardPosition = getForwardPosition(
            current.state.position,
            current.state.direction
        );

        SearchState forwardState{
            forwardPosition,
            current.state.direction
        };

        if (agent.knowsSafe(forwardPosition) &&
            !containsState(visitedStates, forwardState)) {

            SearchNode forwardNode{
                forwardState,
                current.actions
            };

            forwardNode.actions.push_back(Action::Forward);

            frontier.push(forwardNode);
            visitedStates.push_back(forwardState);
        }
    }

    return {};
}