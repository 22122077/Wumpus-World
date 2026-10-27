//
// Created by Mina Armijo on 10/26/26.
//

#include "KnowledgeBase.h"


KnowledgeBase::KnowledgeBase(int size)
    : n(size) {
}

InferenceResult KnowledgeBase::runInference() {
    bool anyChange = false;
    bool passChanged;

    do {
        passChanged = false;

        for (int x = 1; x <= n; x++) {
            for (int y = 1; y <= n; y++) {
                Position position{x, y};

                InferenceResult result = inferNoPits(position);

                if (result == InferenceResult::Contradiction) {
                    return InferenceResult::Contradiction;
                }

                if (result == InferenceResult::Changed) {
                    passChanged = true;
                    anyChange = true;
                }

                result = inferNoWumpus(position);

                if (result == InferenceResult::Contradiction) {
                    return InferenceResult::Contradiction;
                }

                if (result == InferenceResult::Changed) {
                    passChanged = true;
                    anyChange = true;
                }

                result = inferFromBreeze(position);

                if (result == InferenceResult::Contradiction) {
                    return InferenceResult::Contradiction;
                }

                if (result == InferenceResult::Changed) {
                    passChanged = true;
                    anyChange = true;
                }

                result = inferFromStench(position);

                if (result == InferenceResult::Contradiction) {
                    return InferenceResult::Contradiction;
                }

                if (result == InferenceResult::Changed) {
                    passChanged = true;
                    anyChange = true;
                }

                result = inferSafe(position);

                if (result == InferenceResult::Contradiction) {
                    return InferenceResult::Contradiction;
                }

                if (result == InferenceResult::Changed) {
                    passChanged = true;
                    anyChange = true;
                }
            }
        }

        InferenceResult uniqueWumpusResult = inferUniqueWumpus();

        if (uniqueWumpusResult == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (uniqueWumpusResult == InferenceResult::Changed) {
            passChanged = true;
            anyChange = true;
        }

        InferenceResult remainingWumpusResult =
                inferRemainingWumpus();

        if (remainingWumpusResult ==
            InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (remainingWumpusResult ==
            InferenceResult::Changed) {
            passChanged = true;
            anyChange = true;
        }

        InferenceResult simplifyResult = simplifyOrSentences();

        if (simplifyResult == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (simplifyResult == InferenceResult::Changed) {
            passChanged = true;
            anyChange = true;
        }
    } while (passChanged);

    if (anyChange) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

bool KnowledgeBase::isInsideWorld(Position position) const {
    return position.x >= 1 && position.x <= n &&
           position.y >= 1 && position.y <= n;
}

std::vector<Position> KnowledgeBase::getNeighbors(Position position) const {
    std::vector<Position> neighbors;

    Position left{position.x - 1, position.y};
    Position right{position.x + 1, position.y};
    Position bottom{position.x, position.y - 1};
    Position top{position.x, position.y + 1};

    if (isInsideWorld(left)) {
        neighbors.push_back(left);
    }

    if (isInsideWorld(right)) {
        neighbors.push_back(right);
    }

    if (isInsideWorld(bottom)) {
        neighbors.push_back(bottom);
    }

    if (isInsideWorld(top)) {
        neighbors.push_back(top);
    }

    return neighbors;
}

InferenceResult KnowledgeBase::addProposition(Proposition proposition) {
    Proposition opposite = proposition;
    opposite.negated = !opposite.negated;

    if (containsProposition(opposite)) {
        return InferenceResult::Contradiction;
    }

    if (containsProposition(proposition)) {
        return InferenceResult::NoChange;
    }

    propositions.push_back(proposition);
    return InferenceResult::Changed;
}

bool KnowledgeBase::containsProposition(Proposition proposition) const {
    for (const Proposition &existing: propositions) {
        if (existing.type == proposition.type &&
            existing.position.x == proposition.position.x &&
            existing.position.y == proposition.position.y &&
            existing.negated == proposition.negated) {
            return true;
        }
    }

    return false;
}

InferenceResult KnowledgeBase::addPerceptFacts(Percept percept, Position position) {
    Proposition breeze{
        PropositionType::Breeze,
        position,
        !percept.breeze
    };

    Proposition stench{
        PropositionType::Stench,
        position,
        !percept.stench
    };

    InferenceResult breezeResult = addProposition(breeze);

    if (breezeResult == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    InferenceResult stenchResult = addProposition(stench);

    if (stenchResult == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (breezeResult == InferenceResult::Changed ||
        stenchResult == InferenceResult::Changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

InferenceResult KnowledgeBase::markVisited(Position position) {
    Proposition visited{
        PropositionType::Visited,
        position,
        false
    };

    return addProposition(visited);
}

InferenceResult KnowledgeBase::markSafe(Position position) {
    bool changed = false;

    Proposition safe{
        PropositionType::Safe,
        position,
        false
    };

    InferenceResult result = addProposition(safe);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

    Proposition noPit{
        PropositionType::Pit,
        position,
        true
    };

    result = addProposition(noPit);

    if (result == InferenceResult::Contradiction) {
        return InferenceResult::Contradiction;
    }

    if (result == InferenceResult::Changed) {
        changed = true;
    }

    if (wumpusAlive) {
        Proposition noWumpus{
            PropositionType::Wumpus,
            position,
            true
        };

        result = addProposition(noWumpus);

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

InferenceResult KnowledgeBase::inferNoPits(Position position) {
    Proposition noBreeze{
        PropositionType::Breeze,
        position,
        true
    };

    if (!containsProposition(noBreeze)) {
        return InferenceResult::NotApplicable;
    }

    bool changed = false;

    std::vector<Position> neighbors = getNeighbors(position);

    for (Position neighbor: neighbors) {
        Proposition noPit{
            PropositionType::Pit,
            neighbor,
            true
        };

        InferenceResult result = addProposition(noPit);

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

InferenceResult KnowledgeBase::inferNoWumpus(Position position) {
    Proposition noStench{
        PropositionType::Stench,
        position,
        true
    };

    if (!containsProposition(noStench)) {
        return InferenceResult::NotApplicable;
    }

    bool changed = false;

    std::vector<Position> neighbors = getNeighbors(position);

    for (Position neighbor: neighbors) {
        Proposition noWumpus{
            PropositionType::Wumpus,
            neighbor,
            true
        };

        InferenceResult result = addProposition(noWumpus);

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

InferenceResult KnowledgeBase::inferSafe(Position position) {
    Proposition noPit{
        PropositionType::Pit,
        position,
        true
    };

    Proposition noWumpus{
        PropositionType::Wumpus,
        position,
        true
    };

    if (!containsProposition(noPit)) {
        return InferenceResult::NotApplicable;
    }

    if (wumpusAlive && !containsProposition(noWumpus)) {
        return InferenceResult::NotApplicable;
    }

    Proposition safe{
        PropositionType::Safe,
        position,
        false
    };

    return addProposition(safe);
}

InferenceResult KnowledgeBase::inferFromBreeze(Position position) {
    Proposition breeze{
        PropositionType::Breeze,
        position,
        false
    };

    if (!containsProposition(breeze)) {
        return InferenceResult::NotApplicable;
    }

    std::vector<Position> neighbors = getNeighbors(position);
    std::vector<Proposition> possiblePits;

    for (Position neighbor: neighbors) {
        Proposition noPit{
            PropositionType::Pit,
            neighbor,
            true
        };

        Proposition safe{
            PropositionType::Safe,
            neighbor,
            false
        };

        if (containsProposition(noPit) ||
            containsProposition(safe)) {
            continue;
        }

        Proposition possiblePit{
            PropositionType::Pit,
            neighbor,
            false
        };

        possiblePits.push_back(possiblePit);
    }

    if (possiblePits.empty()) {
        return InferenceResult::Contradiction;
    }

    if (possiblePits.size() == 1) {
        return addProposition(possiblePits[0]);
    }

    OrSentence sentence{
        possiblePits
    };

    return addOrSentence(sentence);
}

bool KnowledgeBase::containsOrSentence(const OrSentence &sentence) const {
    for (const OrSentence &existing: orSentences) {
        if (existing.propositions.size() != sentence.propositions.size()) {
            continue;
        }

        bool allMatch = true;

        for (const Proposition &proposition: sentence.propositions) {
            bool foundMatch = false;

            for (const Proposition &existingProposition: existing.propositions) {
                if (existingProposition.type == proposition.type &&
                    existingProposition.position.x == proposition.position.x &&
                    existingProposition.position.y == proposition.position.y &&
                    existingProposition.negated == proposition.negated) {
                    foundMatch = true;
                    break;
                }
            }

            if (!foundMatch) {
                allMatch = false;
                break;
            }
        }

        if (allMatch) {
            return true;
        }
    }

    return false;
}

InferenceResult KnowledgeBase::addOrSentence(const OrSentence &sentence) {
    if (containsOrSentence(sentence)) {
        return InferenceResult::NoChange;
    }

    orSentences.push_back(sentence);
    return InferenceResult::Changed;
}

InferenceResult KnowledgeBase::simplifyOrSentence(OrSentence &sentence) {
    bool changed = false;

    for (size_t i = 0; i < sentence.propositions.size();) {
        Proposition opposite = sentence.propositions[i];
        opposite.negated = !opposite.negated;

        if (containsProposition(opposite)) {
            sentence.propositions.erase(
                sentence.propositions.begin() + i
            );

            changed = true;
        } else {
            i++;
        }
    }

    if (sentence.propositions.empty()) {
        return InferenceResult::Contradiction;
    }

    if (sentence.propositions.size() == 1) {
        InferenceResult result = addProposition(
            sentence.propositions[0]
        );

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed || changed) {
            return InferenceResult::Changed;
        }

        return InferenceResult::NoChange;
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

InferenceResult KnowledgeBase::simplifyOrSentences() {
    bool changed = false;

    for (OrSentence &sentence: orSentences) {
        InferenceResult result = simplifyOrSentence(sentence);

        if (result == InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (result == InferenceResult::Changed) {
            changed = true;
        }
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

InferenceResult KnowledgeBase::inferFromStench(Position position) {
    Proposition stench{
        PropositionType::Stench,
        position,
        false
    };

    if (!containsProposition(stench)) {
        return InferenceResult::NotApplicable;
    }

    std::vector<Position> neighbors = getNeighbors(position);
    std::vector<Proposition> possibleWumpusLocations;

    for (Position neighbor: neighbors) {
        Proposition noWumpus{
            PropositionType::Wumpus,
            neighbor,
            true
        };

        if (containsProposition(noWumpus)) {
            continue;
        }

        Proposition possibleWumpus{
            PropositionType::Wumpus,
            neighbor,
            false
        };

        possibleWumpusLocations.push_back(possibleWumpus);
    }

    if (possibleWumpusLocations.empty()) {
        return InferenceResult::Contradiction;
    }

    if (possibleWumpusLocations.size() == 1) {
        return addProposition(possibleWumpusLocations[0]);
    }

    OrSentence sentence{
        possibleWumpusLocations
    };

    return addOrSentence(sentence);
}

void KnowledgeBase::markWumpusDead() {
    wumpusAlive = false;
}

InferenceResult KnowledgeBase::inferUniqueWumpus() {
    bool foundWumpus = false;
    Position wumpusPosition{};

    for (const Proposition &proposition: propositions) {
        if (proposition.type == PropositionType::Wumpus &&
            !proposition.negated) {
            if (foundWumpus &&
                (proposition.position.x != wumpusPosition.x ||
                 proposition.position.y != wumpusPosition.y)) {
                return InferenceResult::Contradiction;
            }

            foundWumpus = true;
            wumpusPosition = proposition.position;
        }
    }

    if (!foundWumpus) {
        return InferenceResult::NotApplicable;
    }

    bool changed = false;

    for (int x = 1; x <= n; x++) {
        for (int y = 1; y <= n; y++) {
            if (x == wumpusPosition.x &&
                y == wumpusPosition.y) {
                continue;
            }

            Proposition noWumpus{
                PropositionType::Wumpus,
                {x, y},
                true
            };

            InferenceResult result = addProposition(noWumpus);

            if (result == InferenceResult::Contradiction) {
                return InferenceResult::Contradiction;
            }

            if (result == InferenceResult::Changed) {
                changed = true;
            }
        }
    }

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

bool KnowledgeBase::isKnownSafe(Position position) const {
    Proposition safe{
        PropositionType::Safe,
        position,
        false
    };

    return containsProposition(safe);
}

bool KnowledgeBase::isVisited(Position position) const {
    Proposition visited{
        PropositionType::Visited,
        position,
        false
    };

    return containsProposition(visited);
}

std::vector<Position> KnowledgeBase::getSafeUnvisitedPositions() const {
    std::vector<Position> positions;

    for (int x = 1; x <= n; x++) {
        for (int y = 1; y <= n; y++) {
            Position position{x, y};

            if (isKnownSafe(position) &&
                !isVisited(position)) {
                positions.push_back(position);
            }
        }
    }

    return positions;
}

bool KnowledgeBase::getKnownWumpusPosition(Position &position) const {
    for (const Proposition &proposition: propositions) {
        if (proposition.type == PropositionType::Wumpus &&
            !proposition.negated) {
            position = proposition.position;
            return true;
        }
    }

    return false;
}

bool KnowledgeBase::isWumpusAlive() const {
    return wumpusAlive;
}

InferenceResult KnowledgeBase::inferRemainingWumpus() {
    int possibleCount = 0;
    Position possiblePosition{};

    for (int x = 1; x <= n; x++) {
        for (int y = 1; y <= n; y++) {
            Position position{x, y};

            Proposition noWumpus{
                PropositionType::Wumpus,
                position,
                true
            };

            if (!containsProposition(noWumpus)) {
                possibleCount++;
                possiblePosition = position;
            }
        }
    }

    if (possibleCount == 0) {
        return InferenceResult::Contradiction;
    }

    if (possibleCount == 1) {
        Proposition wumpus{
            PropositionType::Wumpus,
            possiblePosition,
            false
        };

        return addProposition(wumpus);
    }

    return InferenceResult::NotApplicable;
}
