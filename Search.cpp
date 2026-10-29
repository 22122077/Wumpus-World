//
// Created by Mina Armijo on 10/27/26.
//

#include "Search.h"
#include "Agent.h"
#include <queue>


// ==================== BFS TO ONE GOAL ====================


// Uses BFS to find the shortest sequence of primitive
// actions to one specific goal. Each Forward/Turn action has equal cost,
// so FIFO expansion gives the shortest action sequence.
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

        // Turning changes direction without changing position.
        // Direction is part of search state because turns also cost actions.
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

        // Known routes only move through cells Agent has proven both
        // Safe and valid. Search does NOT access hidden World state.
        if (agent.knowsSafe(forwardPosition) &&
            agent.knowsValid(forwardPosition) &&
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

    // Empty result means no path can be proven using current knowledge.
    return {};
}


// ==================== BFS TO ANY SAFE GOAL ====================


// Uses BFS to find the shortest action sequence to any safe unexplored goal.
// Unknown CellValid is permitted only for the final goal because Agent may
// need to attempt that cell before a Bump can reveal the world boundary.
std::vector<Action> Search::findPathToAny(
    const Agent &agent,
    const std::vector<Position> &goals
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

        bool isGoal = containsPosition(goals, forwardPosition);

        // Intermediate cells must have confirmed validity.
        // A safe goal may have unknown validity because Agent learns
        // boundaries only by attempting movement and receiving Bump.
        if (agent.knowsSafe(forwardPosition) &&
            (agent.knowsValid(forwardPosition) || isGoal) &&
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

    // Empty result means no safe goal can be reached using current knowledge.
    return {};
}


// ==================== SEARCH STATE HELPERS ====================


// Prevents the same position/direction state from being added repeatedly.
// States are marked visited when added to frontier rather than when removed.
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

bool Search::containsPosition(
    const std::vector<Position> &positions,
    Position position
) const {
    for (const Position &existing: positions) {
        if (existing.x == position.x &&
            existing.y == position.y) {
            return true;
        }
    }

    return false;
}


// ==================== SEARCH MOVEMENT HELPERS ====================


// These helpers simulate possible actions during search without changing
// Agent's real position or direction.
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