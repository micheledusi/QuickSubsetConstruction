/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * State.hpp
 *
 *
 * Header file containing the definition of the State class.
 * The State class is the base class for all the states of the automaton, and
 * it can be specialized in ConstructedState, a state deriving from a determinization algorithm.
 */

#ifndef INCLUDE_STATE_H_
#define INCLUDE_STATE_H_

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <cstdbool>

using std::string;
using std::map;
using std::set;

#define DEFAULT_VOID_DISTANCE 1U<<30

#define EMPTY_EXTENSION_NAME "∅"

namespace quicksc {

	/**
	 * Base class for a generic state in a generic automaton, that can be either a DFA or an NFA.
	 * For DFAs obtained by determinization, the class ConstructedState is used instead.
	 * 
	 * A state is characterized by a set of transitions, each of which is labeled with a symbol.
	 * It has also a name, which is used to identify the state in the automaton.
	 */
	class State {

    private:
        map<string, set<State*>> m_exiting_transitions;		// Map of exiting transitions, each of which is labeled with a symbol and points to a set of children states
        map<string, set<State*>> m_incoming_transitions;	// Map of entering transitions, each of which is labeled with a symbol and points to a set of parent states

        State* getThis() const;

	protected:
		string m_name = "";									// Name of the state
		bool m_final = false;								// This flag is true if the state is final, false otherwise
		unsigned int m_distance = DEFAULT_VOID_DISTANCE;	// Distance of the state from the initial state

    public:
		State(string name, bool final = false);				// Constructor
        ~State();											// Destructor

        string getName() const;
        bool isFinal();
        void setFinal(bool final);
		bool connectChild(string label, State* child);
		void disconnectChild(string label, State* child);
		void detachAllTransitions();
		State* getChild(string label);
		set<State*> getChildren(string label);
		set<State*> getParents(string label);
		const set<State*>& getChildrenRef(string label);
		const set<State*>& getParentsRef(string label);

		bool hasExitingTransition(string label);
		bool hasExitingTransition(string label, State* child);
		bool hasIncomingTransition(string label);
		bool hasIncomingTransition(string label, State* child);
		map<string, set<State*>> getExitingTransitions();
		map<string, set<State*>> getIncomingTransitions();
		const map<string, set<State*>>& getExitingTransitionsRef();
		const map<string, set<State*>>& getIncomingTransitionsRef();
		int getExitingTransitionsCount();
		int getIncomingTransitionsCount();
		void copyExitingTransitionsOf(State* other_state);
		void copyIncomingTransitionsOf(State* other_state);
		void copyAllTransitionsOf(State* other_state);

		bool hasSameTransitionsOf(State* other_state);
		bool hasSameTransitionsNamesOf(State* other_state);

		unsigned int getDistance();
		void setDistance(unsigned int distance);
		void initDistancesRecursively(int root_distance);
	    int getMinimumParentsDistance();

		string toString() const;

		virtual State* clone();

		bool operator<(const State &other) const;
		bool operator==(const State &other) const;
		bool operator!=(const State &other) const;
//		int compareNames(const S &other) const;

		struct Comparator {
            bool operator() (const State* lhs, const State* rhs) const {
                return lhs->getName() < rhs->getName();
            }
        };

    };

	/**
	 * Extension of the State class, used for states obtained by determinization (ConstructedState).
	 * The extension of a state is a set of states of the original automaton, usually states from a NFA, 
	 * obtained by an algorithm of determinization.
	 * 
	 * In general, it's possible to use an Extension as a generic (sorted) set of states, without duplications.
	 */
	using Extension = set<State*, State::Comparator>;

	/**
	 * This class is derived from the State class, and it's used for states obtained by the determinization process.
	 * Different from the State class, a ConstructedState is characterized by an extension, which is a set of states.
	 * The extension holds the refereces to the states of the original automaton, from which the ConstructedState is derived.
	 * The references helps the determinization algorithm to build the new automaton.
	 */
	class ConstructedState : public State {

	private:
		Extension m_extension;			// States of the corresponding NFA
		bool m_mark = false;

	public:
		static string createNameFromExtension(const Extension &ext);
		static Extension subtractExtensions(const Extension &ext1, const Extension &ext2);
		static Extension computeEpsilonClosure(const Extension &ext);
		static Extension computeEpsilonClosure(State* state);
		static bool hasFinalStates(const Extension &ext);

		ConstructedState(Extension &extension);
		virtual ~ConstructedState();

		void setMarked(bool mark);
		bool isMarked();
		bool hasExtension(const Extension &ext);
		const Extension& getExtension();
		set<string>& getLabelsExitingFromExtension();
		Extension computeLClosureOfExtension(string label);
		Extension computeLClosure(string label);
		void replaceExtensionWith(Extension &new_ext);
		bool isExtensionEmpty();
		ConstructedState* clone() override;

		//bool isSafe(Singularity* singularity);
		bool isSafe(State* singularity_state, string singularity_label);
		//bool isUnsafe(Singularity* singularity);
		bool isUnsafe(State* singularity_state, string singularity_label);

	};

}

#endif /* INCLUDE_STATE_H_ */
