//
// Created by Mina Armijo on 10/26/26.
//

#ifndef GAME_H
#define GAME_H

#include "World.h"
#include "Agent.h"

class Game {
    public:
    ActionResult executeAction(Action action, Agent& agent, World& world);
};

#endif //GAME_H
