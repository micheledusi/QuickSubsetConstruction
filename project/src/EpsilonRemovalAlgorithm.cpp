/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * EpsonilonRemovalAlgorithm.cpp
 *
 * 
 * This source file contains the implementation of the class EpsilonRemovalAlgorithm.
 * An algorithm for "epsilon removal" is an algorithms that, given an e-NFA, returns
 * a NFA that is equivalent to the e-NFA, but without epsilon transitions.
 * 
 * The core functionality of this class is the method run(), which is abstract.
 * In fact, the procedure can be implemented in different ways. We'll see two of them:
 * 
 *  - NaiveEpsilonRemovalAlgorithm: the implementation withou epsilon-closure, managing transitions one by one. 
 *  - ClosuringEpsilonRemovalAlgorithm: the implementation with epsilon-closure, aggregating interventions over transitions
 */

#include "EpsilonRemovalAlgorithm.hpp"

#include "Properties.hpp"
#include "State.hpp"

namespace quicksc {

    /**
     * Base constructor.
     * It instantiates the name and abbreviation of the algorithm, identifying the current implementation among the others.
     */
    EpsilonRemovalAlgorithm::EpsilonRemovalAlgorithm(string abbr, string name) {
        this->m_abbr = abbr;
        this->m_name = name;
    }

    /**
     * Empty destructor.
     */
    EpsilonRemovalAlgorithm::~EpsilonRemovalAlgorithm() {}

    /**
     * This method returns the name of the algorithm.
     */
    const string& EpsilonRemovalAlgorithm::abbr() {
        return this->m_abbr;
    };

    /**
     * This method returns the abbreviation name of the algorithm, quickly identifying it.
     */
    const string& EpsilonRemovalAlgorithm::name() {
        return this->m_name;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // NaiveEpsilonRemovalAlgorithm

    NaiveEpsilonRemovalAlgorithm::NaiveEpsilonRemovalAlgorithm() : EpsilonRemovalAlgorithm(NER_ABBR, NER_NAME) {}

    NaiveEpsilonRemovalAlgorithm::~NaiveEpsilonRemovalAlgorithm() {}

    /**
     * This method implements the "epsilon removal" algorithm in a simple way.
     * Each epsilon transition is managed one by one, without aggregating interventions over transitions.
     * 
     * NOTE: The automaton passed as parameter is modified.
     * If you want to preserve the original automaton, you should clone it before calling this method.
     */
    Automaton* NaiveEpsilonRemovalAlgorithm::run(Automaton* e_nfa) {
        // For each state of the e-NFA
        for (State* state : e_nfa->getStatesList()) {
            // If the current state is the initial state
            if (state == e_nfa->getInitialState()) {
                // We ignore it, because we don't want to remove epsilon transitions from the initial state
                // (It's a tricky case, we leave it to the determinization algorithm)
                continue;
            }

            // We take the epsilon-children of the current state, i.e. the states reachable from the current state with an epsilon transition
            for (State* eps_child : state->getChildren(EPSILON)) {
                // NOTE: if no epsilon transition is found, the method getChildren() returns an empty set

                // The epsilon transition is removed
                state->disconnectChild(EPSILON, eps_child);
                // All the exiting transitions of the epsilon-child are added to the current state
                state->copyExitingTransitionsOf(eps_child);         
                // \todo: This call might cause a loop if there's a cycle with epsilon transitions. We assume that it's not the case.

                // If the epsilon-child is final, the current state becomes final
                if (eps_child->isFinal()) {
                    state->setFinal(true);
                }

                // If the epsilon-child has no incoming transitions, it's removed
                if (eps_child->getIncomingTransitionsCount() == 0) {
                    e_nfa->removeState(eps_child);
                }
                // \todo: Check if the removal of the eps-child is safe in the loop of the states
            }
        }
        // At the end, we update the levels
        e_nfa->getInitialState()->initDistancesRecursively(0);

        // We return the e-NFA, modified
        return e_nfa;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // ClosuringEpsilonRemovalAlgorithm

    ClosuringEpsilonRemovalAlgorithm::ClosuringEpsilonRemovalAlgorithm() : EpsilonRemovalAlgorithm(CER_ABBR, CER_NAME) {}

    ClosuringEpsilonRemovalAlgorithm::~ClosuringEpsilonRemovalAlgorithm() {}

    /**
     * This method implements the "epsilon removal" algorithm in a more soficied way.
     * Epsilon transitions are managed with the epsilon-closure, aggregating interventions over transitions.
     * This algorithm can delete multiple epsilon transitions at once.
     */
    Automaton* ClosuringEpsilonRemovalAlgorithm::run(Automaton* e_nfa) {
        /** TODO **/
        return e_nfa;
    }


} /* namespace quicksc */