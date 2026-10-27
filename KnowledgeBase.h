//
// Created by Mina Armijo on 10/26/26.
//

#ifndef KNOWLEDGEBASE_H
#define KNOWLEDGEBASE_H

#include "World.h"
#include <vector>

enum class PropositionType {
    Breeze,
    Stench,
    Pit,
    Wumpus,
    Safe,
    Visited,
    CellValid
};

struct Proposition {
    PropositionType type;
    Position position;
    bool negated = false;
};

struct OrSentence {
    std::vector<Proposition> propositions;
};

enum class InferenceResult {
    NotApplicable,
    NoChange,
    Changed,
    Contradiction
};

class KnowledgeBase {
    private:
    std::vector<Proposition> propositions;
    std::vector<OrSentence> orSentences;
    std::vector<Position> getNeighbors(Position position) const;
    InferenceResult inferNoPits(Position position);
    InferenceResult inferNoWumpus(Position position);
    InferenceResult inferSafe(Position position);
    InferenceResult inferFromBreeze(Position position);
    InferenceResult inferFromStench(Position position);
    bool containsOrSentence(const OrSentence& sentence) const;
    InferenceResult addOrSentence(const OrSentence& sentence);
    InferenceResult simplifyOrSentence(OrSentence& sentence);
    InferenceResult simplifyOrSentences();
    bool wumpusAlive = true;
    InferenceResult inferUniqueWumpus();
    std::vector<Position> getRelevantPositions() const;
    bool containsPosition(
        const std::vector<Position>& positions,
        Position position
    ) const;


    public:
    KnowledgeBase();
    InferenceResult addProposition(Proposition proposition);
    bool containsProposition(Proposition proposition) const;
    InferenceResult addPerceptFacts(Percept percept, Position position);
    InferenceResult markVisited(Position position);
    InferenceResult markSafe(Position position);
    InferenceResult runInference();
    void markWumpusDead();
    bool isKnownSafe(Position position) const;
    bool isVisited(Position position) const;
    std::vector<Position> getSafeUnvisitedPositions() const;
    bool getKnownWumpusPosition(Position& position) const;
    bool isWumpusAlive() const;
    InferenceResult markCellValid(Position position);
    InferenceResult markCellInvalid(Position position);
    bool isKnownValid(Position position) const;
    bool isKnownInvalid(Position position) const;
};

#endif //KNOWLEDGEBASE_H
