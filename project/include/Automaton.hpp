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

	private:
		multiset<State*> m_states;
		State* m_initial_state;

        void removeReachableStates(State* s, set<State*> &states);

	public:
		Automaton();
		virtual ~Automaton();

        int size();
        void addState(State* s);
        bool removeState(State* s);
        set<State*> removeUnreachableStates();
        bool hasState(State* s);
        bool hasState(string name);
        bool isInitial(State* s);
        bool isInitial(string name);
        void setInitialState(State* s);
        void setInitialState(string name);
        State* getInitialState();
        State* getState(string name);
        const vector<State*> getStatesByName(string name);
        const list<State*> getStatesList();
        const vector<State*> getStatesVector();
		unsigned int getTransitionsCount();
        const Alphabet getAlphabet();
        bool connectStates(State *from, State *to, string label);
        bool connectStates(string from, string to, string label);
        Automaton* clone();

        bool operator==(Automaton& other);

	};


} /* namespace quicksc */

#endif /* INCLUDE_AUTOMATON_H_ */
