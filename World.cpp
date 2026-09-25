//
// Created by Mina Armijo on 9/25/26.
//

#include "World.h"

World::World(int size)
    : n(size),
        grid(size, vector<Cell>(size)) {

}