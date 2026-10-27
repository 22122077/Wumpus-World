#include <iostream>
#include "World.h"
#include "Agent.h"
#include "Search.h"
#include "Game.h"
#include "Search.h"

const char* actionToString(Action action) {
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

int main() {
    int n;
    do {
        std::cout << "Enter world size: ";
        std::cin >> n;
    }
    while (n < 2);

    World world(n);

    world.placeWumpus();
    world.placeGold();
    world.placePits();
    std::cout << "\nWorld layout (observer view):" << std::endl;
    world.printWorld();

    Agent agent;
    Game game;
    Search search;

    Percept percept = world.getPercept(
        agent.getPosition(),
        false,
        false
    );

    InferenceResult inferenceResult =
        agent.processPercept(percept);

    bool gameOver = false;

    while (!gameOver && agent.isAlive()) {
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
                  << "]  Score: "
                  << agent.getScore()
                  << std::endl;

        gameOver = actionResult.gameOver;

        if (gameOver) {
            break;
        }

        percept = world.getPercept(
            agent.getPosition(),
            actionResult.bump,
            actionResult.scream
        );

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