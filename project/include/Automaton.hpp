/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Automaton.hpp
 *
 *
 * This header file contains the definition of the Automaton class.
 * An automaton is - by theory - a set of states, a set of transitions between them (labeled with symbols of a given alphabet), a starting state and a set of final states.
 * The Automaton class is used to represent a generic automaton, and must be specialized to represent a NFA or a DFA.
 *
 */

#ifndef INCLUDE_AUTOMATON_H_
#define INCLUDE_AUTOMATON_H_

#include <vector>
#include <list>

#include "Alphabet.hpp"
#include "State.hpp"

namespace quicksc {

	using std::vector;
	using std::list;
	using std::multiset;

	class Automaton {

	    public:
		Automaton();
		virtual ~Automaton();

        int size() const;
        bool hasState(State* s) const;
        bool hasState(string name) const;
        bool isInitial(State* s) const;
        bool isInitial(string name) const;
        State* getInitialState() const;
        State* getState(string name) const;
        const vector<State*> getStatesByName(string name) const;
        const list<State*> getStatesList() const;
        const vector<State*> getStatesVector() const;
		unsigned int getTransitionsCount() const;
        const Alphabet getAlphabet() const;

        void addState(State* s);
        bool removeState(State* s);
        void setInitialState(State* s);
        void setInitialState(string name);
        bool connectStates(State *from, State *to, string label);
        bool connectStates(string from, string to, string label);
        set<State*> removeUnreachableStates();
        void recomputeAllLevels();

        Automaton* clone();
        template <class S> Automaton* cloneConvertingStates();

        bool operator==(Automaton& other);
        
	    private:
		multiset<State*> m_states;
		State* m_initial_state;

        void removeReachableStates(State* s, set<State*> &states);

	};


} /* namespace quicksc */




// TEMPLATE IMPLEMENTATIONS

namespace quicksc {

    /**
     * Clone the automaton, converting each state in the requested type.
     */
    template <class S> Automaton* Automaton::cloneConvertingStates() {
        // Create the new automaton
        Automaton* clone = new Automaton();

        // Create a map that will contain the correspondence between the states of the original automaton and the new ones
        map<State*, S*> correspondence = map<State*, S*>();

        // For each state of the automaton
        for (State* s : this->m_states) {
            // Copying the state, without transitions
            S* new_state = new S(*s);
            // Add the new state to the new automaton
            clone->addState(new_state);
            // Add the correspondence to the map
            correspondence[s] = new_state;
        }

        // For each state of the automaton
        for (State* s : this->m_states) {
            // Get the new state
            S* new_state = correspondence[s];

            // For each transition of the state
            for (auto &pair: s->getExitingTransitions()) {
                // For each state reached by the transition
                for (State* child: pair.second) {
                    // Get the new state
                    S* new_child = correspondence[child];

                    // Add the transition to the new automaton
                    clone->connectStates(new_state, new_child, pair.first);
                }
            }
        }

        // Setting the initial state
        State* initial_state = this->getInitialState();
        S* new_initial_state = correspondence[initial_state];
        clone->setInitialState(new_initial_state);

        return clone;

    }

}

#endif /* INCLUDE_AUTOMATON_H_ */
