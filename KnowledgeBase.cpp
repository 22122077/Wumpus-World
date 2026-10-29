//
// Created by Mina Armijo on 10/26/26.
//

#include "KnowledgeBase.h"


// ==================== FIXED-POINT INFERENCE ====================


// Repeatedly applies all inference rules until a complete pass produces
// no new info. This allows one inferred fact to trigger further deductions
// during the next pass rather than stopping after a single rule application.
InferenceResult KnowledgeBase::runInference() {
    bool anyChange = false;
    bool passChanged;

    do {
        passChanged = false;

        // Agent does NOT know n, inference only checks positions that
        // have appeared in known facts or logical constraints.
        std::vector<Position> relevantPositions =
                getRelevantPositions();

        for (Position position: relevantPositions) {
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

        // Existing OR constraints are simplified before comparing Wumpus
        // constraints so candidates already ruled out are removed first.
        InferenceResult simplifyResult =
                simplifyOrSentences();

        if (simplifyResult ==
            InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (simplifyResult ==
            InferenceResult::Changed) {
            passChanged = true;
            anyChange = true;
        }

        // Multiple Stench constraints can further narrow Wumpus location.
        InferenceResult wumpusConstraintResult =
                inferWumpusFromConstraints();

        if (wumpusConstraintResult ==
            InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (wumpusConstraintResult ==
            InferenceResult::Changed) {
            passChanged = true;
            anyChange = true;
        }

        // Once one definite Wumpus is known, "exactly one" Wumpus reasoning
        // rules it out from all other currently relevant positions.
        InferenceResult uniqueWumpusResult =
                inferUniqueWumpus();

        if (uniqueWumpusResult ==
            InferenceResult::Contradiction) {
            return InferenceResult::Contradiction;
        }

        if (uniqueWumpusResult ==
            InferenceResult::Changed) {
            passChanged = true;
            anyChange = true;
        }
    } while (passChanged);

    if (anyChange) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}


// ==================== KNOWLEDGE STORAGE / UPDATES ====================


// Adds a proposition only if it is not already known.
// If its logical opposite already exists, KB has reached a contradiction.
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

// Checks for an exact proposition match including type, position and negation.
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

// Stores both positive and negative percept info.
// For example, no Breeze is stored as NOT Breeze so it can later
// trigger the rule that all adjacent positions contain no Pit.
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

// Records that Agent has physically occupied a position.
InferenceResult KnowledgeBase::markVisited(Position position) {
    Proposition visited{
        PropositionType::Visited,
        position,
        false
    };

    return addProposition(visited);
}

// A position Agent safely occupies is known to contain no Pit.
// While Wumpus is alive it also proves Wumpus is NOT in that position.
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


// ==================== ADJACENT POSITION REASONING ====================


// Unlike World::getNeighbors(), KB does NOT filter using n because Agent
// does NOT know world size. Coordinates may later be proven invalid by Bump.
std::vector<Position> KnowledgeBase::getNeighbors(
    Position position
) const {
    return {
        {position.x - 1, position.y},
        {position.x + 1, position.y},
        {position.x, position.y - 1},
        {position.x, position.y + 1}
    };
}


// ==================== NEGATIVE PERCEPT INFERENCE ====================


// NOT Breeze means none of the adjacent positions can contain a Pit.
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

// NOT Stench means none of the adjacent positions can contain Wumpus.
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

// A position is only inferred Safe when Pit danger has been ruled out and,
// while Wumpus is alive, Wumpus danger has also been ruled out.
// Invalid coordinates are never classified as usable safe cells.
InferenceResult KnowledgeBase::inferSafe(Position position) {
    if (isKnownInvalid(position)) {
        return InferenceResult::NotApplicable;
    }

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


// ==================== POSITIVE PERCEPT INFERENCE ====================


// Breeze proves that at least one possible adjacent position contains a Pit.
// Known impossible candidates are removed. One remaining candidate becomes
// definite, while multiple candidates are preserved as an OR constraint.
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
        if (isKnownInvalid(neighbor)) {
            continue;
        }

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

    // Breeze with no remaining possible Pit would conflict with KB.
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

// Stench proves that Wumpus occupies at least one possible adjacent position.
// Candidates already known impossible are removed. Remaining uncertainty is
// preserved rather than guessing a location.
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
        if (isKnownInvalid(neighbor)) {
            continue;
        }

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

    // Stench with no possible Wumpus location would conflict with KB.
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


// ==================== OR CONSTRAINT REASONING ====================


// Prevents equivalent OR constraints from being repeatedly stored.
// Proposition order does NOT matter when comparing sentences.
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

// Removes alternatives that have since been proven false.
// If one option remains it becomes a definite proposition.
// If no option remains, stored knowledge is inconsistent.
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

// Rechecks all stored OR constraints because newer percepts or deductions
// may have ruled out candidates that were previously uncertain.
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


// ==================== WUMPUS REASONING ====================


// Scream means Wumpus is no longer dangerous.
// Existing Wumpus location info is intentionally kept.
void KnowledgeBase::markWumpusDead() {
    wumpusAlive = false;
}

// Because exactly one Wumpus exists, the same Wumpus must satisfy every
// Stench constraint. Intersecting remaining candidate sets can therefore
// narrow possible locations or determine one definite location.
InferenceResult KnowledgeBase::inferWumpusFromConstraints() {
    std::vector<Position> sharedCandidates;
    int constraintCount = 0;

    for (const OrSentence &sentence: orSentences) {
        std::vector<Position> currentCandidates;
        bool isWumpusConstraint = false;

        for (const Proposition &proposition: sentence.propositions) {
            if (proposition.type != PropositionType::Wumpus ||
                proposition.negated) {
                continue;
            }

            isWumpusConstraint = true;

            Proposition noWumpus{
                PropositionType::Wumpus,
                proposition.position,
                true
            };

            if (!containsProposition(noWumpus) &&
                !isKnownInvalid(proposition.position)) {
                currentCandidates.push_back(proposition.position);
            }
        }

        if (!isWumpusConstraint) {
            continue;
        }

        // A Stench constraint with no possible location contradicts
        // the rule that exactly one Wumpus exists.
        if (currentCandidates.empty()) {
            return InferenceResult::Contradiction;
        }

        constraintCount++;

        if (constraintCount == 1) {
            sharedCandidates = currentCandidates;
            continue;
        }

        std::vector<Position> intersection;

        for (Position candidate: sharedCandidates) {
            if (containsPosition(currentCandidates, candidate)) {
                intersection.push_back(candidate);
            }
        }

        sharedCandidates = intersection;

        if (sharedCandidates.empty()) {
            return InferenceResult::Contradiction;
        }
    }

    // Intersection reasoning requires at least 2 Wumpus constraints.
    if (constraintCount < 2) {
        return InferenceResult::NotApplicable;
    }

    // One shared candidate identifies Wumpus location without guessing.
    if (sharedCandidates.size() == 1) {
        Proposition wumpus{
            PropositionType::Wumpus,
            sharedCandidates[0],
            false
        };

        return addProposition(wumpus);
    }

    // More than one shared candidate is still useful narrower knowledge,
    // so it is kept as another OR constraint for later inference.
    std::vector<Proposition> possibleWumpusLocations;

    for (Position position: sharedCandidates) {
        possibleWumpusLocations.push_back({
            PropositionType::Wumpus,
            position,
            false
        });
    }

    OrSentence narrowedConstraint{
        possibleWumpusLocations
    };

    return addOrSentence(narrowedConstraint);
}


// Once one definite Wumpus location is known, exactly-one-Wumpus reasoning
// means all other currently relevant positions can be marked NOT Wumpus.
InferenceResult KnowledgeBase::inferUniqueWumpus() {
    bool foundWumpus = false;
    Position wumpusPosition{};

    for (const Proposition &proposition: propositions) {
        if (proposition.type == PropositionType::Wumpus &&
            !proposition.negated) {
            // More than one definite Wumpus would violate world rules.
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

    std::vector<Position> relevantPositions =
            getRelevantPositions();

    for (Position position: relevantPositions) {
        if (position.x == wumpusPosition.x &&
            position.y == wumpusPosition.y) {
            continue;
        }

        Proposition noWumpus{
            PropositionType::Wumpus,
            position,
            true
        };

        InferenceResult result =
                addProposition(noWumpus);

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


// ==================== KNOWLEDGE QUERIES ====================


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

// Returns positions that are proven Safe but have not yet been visited.
// Known invalid coordinates are excluded from possible exploration targets.
std::vector<Position>
KnowledgeBase::getSafeUnvisitedPositions() const {
    std::vector<Position> positions;

    std::vector<Position> relevantPositions =
            getRelevantPositions();

    for (Position position: relevantPositions) {
        if (isKnownSafe(position) &&
            !isVisited(position) &&
            !isKnownInvalid(position)) {
            positions.push_back(position);
        }
    }

    return positions;
}

// Returns Wumpus location only when KB has a definite positive proposition.
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


// ==================== KNOWLEDGE HELPERS ====================


bool KnowledgeBase::containsPosition(
    const std::vector<Position> &positions,
    Position position
) const {
    for (const Position &existing: positions) {
        if (existing.x == position.x &&
            existing.y == position.y) {
            return true;
        }
    }

    return false;
}

// Builds working set of positions from everything Agent has learned.
// This replaces iterating over a known n x n grid because Agent does NOT
// know world dimensions.
std::vector<Position> KnowledgeBase::getRelevantPositions() const {
    std::vector<Position> positions;

    for (const Proposition &proposition: propositions) {
        if (!containsPosition(positions, proposition.position)) {
            positions.push_back(proposition.position);
        }
    }

    for (const OrSentence &sentence: orSentences) {
        for (const Proposition &proposition: sentence.propositions) {
            if (!containsPosition(positions, proposition.position)) {
                positions.push_back(proposition.position);
            }
        }
    }

    return positions;
}


// ==================== WORLD BOUNDARY KNOWLEDGE ====================


// Successful movement/current occupancy proves a coordinate exists.
InferenceResult KnowledgeBase::markCellValid(Position position) {
    Proposition validCell{
        PropositionType::CellValid,
        position,
        false
    };

    return addProposition(validCell);
}

// Bump proves an attempted coordinate is outside the world.
// An invalid coordinate also cannot contain a Pit or Wumpus, allowing
// boundary discoveries to simplify existing hazard constraints.
InferenceResult KnowledgeBase::markCellInvalid(Position position) {
    bool changed = false;

    Proposition invalidCell{
        PropositionType::CellValid,
        position,
        true
    };

    InferenceResult result =
            addProposition(invalidCell);

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

    if (changed) {
        return InferenceResult::Changed;
    }

    return InferenceResult::NoChange;
}

// Positive CellValid means Agent has confirmed the coordinate exists.
bool KnowledgeBase::isKnownValid(Position position) const {
    Proposition validCell{
        PropositionType::CellValid,
        position,
        false
    };

    return containsProposition(validCell);
}

// Negative CellValid means Agent has confirmed the coordinate is a wall/outside.
bool KnowledgeBase::isKnownInvalid(Position position) const {
    Proposition invalidCell{
        PropositionType::CellValid,
        position,
        true
    };

    return containsProposition(invalidCell);
}
