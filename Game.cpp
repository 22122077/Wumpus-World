//
// Created by Mina Armijo on 10/26/26.
//

#include "Game.h"
#include "Agent.h"


// ==================== GAME ACTION EXECUTION ====================


// Executes one chosen action and applies Wumpus World scoring.
// Every action costs 1 point. Additional penalties/rewards are
// applied for shooting, death and successfully returning with Gold.
ActionResult Game::executeAction(Action action, Agent &agent, World &world) {
    ActionResult result;

    // Every action has a base cost of 1 point.
    agent.updateScore(-1);

    switch (action) {
        case Action::Forward: {
            bool moved = agent.moveForward(world);

            // Failed movement means Agent hit a wall and receives Bump.
            if (!moved) {
                result.bump = true;
                break;
            }

            // World checks the true cell contents only after physical movement.
            // Agent does NOT use this hidden info when choosing its action.
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

            // Using the single arrow costs an additional 10 points,
            // regardless of whether Wumpus is hit.
            if (hadArrow) {
                agent.updateScore(-10);
            }

            result.scream = scream;
            break;
        }

        case Action::Climb:
            if (agent.climb(world)) {
                result.gameOver = true;

                // Returning to [1,1] with Gold earns the 1000-point reward.
                if (agent.hasGoldCollected()) {
                    agent.updateScore(1000);
                }
            }
            break;
    }

    return result;
}
