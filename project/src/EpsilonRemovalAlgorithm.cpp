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
 *  - GlobalEpsilonRemovalAlgorithm: the implementation with epsilon-closure, aggregating interventions over transitions
 */

#include "EpsilonRemovalAlgorithm.hpp"

#include "Properties.hpp"
#include "State.hpp"
#include <deque>
#include <algorithm>

#define DEBUG_MODE

#include "Debug.hpp"

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
        IF_DEBUG_ACTIVE( int e_nfa_size = e_nfa->size(); )

        // For each state of the e-NFA
        // NOTE: we iterate over all the states that we have at the beginning. Even if some of them are removed, we don't care.
        for (State* state : e_nfa->getStatesList()) {
            DEBUG_LOG("Current state: %s out of %d", state->getName().c_str(), e_nfa_size);
            // If the current state is the initial state
            if (state == e_nfa->getInitialState()) {
                // We ignore it, because we don't want to remove epsilon transitions from the initial state
                // (It's a tricky case, we leave it to the determinization algorithm)
                DEBUG_LOG("The current state is the initial state, therefore we ignore it.");
                continue;
            }

            // We take the epsilon-children of the current state, i.e. the states reachable from the current state with an epsilon transition
            for (State* eps_child : state->getChildren(EPSILON)) {
                // NOTE: if no epsilon transition is found, the method getChildren() returns an empty set
                DEBUG_LOG("\tEpsilon child: %s", eps_child->getName().c_str());

                // The epsilon transition is removed
                state->disconnectChild(EPSILON, eps_child);
                DEBUG_LOG("\tEpsilon transition removed.");

                // If we're considering an eps-loop
                if (state != eps_child) {
                    // All the exiting transitions of the epsilon-child are added to the current state
                    // (ALSO the exiting epsilon transitions)
                    state->copyExitingTransitionsOf(eps_child);
                    DEBUG_LOG("\tExiting transitions from state %s copied as exiting transitions from state %s.", eps_child->getName().c_str(), state->getName().c_str());
                }  

                // If the epsilon-child is final, the current state becomes final
                if (eps_child->isFinal()) {
                    state->setFinal(true);
                }

                // If the epsilon-child has no incoming transitions, it's removed
                if (eps_child->getIncomingTransitionsCount() == 0) {
                    DEBUG_LOG("\tState %s has no incoming transitions, therefore it's removed.", eps_child->getName().c_str());
                    e_nfa->removeState(eps_child);
                }
                // \todo: Check if the removal of the eps-child is safe in the loop of the states
            }
        }
        // At the end, we update the levels
        e_nfa->recomputeAllDistances();

        DEBUG_ASSERT_TRUE( e_nfa->size() <= e_nfa_size );
        DEBUG_LOG("Epsilon removal algorithm completed. The new automaton has %d states.", e_nfa->size());

        // We return the e-NFA, modified
        return e_nfa;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // GlobalEpsilonRemovalAlgorithm

    GlobalEpsilonRemovalAlgorithm::GlobalEpsilonRemovalAlgorithm() : EpsilonRemovalAlgorithm(CER_ABBR, CER_NAME) {}

    GlobalEpsilonRemovalAlgorithm::~GlobalEpsilonRemovalAlgorithm() {}

    /**
     * This method implements the "epsilon removal" algorithm in a more soficied way.
     * Epsilon transitions are managed with the epsilon-closure, aggregating interventions over transitions.
     * This algorithm can delete multiple epsilon transitions at once.
     */
    Automaton* GlobalEpsilonRemovalAlgorithm::run(Automaton* e_nfa) {
        IF_DEBUG_ACTIVE( int e_nfa_size = e_nfa->size(); )
        DEBUG_LOG("Epsilon removal algorithm started. The automaton has %d states.", e_nfa_size);

        // Create a new queue of states, that will contain the states that have an exiting epsilon transition
        deque<State*> states_eps_parents = deque<State*>();
        // Create a new set of states, that will contain the states that have an incoming epsilon transition
        set<State*> states_eps_children = set<State*>();

        // For each state of the e-NFA
        for (State* state : e_nfa->getStatesList()) {
            DEBUG_LOG("Current state: %s", state->getName().c_str());

            // If the current state has epsilon transitions
            if (state->hasExitingTransition(EPSILON)) {
                // We add it to the set of states with an exiting epsilon transition
                states_eps_parents.push_back(state);
                DEBUG_LOG("\tState %s has at least one exiting epsilon transition.", state->getName().c_str());
            }
            else {
                // Otherwise, we skip to the next state
                DEBUG_LOG("\tState %s has no exiting epsilon transition; therefore, we skip it.", state->getName().c_str());
                continue;
            }
            
            // For each epsilon transition of the current state
            deque<State*> eps_children = deque<State*>();
            for (State* s : state->getChildren(EPSILON)) {
                eps_children.push_back(s);
            }

            while (!eps_children.empty()) {
                // Extracting the first epsilon child
                State* eps_child = eps_children.front();
                eps_children.pop_front();
                DEBUG_LOG("\tEpsilon child: %s", eps_child->getName().c_str());

                // We don't consider the epsilon loop
                if (state == eps_child) {
                    DEBUG_LOG("\t\tEpsilon loop found; therefore, we skip it.");
                    continue;
                } 

                // We add the epsilon-child to the set of states with an incoming epsilon transition
                bool insertion_flag = states_eps_children.insert(eps_child).second;
                if (insertion_flag) {
                    DEBUG_LOG("\t\tState %s has been inserted in the set of epsilon children.", eps_child->getName().c_str());
                }
                else {
                    DEBUG_LOG("\t\tState %s is already present in the set of epsilon children.", eps_child->getName().c_str());
                }

                // For each epsilon transition of the epsilon-child
                for (State* eps_grandchild : eps_child->getChildren(EPSILON)) {
                    DEBUG_LOG("\t\tEpsilon grandchild: %s", eps_grandchild->getName().c_str());

                    // If the grandchild is the current state or the epsilon-child, we skip it
                    if (eps_grandchild == state || eps_grandchild == eps_child) {
                        DEBUG_LOG("\t\t\tEpsilon loop found; therefore, we skip it.");
                        continue;
                    }

                    // We create the new epsilon transition
                    bool transition_added = e_nfa->connectStates(state, eps_grandchild, EPSILON);

                    if (transition_added) {
                        DEBUG_LOG("\t\t\tAdded epsilon transition from state %s to state %s.", state->getName().c_str(), eps_grandchild->getName().c_str());
                        // We add the grandchild to the set of children to be considered
                        eps_children.push_back(eps_grandchild);
                        DEBUG_LOG("\t\t\tState %s has been added to the list of epsilon children of state %s to process.", eps_grandchild->getName().c_str(), state->getName().c_str());
                        // Notice that we don't add the grandchild to the set of processing states if the transition is already present
                    }
                    else {
                        DEBUG_LOG("\t\t\tEpsilon transition from state %s to state %s already exists.", state->getName().c_str(), eps_grandchild->getName().c_str());
                    }
                }

            }
        }

        DEBUG_LOG("Starting the label back-propagation phase.");
        // For each state with an exiting epsilon transition
        while (!states_eps_parents.empty()) {
            State* state = states_eps_parents.front();
            states_eps_parents.pop_front();
            DEBUG_LOG("Current state: %s", state->getName().c_str());

            // For each epsilon transition of the current state
            for (State* eps_child : state->getChildren(EPSILON)) {
                DEBUG_LOG("\tEpsilon child: %s", eps_child->getName().c_str());
                
                // If the epsilon-child corresponds to the current state, we skip it
                if (state == eps_child) {
                    DEBUG_LOG("\t\tEpsilon loop found; therefore, we skip it.");
                    continue;
                }

                // For each transition of the epsilon-child
                
                for (auto &pair: eps_child->getExitingTransitionsRef()) {
                    string label = pair.first;
                    for (State* eps_grandchild: pair.second) {
                        if (label == EPSILON) {
                            continue;
                        }

                        DEBUG_LOG("\t\tConsidering the non-epsilon transition: %s --(%s)--> %s", 
                            eps_child->getName().c_str(), label.c_str(), eps_grandchild->getName().c_str());
                        
                        if (!state->hasExitingTransition(label, eps_grandchild)) {
                            DEBUG_LOG("\t\t\tThe transition does not exist; therefore, we add it: %s --(%s)--> %s", 
                                state->getName().c_str(), label.c_str(), eps_grandchild->getName().c_str());
                            state->connectChild(label, eps_grandchild);

                            // Then, we need to check if state has some incoming epsilon transitions
                            // If so, we need to add its parents to the set of states to process
                            if (state->hasIncomingTransition(EPSILON)) {
                                DEBUG_LOG("\t\t\tState %s has at least one incoming epsilon transition:", state->getName().c_str());

                                // For each epsilon-parent of the current state
                                for (State* eps_parent : state->getParents(EPSILON)) {
                                    DEBUG_LOG("\t\t\tIncoming epsilon transition: %s --(%s)--> %s", eps_parent->getName().c_str(), EPSILON_PRINT, state->getName().c_str());

                                    // If the epsilon-parent corresponds to the current state, we skip it
                                    if (state == eps_parent) {
                                        DEBUG_LOG("\t\t\t\tEpsilon loop found; therefore, we skip it.");
                                        continue;
                                    }

                                    // If the epsilon-parent is not already in the set of states to process, we add it
                                    deque<State*>::iterator it = std::find(states_eps_parents.begin(), states_eps_parents.end(), eps_parent);
                                    if (it == states_eps_parents.end()) {
                                        DEBUG_LOG("\t\t\t\tState %s has been added to the set of states to process.", eps_parent->getName().c_str());
                                        states_eps_parents.push_back(eps_parent);
                                    }
                                    else {
                                        DEBUG_LOG("\t\t\t\tState %s is already in the set of states to process.", eps_parent->getName().c_str());
                                    }
                                }
                            }
                        }
                        else {
                            DEBUG_LOG("\t\t\tThe transition %s --(%s)--> %s already exists; therefore, we skip it.", 
                                state->getName().c_str(), label.c_str(), eps_grandchild->getName().c_str());
                        }
                    }
                }


            }
        }

        // Here we should setup the final states, but it's not necessary for the determinization
        // TODO: setup final states

        // Finally, we remove all epsilon transitions
        DEBUG_LOG("Removing all epsilon transitions.");
        for (State* eps_child : states_eps_children) {
            for (State* eps_parent : eps_child->getParents(EPSILON)) {
                DEBUG_LOG("\tRemoving the transition: %s --(%s)--> %s", eps_parent->getName().c_str(), EPSILON_PRINT, eps_child->getName().c_str());
                eps_parent->disconnectChild(EPSILON, eps_child);
            }
        }

        DEBUG_LOG("Removing the unreachable states.");
        e_nfa->removeUnreachableStates();
        e_nfa->recomputeAllDistances();

        DEBUG_LOG("Returning the e-NFA.");
        return e_nfa;
    }


} /* namespace quicksc */