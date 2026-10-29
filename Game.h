//
// Created by Mina Armijo on 10/26/26.
//

#ifndef GAME_H
#define GAME_H

#include "World.h"

class Agent;


// ==================== GAME ACTION EXECUTION ====================


// Applies Agent's chosen action to World and returns any resulting
// Bump, Scream or game-ending state. Scoring is also handled here
// so Agent action selection stays separate from game rules.
class Game {
    public:
    ActionResult executeAction(Action action, Agent& agent, World& world);
};

#endif //GAME_H
