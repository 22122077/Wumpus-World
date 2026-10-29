# Wumpus World

## Overview

This project implements an intelligent agent for the Wumpus World environment using **propositional logic** and **breadth-first search (BFS)**.

The program generates a variable `n x n` world containing Gold, Wumpus and randomly placed Pits. Agent begins at `[1,1]` facing East. It does **not** know the size of World or the locations of hazards and Gold. Instead, it builds a knowledge base from percepts and the results of its own actions.

The complete map is printed at the beginning of each run for **observer/testing purposes only**. Agent does not use this displayed map when making decisions.

## Requirements and Compiler

The project uses C++20 and the standard C++ library only.

It was compiled and tested using:

```text
Apple clang version 21.0.0 (clang-2100.3.34.2)
Target: arm64-apple-darwin25.6.0
```

A C++20-compatible compiler is required.

## Compiling and Running

### macOS / Linux with Clang

From the project directory, compile using:

```bash
clang++ -std=c++20 main.cpp World.cpp Agent.cpp Game.cpp KnowledgeBase.cpp Search.cpp -o Wumpus_World
```

Then run:

```bash
./Wumpus_World
```

These commands were tested successfully using Apple Clang 21.0.0.

### Windows with MinGW g++

If using Windows with a C++20-compatible MinGW `g++` compiler installed, compile using:

```bash
g++ -std=c++20 main.cpp World.cpp Agent.cpp Game.cpp KnowledgeBase.cpp Search.cpp -o Wumpus_World.exe
```

Then run in Command Prompt:

```text
Wumpus_World.exe
```

or in PowerShell:

```powershell
.\Wumpus_World.exe
```

The compiler must support C++20 and be available in the system PATH.

### Program Input

The program will prompt for a world size:

```text
Enter world size: 4
```

The world size must be at least `2 x 2`.

## Agent Reasoning

I kept the real World state separate from what Agent knows.

`World` stores the actual map size and hidden locations of Gold, Wumpus and Pits. Agent instead builds knowledge using propositions such as `Breeze`, `Stench`, `Pit`, `Wumpus`, `Safe`, `Visited` and `CellValid`. Propositions can also be negated, such as `NOT Pit` or `NOT Wumpus`.

Negative percepts allow definite conclusions. For example, if Agent perceives no Breeze and no Stench at a position, adjacent positions can be inferred as `NOT Pit` and `NOT Wumpus`. Once both hazards are ruled out, those positions can be marked as `Safe`.

Positive percepts may only show that a hazard exists in one of several neighboring positions. In these cases, the knowledge base stores an **OR constraint** rather than guessing. For example:

```text
Breeze
→ Pit(A) OR Pit(B)
```

As Agent learns more facts, impossible alternatives are removed. If only one candidate remains, that hazard can be inferred as definite.

Inference runs repeatedly until no more knowledge can be added, allowing one new fact to produce further conclusions.

Because there is exactly one Wumpus, overlapping Stench constraints can also be compared to narrow down its location. Once Wumpus is known, Agent can turn toward it and Shoot if it is aligned in the same row or column. A successful shot produces Scream, after which Wumpus is no longer considered dangerous.

The propositional knowledge is then used by the **BFS search algorithm**. BFS does not inspect the hidden World map. Instead, propositional logic determines which positions Agent knows are safe, and BFS determines an efficient sequence of `Forward`, `TurnLeft` and `TurnRight` actions to reach them.

## Learning World Boundaries

Agent is not given `n`.

World boundaries are discovered through **Bump**. If Agent attempts to move forward and hits a wall, the attempted coordinate is recorded as `NOT CellValid`.

This means Agent learns the accessible boundaries through experience rather than being given the dimensions directly.

## Search and Movement

BFS is used to plan paths.

A search state contains both:

```text
(position, direction)
```

Direction is included because reaching the same position while facing a different direction may require a different number of actions.

Search considers `Forward`, `TurnLeft` and `TurnRight`, and only plans through positions Agent has established as safe.

For exploration, Agent may attempt to enter a Safe position whose validity is still unknown. If that position is outside World, Bump provides the information needed to update the knowledge base and replan.

## Action Selection

Agent generally prioritizes:

1. Grab Gold when Glitter is perceived.
2. Return to `[1,1]` and Climb after collecting Gold.
3. Shoot when Wumpus location is definitely known and aligned.
4. Use BFS to reach a Safe unvisited position.
5. Return home and Climb if no further safe exploration can be proven.

Agent follows a conservative policy and does not deliberately enter a position unless it has been inferred as Safe. Because of this, some generated worlds may contain reachable Gold that Agent cannot safely prove a route to. In that case, Agent returns home rather than taking an unknown risk.

## Scoring

```text
Each action:                -1
Using the arrow:            -10 additional
Agent death:                -1000 additional
Climb at [1,1] with Gold:   +1000
```

Agent has one arrow. The game ends when Agent dies or Climbs from `[1,1]`.

## Program Structure

`World` stores the real environment and generates percepts.

`KnowledgeBase` stores propositions and performs logical inference.

`Agent` processes percepts, maintains its state and chooses actions.

`Search` performs BFS using only knowledge available to Agent.

`Game` executes actions and applies scoring and game rules.

`main.cpp` controls the overall **perceive → reason → act** loop.