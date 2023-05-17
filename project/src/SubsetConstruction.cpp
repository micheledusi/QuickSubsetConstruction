/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * SubsetConstruction.cpp
 *
 *
 * This source file contains the definition of the class SubsetConstruction.
 * The Subset Construction algorithm is needed to build a DFA from a NFA. It represents the benchmark
 * for the determinization task over finite automata.
 */

#include "SubsetConstruction.hpp"

#include <queue>

#include "Debug.hpp"
#include "Properties.hpp"
#include "State.hpp"

namespace quicksc {

	/**
	 * Constructor.
	 */
	SubsetConstruction::SubsetConstruction() : DeterminizationAlgorithm(SC_ABBR, SC_NAME) {};

	/**
	 * Destructor.
	 */
	SubsetConstruction::~SubsetConstruction() {};

	/**
	 * Returns the NFA passed as parameter, optionally converting its states to the proper State class.
	*/
	Automaton* SubsetConstruction::prepareInputAutomaton(Automaton* nfa) const {
		return nfa->cloneConvertingStates<State>();
	}

	/**
	 * Returns the DFA obtained by the Subset Construction algorithm.
	 * It runs the algorithm on the NFA passed as parameter.
	 */
	Automaton* SubsetConstruction::run(Automaton* nfa) {
		Automaton* dfa = new Automaton();

        // Create the initial state of the DFA
		Extension initial_dfa_extension;
		State* nfa_initial_state = nfa->getInitialState();
		DEBUG_ASSERT_NOT_NULL(nfa_initial_state);
		initial_dfa_extension.insert(nfa_initial_state);
		Extension epsilon_closure = ConstructedState::computeEpsilonClosure(initial_dfa_extension);
		ConstructedState * initial_dfa_state = new ConstructedState(epsilon_closure);

		// Adding the initial state to the DFA
        dfa->addState(initial_dfa_state);

        // Creating a stack of states to be processed
        std::queue<ConstructedState*> singularities_stack;

        // The first state to be processed is the initial state
        singularities_stack.push(initial_dfa_state);

        // Continue untile the stack is empty
        while (! singularities_stack.empty()) {

        	// Pop the first state from the stack
        	ConstructedState* current_state = singularities_stack.front();			// Obtain the extracted state
            singularities_stack.pop();								// Remove the extracted state from the stack

			// For all the labels that mark outgoing transitions from this state
            for (string l : current_state->getLabelsExitingFromExtension()) {
				// We skip the epsilon-transitions
            	if (l == EPSILON) {
            		continue;
            	}

				// We compute the l-closure of the state and create a new DFA state
            	Extension l_closure = current_state->computeLClosureOfExtension(l);
            	ConstructedState* new_state = new ConstructedState(l_closure);
            	DEBUG_LOG("From state %s, with label %s, the state %s has been created",
            			current_state->getName().c_str(),
						l.c_str(),
						new_state->getName().c_str());

                // Check if the new state has an empty extension
                if (new_state->isExtensionEmpty()) {
					// If so, we delete it
                	DEBUG_LOG("Empty state %s has been deleted", new_state->getName().c_str());
                    delete new_state;
                    continue;
                }
				// Check if the new state is already present in the DFA (according to the name)
                else if (dfa->hasState(new_state->getName())) {
					// If so, we delete the extracted state
                	DEBUG_LOG("The state %s is already present in the DFA, we can delete the new extracted state", new_state->getName().c_str());
                	ConstructedState* tmp_state = new_state;
                    new_state = dynamic_cast<ConstructedState*> (dfa->getState(tmp_state->getName()));
                    delete tmp_state;
                }
                // If it's a new state
                else {
                	// We add it to the DFA
                	DEBUG_LOG("The state is new, we add it to the DFA");
                    dfa->addState(new_state);
                    singularities_stack.push(new_state);
                }

                // We create the transition between the current state and the new state
                //	state--(l)-->new_state
                current_state->connectChild(l, new_state);
            }
        }

        // Set the initial state of the DFA
		// This procedure sets the distances from the initial state to all the other states, automatically
        dfa->setInitialState(initial_dfa_state);

        return dfa;
	}

	/**
	 * Constructor.
	 */
	RedundantSubsetConstruction::RedundantSubsetConstruction() : DeterminizationAlgorithm(RSC_ABBR, RSC_NAME) {};

	/**
	 * Destructor.
	 */
	RedundantSubsetConstruction::~RedundantSubsetConstruction() {};

	/**
	 * Returns the NFA passed as parameter, optionally converting its states to the proper State class.
	*/
	Automaton* RedundantSubsetConstruction::prepareInputAutomaton(Automaton* nfa) const {
		return nfa->cloneConvertingStates<BidirectionalState>();
	}

	/**
	 * Returns the DFA obtained by the Subset Construction algorithm.
	 * It runs the algorithm on the NFA passed as parameter.
	 */
	Automaton* RedundantSubsetConstruction::run(Automaton* nfa) {
		Automaton* dfa = new Automaton();

        // Create the initial state of the DFA
		Extension initial_dfa_extension;
		BidirectionalState* nfa_initial_state = dynamic_cast<BidirectionalState*> (nfa->getInitialState());
		DEBUG_ASSERT_NOT_NULL(nfa_initial_state);
		initial_dfa_extension.insert(nfa_initial_state);
		Extension epsilon_closure = ConstructedState::computeEpsilonClosure(initial_dfa_extension);
		BidirectionalConstructedState * initial_dfa_state = new BidirectionalConstructedState(epsilon_closure);

		// Adding the initial state to the DFA
        dfa->addState(initial_dfa_state);

        // Creating a stack of states to be processed
        std::queue<BidirectionalConstructedState*> singularities_stack;

        // The first state to be processed is the initial state
        singularities_stack.push(initial_dfa_state);

        // Continue untile the stack is empty
        while (! singularities_stack.empty()) {

        	// Pop the first state from the stack
        	BidirectionalConstructedState* current_state = singularities_stack.front();			// Obtain the extracted state
            singularities_stack.pop();								// Remove the extracted state from the stack

			// For all the labels that mark outgoing transitions from this state
            for (string l : current_state->getLabelsExitingFromExtension()) {
				// We skip the epsilon-transitions
            	if (l == EPSILON) {
            		continue;
            	}

				// We compute the l-closure of the state and create a new DFA state
            	Extension l_closure = current_state->computeLClosureOfExtension(l);
            	BidirectionalConstructedState* new_state = new BidirectionalConstructedState(l_closure);
            	DEBUG_LOG("From state %s, with label %s, the state %s has been created",
            			current_state->getName().c_str(),
						l.c_str(),
						new_state->getName().c_str());

                // Check if the new state has an empty extension
                if (new_state->isExtensionEmpty()) {
					// If so, we delete it
                	DEBUG_LOG("Empty state %s has been deleted", new_state->getName().c_str());
                    delete new_state;
                    continue;
                }
				// Check if the new state is already present in the DFA (according to the name)
                else if (dfa->hasState(new_state->getName())) {
					// If so, we delete the extracted state
                	DEBUG_LOG("The state %s is already present in the DFA, we can delete the new extracted state", new_state->getName().c_str());
                	BidirectionalConstructedState* tmp_state = new_state;
                    new_state = dynamic_cast<BidirectionalConstructedState*> (dfa->getState(tmp_state->getName()));
                    delete tmp_state;
                }
                // If it's a new state
                else {
                	// We add it to the DFA
                	DEBUG_LOG("The state is new, we add it to the DFA");
                    dfa->addState(new_state);
                    singularities_stack.push(new_state);
                }

                // We create the transition between the current state and the new state
                //	state--(l)-->new_state
                current_state->connectChild(l, new_state);
            }
        }

        // Set the initial state of the DFA
		// This procedure sets the distances from the initial state to all the other states, automatically
        dfa->setInitialState(initial_dfa_state);

        return dfa;
	}
}
