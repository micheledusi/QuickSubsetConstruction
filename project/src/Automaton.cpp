/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Automaton.cpp
 *
 *
 * Source file containing the "Automaton" class, which represents an abstract automaton.
 * This class is inherited by the two concrete subclasses DFA and NFA, respectively 
 * Deterministic Finite State Automaton and Non-Deterministic Finite State Automaton.
 */

#include "Automaton.hpp"

#include <algorithm>

//#define DEBUG_MODE
#include "Debug.hpp"

namespace quicksc {

	/**
     * Constructor.
     */
    Automaton::Automaton()
	: m_states() {
    	m_initial_state = NULL;
    }


    /**
     * Destructor.
     */
    Automaton::~Automaton() {
    	DEBUG_MARK_PHASE("Distruzione di un automa con %lu stati", this->m_states.size()) {

    	for (State* s : m_states) {
    		s->detachAllTransitions();
    	}
    	for (State* s : m_states) {
//    		this->m_states.erase(s);
    	}

    	}
    }

    /**
     * Returns the size of the automaton, i.e. the number of states.
     */
    int Automaton::size() {
        return m_states.size();
    }

    /**
     * Checks if the automaton contains the state s.
     * Comparisons are made by pointer, NOT by name.
     * To perform a comparison by name it is advisable to use
     * the same method that accepts a string as input.
     */
    bool Automaton::hasState(State* s) {
        return m_states.find(s) != m_states.end();
    }

    /**
     * Checks if the automaton contains a state with a specific name.
     * Comparisons are made by name, NOT by pointer.
     * In case of states with the same name, this method would return a positive result.
     */
    bool Automaton::hasState(string name) {
    	for (State* s : m_states) {
    		if (s->getName() == name) {
    			return true;
    		}
    	}
    	return false;
    }

    /**
     * Returns - if present - the state corresponding to the label passed as a parameter.
     * In case this state is not part of the automaton, a NULL pointer is returned.
     *
     * Note: In some cases (during the execution of construction algorithms) there is more than one state
     * with the same name. This condition occurs only within an execution and is "corrected" at the end,
     * but still does not allow the correct functioning of this method (which instead returns the first state found).
     * To avoid problems related to identical states, it is recommended to use the "getStatesByName" method
     * which returns a set containing ALL states with the same name.
     */
    State* Automaton::getState(string name) {
    	for (State* s : m_states) {
    		if (s->getName() == name) {
    			return s;
    		}
    	}
    	return NULL;
    }

    /**
     * Returns the set of all states with the name passed as a parameter.
     * Normally this method returns a single state, since the states are unique by name.
     * However, during the execution of construction algorithms, the automaton may be in special conditions
     * where there is more than one state with the same name. In that case, it is better to use this method and not "getState".
     */
    const vector<State*> Automaton::getStatesByName(string name) {
    	vector<State*> namesake_states; // Homonymous states
    	for (State* s : m_states) {
    		if (s->getName() == name) {
    			namesake_states.push_back(s);
    		}
    	}
    	return namesake_states;
    }

    /**
     * Adds a state to the map of states of this automaton.
     * In case there is already a state associated with that name, the existing one is overwritten.
     */
    void Automaton::addState(State* s) {
        m_states.insert(s);
    }

    /**
     * Requires a state as input.
     * It removes a state from the automaton that has the same name as the state passed as input.
     * If the removal is successful, it returns "TRUE", otherwise if the state is not found it returns "FALSE".
     * This method does NOT destroy the state.
     */
    bool Automaton::removeState(State* s) {
    	DEBUG_MARK_PHASE("Function \"detachAllTransitions\" sullo stato %s", s->getName().c_str()) {
    		s->detachAllTransitions();
    	}
    	DEBUG_LOG("Verifica dello stato dopo la funzione \"detachAllTransitions\" e prima di essere rimosso:\n%s", s->toString().c_str());
    	DEBUG_ASSERT_TRUE(this->hasState(s));
    	m_states.erase(s);
    	DEBUG_ASSERT_FALSE(this->hasState(s));
    	return true;
    }

    /**
     * Sets a state as the initial state.
     * At instantiation, the reference to the initial state has a NULL value.
     *
     * In case this method is called a second time, the initial node is
     * "overwritten", and the reference to the previous one is lost.
     *
     * Note: it is not possible to set as initial state a state that does not belong to the automaton.
     * In case the state does not belong to the automaton, the operation will not be performed.
     *
     * In addition, it is assumed that this operation is performed at the end of the insertion of all
     * the states, because it also causes the assignment of the distances for each node reachable from the state
     * set as the initial state. Therefore, at the end of this call, each state will contain the distance from the initial node,
     * starting from the initial node that will have distance 0.
     * The assignment of the distances DOES NOT affect the states NOT reachable from the state set as
     * initial state.
     */
    void Automaton::setInitialState(State* s) {
    	if (hasState(s)) {
			m_initial_state = s;
	    	s->initDistancesRecursively(0);
    	} else {
    		DEBUG_LOG_ERROR("Impossible to set %s as the initial state because it does not belong to the automaton", s->getName().c_str());
    	}
    }

    /**
     * Sets the state associated with the name passed as a parameter as the initial state.
     *
     * In case this method is called more than once, the initial node is
     * "overwritten" and the reference to the previous one is lost.
     *
     * Note: it is not possible to set as initial state a state that does not belong to the automaton.
     * In case the state does not belong to the automaton, the operation will not be performed.
     *
     * In addition, it is assumed that this operation is performed at the end of the insertion of all
     * the states, because it also causes the assignment of the distances for each node reachable from the state
     * set as the initial state. Therefore, at the end of this call, each state will contain the distance from the initial node,
     * starting from the initial node that will have distance 0.
     * The assignment of the distances DOES NOT affect the states NOT reachable from the state set as initial state.
     */
    void Automaton::setInitialState(string name) {
    	if (hasState(name)) {
			m_initial_state = getState(name);
	    	m_initial_state->initDistancesRecursively(0);
    	}
    }

    /**
     * Checks if a state associated with a certain name is set as the initial state.
     */
    bool Automaton::isInitial(string name) {
        return (m_initial_state->getName() == name);
    }

    /**
     * Checks if a state is set as the initial state.
     */
    bool Automaton::isInitial(State* s) {
        return (m_initial_state == s);
        /*
         * Note: previously the comparison was performed by name, but since the equality operator has been defined as a name comparison,
         * this implementation should not cause problems.
         */
    }

    /**
     * Returns the initial state.
     */
    State* Automaton::getInitialState() {
        return m_initial_state;
    }

    /**
     * Private method.
     * Removes from a set all the nodes that are reachable (read: CONNECTED) from the node s passed as a parameter
     * via a transition s->s', plus all those that are also reachable from them in cascade.
     * In addition, it also removes the state s.
     * This method is used to remove all the nodes connected (in a DIRECTIONAL way) to the initial state, that is, all those reachable in the automaton.
     */
    void Automaton::removeReachableStates(State* s, set<State*> &states) {
        // Check if the automaton contains the state s
        if (states.find(s) != states.end()) {
            states.erase(s);
            // Remove the state from the map. If I removed it AFTER the recursive call, a cycle
            // of transitions on the states would generate an unlimited recursive call stack.

            // For each exiting transition from s
            for (auto &pair: s->getExitingTransitions()) {
                // For each state reached by the transitions
                for (State* child: pair.second) {
                	removeReachableStates(child, states);   // Recursive call on the children
                }
            }
        }
    }

    /**
     * It removes the states of the automaton that are no longer reachable from the initial state,
     * that is, all the states of the automaton that cannot be "visited" via a sequence of transitions.
     * 
     * The idea is to take the set of all the states of the automaton and remove those that are reachable.
     * The states that will remain will necessarily be the unreachable states.
     * 
     * Returns the states that have been removed and that were unreachable.
     */
    set<State*> Automaton::removeUnreachableStates() {
        // Create the set of all the states of the automaton
        set<State*> unreachable = set<State*>(m_states.begin(), m_states.end());

        // Work by difference: remove from the set all the states that are reachable
        removeReachableStates(m_initial_state, unreachable);

        // The remaining states are the unreachable ones, on which I iterate
        for (State* s: unreachable) {
            // Remove from the automaton map every unreachable state
            m_states.erase(s);
        }

        return unreachable;
    }

    /**
     * Returns the list of all the states of the automaton in the form of a list.
     * The states are returned as pointers.
     * The "list" class allows an addition or removal in the middle of the list without reallocation of the queue.
     */
    const list<State*> Automaton::getStatesList() {
    	return list<State*>(m_states.begin(), m_states.end());
    }

    /**
     * Returns the dynamic vector of all the states of the automaton in the form of vector.
     * The states are returned as pointers.
     * The "vector" class allows random access with constant time.
     */
    const vector<State*> Automaton::getStatesVector() {
    	return vector<State*>(m_states.begin(), m_states.end());
    }

    /**
     * Returns the total number of transitions of the automaton.
     */
    unsigned int Automaton::getTransitionsCount() {
        unsigned int count = 0;
        for (State* s : m_states) {
            count += s->getExitingTransitionsCount();
        }
        return count;
    }

    /**
     * Returns the alphabet of the automaton.
     * NOTE: This method does not necessarily return the entire alphabet on which the automaton was
     * defined in principle, because an automaton DOES NOT maintain a reference to such alphabet.
     * On the contrary, it computes the alphabet by analyzing all the labels present in all the transitions
     * of the automaton. For this reason, it can be expensive in terms of performance.
     */
    const Alphabet Automaton::getAlphabet() {
        Alphabet alphabet = Alphabet();
        for (State* s : m_states) {
            for (auto &trans: s->getExitingTransitionsRef()) {
            	auto iterator = std::find(alphabet.begin(), alphabet.end(), trans.first);
            	if (iterator == alphabet.end()) {
            		alphabet.push_back(trans.first);
            	}
            }
        }

        return alphabet;
    }

    /**
     * Inserts a transition between the two states marked with the label passed as a parameter.
     * The method works only if both states are part of the automaton, and in this case it returns TRUE.
     * Otherwise it returns FALSE.
     */
    bool Automaton::connectStates(State *from, State *to, string label) {
    	if (this->hasState(from) && this->hasState(to)) {
    		from->connectChild(label, to);
    		return true;
    	} else {
    		return false;
    	}
    }

    /**
     * Inserts a transition between the two states marked with the label passed as a parameter.
     * The method works only if both states are part of the automaton, and in this case it returns TRUE.
     * Otherwise it returns FALSE.
     */
    bool Automaton::connectStates(string from, string to, string label) {
    	return this->connectStates(getState(from), getState(to), label);
    }

    /**
     * Clone the automaton.
     * The method returns a pointer to the new automaton, which is a copy of the first.
     * The method relies on the clone method of the State class, which allows to copy a State object
     * but without messing up with the transitions.
     */
    Automaton* Automaton::clone() {
        // Create the new automaton
        Automaton* clone = new Automaton();

        // Create a map that will contain the correspondence between the states of the original automaton and the new ones
        map<State*, State*> correspondence = map<State*, State*>();

        // For each state of the automaton
        for (State* s : this->m_states) {
            // Clone the state
            State* new_state = s->clone();
            // Add the new state to the new automaton
            clone->addState(new_state);
            // Add the correspondence to the map
            correspondence[s] = new_state;
        }

        // For each state of the automaton
        for (State* s : this->m_states) {
            // Get the new state
            State* new_state = correspondence[s];

            // If the state is the initial state, set it as the initial state of the new automaton
            if (s == m_initial_state) {
                clone->setInitialState(new_state);
            }

            // For each transition of the state
            for (auto &pair: s->getExitingTransitions()) {
                // For each state reached by the transition
                for (State* child: pair.second) {
                    // Get the new state
                    State* new_child = correspondence[child];

                    // Add the transition to the new automaton
                    clone->connectStates(new_state, new_child, pair.first);
                }
            }
        }
    	return clone;
    }

    /** 
     * Equality operator for automata. 
     */
    bool Automaton::operator==(Automaton& other) {
        DEBUG_MARK_PHASE("Automata comparison") {

    	// Check if the automata have the same initial state
        if (*m_initial_state != *other.m_initial_state) {
        	DEBUG_LOG("The state %s è is different from %s", m_initial_state->getName().c_str(), other.m_initial_state->getName().c_str());
        	return false;
        }

        // Check if the automata have the same number of states
        if (m_states.size() != other.m_states.size()) {
        	DEBUG_LOG("The first automaton has %lu states, the second automaton has %lu states", m_states.size(), other.m_states.size());
        	return false;
        }

        // Check if the automata have the same states
        // For each state of the first automaton I check that it exists also in the other.
        for (auto state : m_states) {

            // Search for a state with the same name in the other automaton
        	State* sakename_state;
        	if ((sakename_state = other.getState(state->getName())) != NULL) {
        		DEBUG_LOG("In both automata there's a state with name \"%s\"", state->getName().c_str());

                // Check if the state has the same transitions to states with the same name (!)
        		if (!state->hasSameTransitionsNamesOf(sakename_state)) {
        			DEBUG_LOG("However, the states have not the same transitions");
        			return false;
        		}

        	} else {
        		DEBUG_LOG("In the secon automata, there's no state with name \"%s\", which is instead contained in the first automaton", state->getName().c_str());
        		return false;
        	}
        }

    	}
    	DEBUG_LOG("The automata are equal");
        return true;
    }

} /* namespace quicksc */
