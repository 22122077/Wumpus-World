//
// Created by Mina Armijo on 10/26/26.
//

#include "Game.h"


ActionResult executeAction(Action action, Agent& agent, World& world) {
    ActionResult result;

    agent.updateScore(-1);

    switch (action) {
        case Action::Forward: {
            bool moved = agent.moveForward(world);

            if (!moved) {
                result.bump = true;
                break;
            }

            if (world.isDeadly(agent.getPosition())) {
                agent.kill();
                agent.updateScore(-1000);
                result.gameOver = true;
            }

            break;
        }

        case Action::TurnLeft:
            agent.turnLeft();
            break;

        case Action::TurnRight:
            agent.turnRight();
            break;

        case Action::Grab:
            agent.grab(world);
            break;

        case Action::Shoot: {
            bool hadArrow = agent.hasArrowAvailable();
            bool scream = agent.shoot(world);

            if (hadArrow) {
                agent.updateScore(-10);
            }

            result.scream = scream;
            break;
        }

        case Action::Climb:
            if (agent.climb(world)) {
                result.gameOver = true;

                if (agent.hasGoldCollected()) {
                    agent.updateScore(1000);
                }
            }
            break;
    }

    return result;
}