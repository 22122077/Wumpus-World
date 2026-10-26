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
    Visited
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
    int n;
    std::vector<Proposition> propositions;
    std::vector<OrSentence> orSentences;
    bool isInsideWorld(Position position) const;
    std::vector<Position> getNeighbors(Position position) const;
    InferenceResult inferNoPits(Position position);
    InferenceResult inferNoWumpus(Position position);
    InferenceResult inferSafe(Position position);
    InferenceResult inferSafeNeighbors(Position position);
    InferenceResult inferFromBreeze(Position position);
    InferenceResult inferFromStench(Position position);
    bool containsOrSentence(const OrSentence& sentence) const;
    InferenceResult addOrSentence(const OrSentence& sentence);
    InferenceResult simplifyOrSentence(OrSentence& sentence);
    InferenceResult simplifyOrSentences();
    bool wumpusAlive = true;
    InferenceResult inferUniqueWumpus();


    public:
    explicit KnowledgeBase(int size);
    InferenceResult addProposition(Proposition proposition);
    bool containsProposition(Proposition proposition) const;
    InferenceResult addPerceptFacts(Percept percept, Position position);
    InferenceResult markVisited(Position position);
    InferenceResult markSafe(Position position);
    InferenceResult runInference();
    void markWumpusDead();
};

#endif //KNOWLEDGEBASE_H
