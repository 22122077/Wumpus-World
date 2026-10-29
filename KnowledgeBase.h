//
// Created by Mina Armijo on 10/26/26.
//

#ifndef KNOWLEDGEBASE_H
#define KNOWLEDGEBASE_H

#include "World.h"
#include <vector>


// ==================== KNOWLEDGE REPRESENTATION ====================


// Types of facts Agent can store or infer about a position.
// A proposition can also be negated, allowing the KB to represent
// both positive and negative knowledge such as Pit(X) and NOT Pit(X).
enum class PropositionType {
    Breeze,
    Stench,
    Pit,
    Wumpus,
    Safe,
    Visited,
    CellValid
};

// Represents one propositional fact about a specific position.
// If negated is false the proposition is positive, otherwise it
// represents the logical negation of that fact.
struct Proposition {
    PropositionType type;
    Position position;
    bool negated = false;
};

// Stores uncertainty from positive percepts.
// For example, a Breeze can create Pit(A) OR Pit(B) until later
// knowledge rules out one or more possible locations.
struct OrSentence {
    std::vector<Proposition> propositions;
};

// Inference functions return more than just true/false so the fixed point
// loop knows whether knowledge changed or became inconsistent.
enum class InferenceResult {
    NotApplicable,
    NoChange,
    Changed,
    Contradiction
};

class KnowledgeBase {
private:
    // ==================== STORED KNOWLEDGE ====================

    // Definite positive and negative facts currently known by Agent.
    std::vector<Proposition> propositions;

    // Logical OR constraints used to preserve uncertain hazard locations.
    std::vector<OrSentence> orSentences;

    // Wumpus location knowledge is kept even after it is killed.
    // This flag separately tracks whether it is still a danger.
    bool wumpusAlive = true;

    // ==================== INFERENCE RULES ====================

    // Returns all 4 possible adjacent coordinates without assuming
    // they are inside the world. Agent does NOT know world size and
    // learns invalid coordinates later through Bump.
    std::vector<Position> getNeighbors(Position position) const;

    // Negative percepts allow definite hazard info to be inferred.
    InferenceResult inferNoPits(Position position);

    InferenceResult inferNoWumpus(Position position);

    // A cell becomes Safe once the required absence of hazards is known.
    InferenceResult inferSafe(Position position);

    // Positive percepts create or reduce possible hazard locations.
    InferenceResult inferFromBreeze(Position position);

    InferenceResult inferFromStench(Position position);

    // ==================== OR CONSTRAINT REASONING ====================

    bool containsOrSentence(const OrSentence &sentence) const;

    InferenceResult addOrSentence(const OrSentence &sentence);

    InferenceResult simplifyOrSentence(OrSentence &sentence);

    InferenceResult simplifyOrSentences();

    // ==================== WUMPUS REASONING ====================

    // Once Wumpus is known, the "exactly one" Wumpus rule
    // allows other relevant positions to be ruled out.
    InferenceResult inferUniqueWumpus();

    // Multiple Stench constraints are compared because the single Wumpus
    // must satisfy all of them. Their shared candidates can
    // narrow or uniquely determine its location.
    InferenceResult inferWumpusFromConstraints();

    // ==================== KNOWLEDGE HELPERS ====================

    // Relevant positions are discovered from facts and constraints rather
    // than by looping through a known n x n world.
    std::vector<Position> getRelevantPositions() const;

    bool containsPosition(
        const std::vector<Position> &positions,
        Position position
    ) const;

    // Adds facts while preventing a proposition and its negation
    // from both being accepted into the KB.
    InferenceResult addProposition(Proposition proposition);

    bool containsProposition(Proposition proposition) const;

    bool isVisited(Position position) const;

public:
    // ==================== KNOWLEDGE BASE UPDATES ====================

    KnowledgeBase() = default;

    // Converts current Breeze/Stench percepts into propositional facts.
    InferenceResult addPerceptFacts(Percept percept, Position position);

    InferenceResult markVisited(Position position);

    InferenceResult markSafe(Position position);

    // Repeatedly applies inference rules until no new facts can be derived.
    InferenceResult runInference();

    // Scream changes the danger state of the Wumpus without deleting
    // previously inferred info about its location.
    void markWumpusDead();

    // ==================== KNOWLEDGE QUERIES ====================

    bool isKnownSafe(Position position) const;

    // Provides safe unexplored targets for Agent's BFS navigation.
    std::vector<Position> getSafeUnvisitedPositions() const;

    bool getKnownWumpusPosition(Position &position) const;

    bool isWumpusAlive() const;

    // ==================== WORLD BOUNDARY KNOWLEDGE ====================

    // Cell validity is learned through successful movement or Bump.
    // If neither CellValid nor NOT CellValid exists, validity is unknown.
    InferenceResult markCellValid(Position position);

    InferenceResult markCellInvalid(Position position);

    bool isKnownValid(Position position) const;

    bool isKnownInvalid(Position position) const;
};

#endif //KNOWLEDGEBASE_H
