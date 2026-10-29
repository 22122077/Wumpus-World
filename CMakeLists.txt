cmake_minimum_required(VERSION 3.16)
project(Wumpus_World)

set(CMAKE_CXX_STANDARD 20)

add_executable(Wumpus_World
        main.cpp
        World.cpp
        Agent.cpp
        Game.cpp
        KnowledgeBase.cpp
        Search.cpp
)
