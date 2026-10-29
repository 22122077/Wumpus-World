#include <iostream>
#include "World.h"
#include "Agent.h"
#include "Search.h"
#include "Game.h"


// ==================== OUTPUT HELPERS ====================


// Converts each action into readable output for observing Agent behavior.
const char *actionToString(Action action) {
    switch (action) {
        case Action::Forward:
            return "Forward";
        case Action::TurnLeft:
            return "TurnLeft";
        case Action::TurnRight:
            return "TurnRight";
        case Action::Grab:
            return "Grab";
        case Action::Shoot:
            return "Shoot";
        case Action::Climb:
            return "Climb";
    }

    return "Unknown";
}

// Converts each direction into readable output for observing Agent behavior.
const char *directionToString(Direction direction) {
    switch (direction) {
        case Direction::North:
            return "North";
        case Direction::East:
            return "East";
        case Direction::South:
            return "South";
        case Direction::West:
            return "West";
    }

    return "Unknown";
}

// Displays the percept currently available to Agent.
// Multiple percepts may be present at the same time.
void printPercept(const Percept &percept) {
    std::cout << "Percept:";

    bool hasPercept = false;

    if (percept.stench) {
        std::cout << " Stench";
        hasPercept = true;
    }

    if (percept.breeze) {
        std::cout << " Breeze";
        hasPercept = true;
    }

    if (percept.glitter) {
        std::cout << " Glitter";
        hasPercept = true;
    }

    if (percept.bump) {
        std::cout << " Bump";
        hasPercept = true;
    }

    if (percept.scream) {
        std::cout << " Scream";
        hasPercept = true;
    }

    if (!hasPercept) {
        std::cout << " None";
    }

    std::cout << std::endl;
}


// ==================== MAIN GAME LOOP ====================


int main() {
    int n;

    // World supports variable n x n sizes rather than assuming a fixed map.
    do {
        std::cout << "Enter world size: ";
        std::cin >> n;
    }
    // World must be at least 2 x 2 so valid non-start positions exist.
    while (n < 2);

    // World generates the true hidden environment.
    // Only World knows n and the actual locations of hazards/Gold.
    World world(n);

    world.placeWumpus();
    world.placeGold();
    world.placePits();

    // Full map is printed only for observer/testing purposes.
    // Agent does NOT receive this layout.
    std::cout << "\nWorld layout (observer view):" << std::endl;
    world.printWorld();

    Agent agent;
    Game game;
    Search search;

    // Agent begins at [1,1] and receives only the percept available
    // at its current position rather than hidden World info.
    Percept percept = world.getPercept(
        agent.getPosition(),
        false,
        false
    );

    InferenceResult inferenceResult =
            agent.processPercept(percept);

    if (inferenceResult ==
        InferenceResult::Contradiction) {
        std::cout << "Knowledge base contradiction detected."
                << std::endl;
        return 0;
    }

    bool gameOver = false;

    // Each cycle follows the Agent loop:
    // perceive/reason -> choose action -> execute -> receive new percept.
    while (!gameOver && agent.isAlive()) {
        printPercept(percept);

        Action action = agent.chooseAction(
            percept,
            search
        );

        std::cout << "Action: "
                << actionToString(action)
                << std::endl;

        ActionResult actionResult =
                game.executeAction(
                    action,
                    agent,
                    world
                );

        // Bump and Scream are action-result percepts that Agent
        // receives during the next reasoning cycle.
        if (actionResult.bump) {
            std::cout << "Bump: wall detected" << std::endl;
        }

        if (actionResult.scream) {
            std::cout << "Scream: Wumpus killed" << std::endl;
        }

        Position currentPosition = agent.getPosition();

        std::cout << "Position: ["
                << currentPosition.x
                << ","
                << currentPosition.y
                << "]  Direction: "
                << directionToString(agent.getDirection())
                << "  Score: "
                << agent.getScore()
                << std::endl;

        gameOver = actionResult.gameOver;

        if (gameOver) {
            break;
        }

        // World generates the next percept from Agent's actual position
        // plus Bump/Scream produced by the previous action.
        percept = world.getPercept(
            agent.getPosition(),
            actionResult.bump,
            actionResult.scream
        );

        // Agent updates KB and completes inference before choosing
        // another action.
        inferenceResult =
                agent.processPercept(percept);

        if (inferenceResult ==
            InferenceResult::Contradiction) {
            std::cout << "Knowledge base contradiction detected."
                    << std::endl;
            break;
        }
    }

    std::cout << "Final score: "
            << agent.getScore()
            << std::endl;

    return 0;
}
