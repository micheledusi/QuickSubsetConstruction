/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * State.hpp
 *
 *
 * Header file containing the definition of the State classes.
 * This is a hierarchy of classes that represent the states of an automaton:
 * - State, which is the base class for a generic state in a generic automaton, that can be either a DFA or an NFA.
 * 		This class contains only the essential attributes for a state to be used in an automaton.
 * - BidirectionalState, which is a class derived from State, and it is used for a generic state in a generic automaton
 * 		that can be either a DFA or an NFA. This class contains both exiting and entering transitions, and it is therefore
 * 		faster in searching and checking operations.
 * - ConstructedState, which is a class derived from State, and it is used for a state in a DFA obtained by
 * 		determinization. It contains only exiting transitions.
 * - BidirectionalConstructedState, which is a class derived from BidirectionalState and ConstructedState, and it is used
 * 		for a state in a DFA obtained by determinization. It contains both exiting and entering transitions.
 * 
 * In this project, the classes will be used as follows:
 * - State will be used for the input NFA of the determinization algorithm "Subset Construction".
 * - BidirectionalState will be used for input NFA of the determinization algorithm "Quick Subset Construction".
 * - ConstructedState will be used for the output DFA of the determinization algorithm "Subset Construction".
 * - BidirectionalConstructedState will be used for the output DFA of the determinization algorithm "Quick Subset Construction".
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

#define DEFAULT_VOID_LEVEL 1U<<30

#define EMPTY_EXTENSION_NAME "∅"

namespace quicksc {

	/**
	 * Base class for a generic state in a FA (Finite Automaton).
	 * 
	 * A state is characterized by:
	 * - a name, which is used to identify the state in the automaton.
	 * - a flag that indicates whether the state is final or not.
	 * - a set of exiting transitions, each of which is labeled with a symbol.
	 * 
	 * This way, a state implements a graph node in implicit form, while the transitions are the edges.
	 * The topology of the graph is defined by the transition, which are stored distributed in the states.
	 */
	class State {

		public:
		State(const string& name, bool final = false);	
		State(const State& other);
		virtual ~State();

		virtual constexpr bool isBidirectional() const {return false;}
		virtual constexpr bool isConstructed() const {return false;}

		const string& getName() const;
		bool isFinal() const;
		void setFinal(bool final);

		State* getChild(const string& label) const;
		set<State*> getChildren(const string& label) const;
		const set<State*>& getChildrenRef(const string& label) const;

		bool hasExitingTransition(const string& label) const;
		bool hasExitingTransition(const string& label, State* child) const;
		virtual bool hasSameTransitionsOf(State* other_state) const;
		virtual bool hasSameTransitionsNamesOf(State* other_state) const;

		map<string, set<State*>> getExitingTransitions() const;
		const map<string, set<State*>>& getExitingTransitionsRef() const;
		int getExitingTransitionsCount() const;

		virtual bool connectChild(const string& label, State* child);
		virtual bool disconnectChild(const string& label, State* child);
		
		virtual void detachAllTransitions();
		void copyExitingTransitionsOf(State* other_state);
		virtual void copyAllTransitionsOf(State* other_state);

		virtual string toString() const;
		virtual State* clone();

		bool operator<(const State &other) const;
		bool operator==(const State &other) const;
		bool operator!=(const State &other) const;

		/**
		 * Comparator for the set of states.
		 * It performs the comparison based on the name of the states.
		*/
		struct Comparator {
			bool operator() (const State* lhs, const State* rhs) const {
				return lhs->getName() < rhs->getName();
			}
		};

		private:
		string m_name = "";						// Name of the state
		bool m_final = false;					// This flag is true if the state is final, false otherwise
	
		protected:
		map<string, set<State*>> m_exiting_transitions;		// Map of exiting transitions, each of which is labeled with a symbol and points to a set of children states
		void setName(const string& name);

	};

	class BidirectionalState : virtual public State {

		public:
		BidirectionalState(const string& name, bool final = false);
		BidirectionalState(const State& other);
		~BidirectionalState() = default;
		
		virtual constexpr bool isBidirectional() const {return true;}
		virtual constexpr bool isConstructed() const {return false;}

		BidirectionalState* getParent(const string& label) const;
		set<BidirectionalState*> getParents(const string& label) const;
		const set<BidirectionalState*>& getParentsRef(const string& label) const;

		bool hasIncomingTransition(const string& label) const;
		bool hasIncomingTransition(const string& label, BidirectionalState* child) const;
		virtual bool hasSameTransitionsOf(State* other_state) const override;

		map<string, set<BidirectionalState*>> getIncomingTransitions() const;
		const map<string, set<BidirectionalState*>>& getIncomingTransitionsRef() const;
		int getIncomingTransitionsCount() const;

		virtual bool connectChild(const string& label, State* child) override;
		virtual bool disconnectChild(const string& label, State* child) override;
		
		virtual void detachAllTransitions();
		void copyIncomingTransitionsOf(BidirectionalState* other_state);
		virtual void copyAllTransitionsOf(State* other_state) override;

		unsigned int getLevel() const;
		void setLevel(unsigned int level);
		void initLevelsRecursively(int root_level);
		int getMinimumParentsLevel();

		virtual BidirectionalState* clone() override;
		
		protected:
		map<string, set<BidirectionalState*>> m_incoming_transitions;	// Map of entering transitions, each of which is labeled with a symbol and points to a set of parent states
		unsigned int m_level = DEFAULT_VOID_LEVEL;	// Level of the state from the initial state

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
	class ConstructedState : virtual public State {

		public:
		static string createNameFromExtension(const Extension &ext);
		static Extension subtractExtensions(const Extension &ext1, const Extension &ext2);
		static Extension computeEpsilonClosure(const Extension &ext);
		static Extension computeEpsilonClosure(State* state);
		static bool hasFinalStates(const Extension &ext);

		ConstructedState(Extension &extension);
		~ConstructedState() = default;
		
		virtual constexpr bool isBidirectional() const {return false;}
		virtual constexpr bool isConstructed() const {return true;}

		virtual ConstructedState* clone() override;

		void setMarked(bool mark);
		bool isMarked() const;

		bool hasExtension(const Extension &ext) const;
		const Extension& getExtension() const;
		set<string>& getLabelsExitingFromExtension() const;
		Extension computeLClosureOfExtension(const string& label) const;
		Extension computeLClosure(const string& label) const;
		bool isExtensionEmpty() const;

		void replaceExtensionWith(Extension &new_ext);

		private:
		Extension m_extension; // States of the corresponding NFA
		bool m_mark = false; // Useful mark

	};

	class BidirectionalConstructedState : virtual public BidirectionalState, virtual public ConstructedState {

		public:
		BidirectionalConstructedState(Extension &extension);
		~BidirectionalConstructedState() = default;
	
		virtual constexpr bool isBidirectional() const {return true;}
		virtual constexpr bool isConstructed() const {return true;}
		
		bool isSafe(BidirectionalConstructedState* singularity_state, const string& singularity_label) const;
		bool isUnsafe(BidirectionalConstructedState* singularity_state, const string& singularity_label) const;

		virtual BidirectionalConstructedState* clone() override;
	};
		
}

#endif /* INCLUDE_STATE_H_ */
