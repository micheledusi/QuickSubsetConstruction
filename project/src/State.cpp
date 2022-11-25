/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * State.cpp
 *
 * 
 * This source file contains the definition of the class State and its subclass ConstructedState.
 * A State is a node of a finite automaton, may it be a NFA or a DFA.
 * It's identified by a unique name and it has a set of transitions, linking it to other states.
 * Every transition is labeled with a symbol of the alphabet.
 */

#include "State.hpp"

#include <list>
#include <map>
#include <set>
#include <string>

#include "Alphabet.hpp"
//#define DEBUG_MODE
#include "Debug.hpp"

using std::list;
using std::map;
using std::set;
using std::string;

namespace quicksc {

	/**
	 * Constructor of the class State.
	 * Initializes the incoming and outgoing transitions sets as empty.
	 */
	State::State (string name, bool final) {
		this->m_exiting_transitions = map<string, set<State*>>();
		this->m_incoming_transitions = map<string, set<State*>>();
		m_final = final;
		m_name = name;
		DEBUG_LOG( "New object State created correctly" );
	}

	/**
	 * Destructor of the class State.
	 */
	State::~State () {
		DEBUG_LOG( "Destructing the State object \"%s\"", m_name.c_str() );
	}

	/**
	 * Private method.
	 * Returns a pointer to the current object, automatically
	 * casted to the correct type.
	 */
	State* State::getThis() const {
		return (State*) this;
	}

	/**
	 * Getter for the name of the state.
	 */
	string State::getName() const {
		return m_name;
	}

	/**
	 * Returns true if the state is final, false otherwise.
	 */
	bool State::isFinal() {
		return m_final;
	}

	/**
	 * Sets the state as FINAL or NON-FINAL,
	 * depending on the value passed as parameter.
	 */
	void State::setFinal(bool final) {
		m_final = final;
	}

	/**
	 * Links the current state to the state passed as parameter, with a transition
	 * labeled with the label "label" passed as parameter.
	 * 
	 * NOTE: if the transition already exists, it is *not* added again.
	 */
	void State::connectChild(string label, State* child)	{
		bool flag_new_insertion = false;
		if (this->m_exiting_transitions.count(label) == 0) {
			this->m_exiting_transitions.insert({label, set<State*>()});
			flag_new_insertion = true;
		}

		if (child->m_incoming_transitions.count(label) == 0) {
			child->m_incoming_transitions.insert({label, set<State*>()});
			flag_new_insertion = true;
		}

		if (flag_new_insertion || !this->hasExitingTransition(label, child)) {
			this->m_exiting_transitions[label].insert(child);
			child->m_incoming_transitions[label].insert(getThis());
		}
	}

	/**
	 * Disconnects two states, removing the l-transition that starts from this
	 * state and arrives in the one passed as parameter.
	 * 
	 * Precondition: it is assumed that such a transition exists.
	 */
	void State::disconnectChild(string label, State* child) {
		if (!this->hasExitingTransition(label)) {
			DEBUG_LOG("There are no exiting transitions with label %s", label.c_str());
			return;
		}
		// Search for the child to disconnect
		auto iterator = this->m_exiting_transitions[label].find(child);

		if (iterator != this->m_exiting_transitions[label].end()) {
			DEBUG_ASSERT_TRUE(this->hasExitingTransition(label, child));
			this->m_exiting_transitions[label].erase(iterator);
			DEBUG_ASSERT_FALSE(this->hasExitingTransition(label, child));
			child->m_incoming_transitions[label].erase(getThis());
		} else {
			DEBUG_LOG_FAIL("The child state %s has not been found for the label %s", child->getName().c_str(), label.c_str());
			return;
		}
	}

	/**
	 * Removes all the incoming and outgoing transitions
	 * from this state.
	 * The transitions are also updated on the nodes that
	 * were previously connected.
	 */
	void State::detachAllTransitions() {
		for (auto pair_it = m_exiting_transitions.begin(); pair_it != m_exiting_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto child_it = pair_it->second.begin(); child_it != pair_it->second.end(); ) {
				getThis()->disconnectChild(label, *(child_it++));
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}

		for (auto pair_it = m_incoming_transitions.begin(); pair_it != m_incoming_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto parent_iterator = pair_it->second.begin(); parent_iterator != pair_it->second.end(); ) {
				if (*parent_iterator != this->getThis()) {
					(*(parent_iterator++))->disconnectChild(label, getThis());
				}
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}
	}

	/**
	 * Returns the state reached by a transition with a specific label.
	 * Basically, it does the same operations of the "getChildren" method but it takes
	 * only the first node.
	 * If no child is found related to the label passed as argument, a null value is returned.
	 */
	State* State::getChild(string label) {
		if (this->hasExitingTransition(label)) {
			IF_DEBUG_ACTIVE(
				if (this->getChildrenRef(label).size() > 1) {
					DEBUG_LOG_ERROR("The DFA node \"%s\" has more than one child", this->getName().c_str());
				}
			)
			return *(this->getChildrenRef(label).begin());
		}
		else {
			return NULL;
		}
	}

	/**
	 * Protected method.
	 * Returns the set of states reached by all the transitions
	 * starting from this node and labeled with the label "l" passed as parameter.
	 * Basically, it returns the l-closure of this node.
	 *
	 * Note: this method will be inherited by the State class, since for an NFA
	 * it is possible to have more children labeled with the same label. In the case of a State
	 * state, however, it will be necessary to perform some checks to verify that
	 * there is only one child for each label.
	 */
	set<State*> State::getChildren(string label) {
		if (this->hasExitingTransition(label)) {
			return this->m_exiting_transitions[label];
		} else {
			return set<State*>();
		}
	}
	
	/**
	 * Returns the states that have a transition marked by a specific label
	 * that points to this state. In practice, all the "parent" states according to a certain
	 * label.
	 */
	set<State*> State::getParents(string label) {
		if (this->hasIncomingTransition(label)) {
			return this->m_incoming_transitions[label];
		} else {
			return set<State*>();
		}
	}

	/**
	 * Returns a reference to the vector of children according to a certain label.
	 * Attention: it is necessary to have children with such a label. If this is not the case, the behavior of this method is undefined.
	 */
	const set<State*>& State::getChildrenRef(string label) {
		DEBUG_LOG("Ho chiamato il metodo per i figli della label %s", label.c_str());
		for (State* child : this->m_exiting_transitions[label]) {
			DEBUG_LOG("----> Figlio: %s", child->getName().c_str());
		}

		DEBUG_LOG("Ho dei figli con label %s", label.c_str());
		return this->m_exiting_transitions[label];
	}

	/**
	 * Returns a reference to the vector of parents according to a certain label.
	 * Attention: it is necessary to have parents with such a label. If this is not the case, the behavior of this method is undefined.
	 */
	const set<State*>& State::getParentsRef(string label) {
		return this->m_incoming_transitions[label];
	}

	/**	
	 * Checks if the subject state has an OUTGOING transition
	 * marked with the label passed as a parameter.
	 */
	bool State::hasExitingTransition(string label)	{
		auto search = this->m_exiting_transitions.find(label);
		return (search != this->m_exiting_transitions.end()) && !(this->m_exiting_transitions[label].empty());
	}

	/**	
	 * Checks if the subject state has an OUTGOING transition that goes
	 * to the state "child" and that is marked with the label "label".
	 */
	bool State::hasExitingTransition(string label, State* child) {
		if (this->m_exiting_transitions.count(label)) {
			return (this->m_exiting_transitions[label].find(child) != this->m_exiting_transitions[label].end());
		} else {
			return false;
		}
	}

	/**	
	 * Checks if the subject state has an INCOMING transition
	 * marked with the label passed as a parameter.
	 */
	bool State::hasIncomingTransition(string label)	{
		auto search = this->m_incoming_transitions.find(label);
		return (search != this->m_incoming_transitions.end()) && !(this->m_incoming_transitions[label].empty());
	}

	/**	
	 * Checks if the subject state has an INCOMING transition
	 * that starts from the state "parent" and that is marked with the label "label".
	 */
	bool State::hasIncomingTransition(string label, State* parent) {
		if (this->m_incoming_transitions.count(label)) {
			return (this->m_incoming_transitions[label].find(parent) != this->m_incoming_transitions[label].end());
		} else {
			return false;
		}
	}

	/**	
	 * Returns the map of outgoing transitions from this state.
	 */
	map<string, set<State*>> State::getExitingTransitions() {
		return m_exiting_transitions;
	}

	/**	
	 * Returns the map of incoming transitions into this state.
	 */
	map<string, set<State*>> State::getIncomingTransitions() {
		return m_incoming_transitions;
	}

	/**
	 * Returns a reference to the map of transitions, that is, the address of the memory
	 * where the map is saved.
	 * Returning an address allows you to use this method as an lvalue in an assignment, for example.
	 */
	const map<string, set<State*>>& State::getExitingTransitionsRef() {
		return m_exiting_transitions;
	}

	/**	
	 * Returns a reference to the map of incoming transitions, that is, the address of the memory
	 * where the map is saved.
	 * Returning an address allows you to use this method as an lvalue in an assignment, for example.
	 */
	const map<string, set<State*>>& State::getIncomingTransitionsRef() {
		return m_incoming_transitions;
	}

	/**	
	 * Counts the outgoing transitions from the state.
	 * For each label, counts the amount of transitions referred to by that label outgoing
	 * from the current state.
	 */
	int State::getExitingTransitionsCount() {
		int count = 0;
		for (auto &pair: m_exiting_transitions) {
			count += pair.second.size();
		}
		return count;
	}

	/**
	 * Counts the incoming transitions into the state.
	 */
	int State::getIncomingTransitionsCount() {
		int count = 0;
		for (auto &pair: m_incoming_transitions) {
			count += pair.second.size();
		}
		return count;
	}

	/**
	 * Looks at all the outgoing transitions from the state "state".
	 * If it finds a transition that this state does not have,
	 * it adds it as its own transition.
	 * At the end of the execution, it is guaranteed that this state
	 * contains at least all the outgoing transitions from the state
	 * passed as a parameter.
	 */
	void State::copyExitingTransitionsOf(State* state) {
        for (auto &pair: state->getExitingTransitionsRef()) {
            string label = pair.first;
            for (State* child: pair.second) {
                if (!this->hasExitingTransition(label, child)) {
                    this->connectChild(label, child);
                }
            }
        }
    }

	/**	
	 * Looks at all the incoming transitions into the state "state".
	 * If it finds a transition that this state does not have,
	 * it adds it as its own transition.
	 * At the end of the execution, it is guaranteed that this state
	 * contains at least all the incoming transitions into the state
	 * passed as a parameter.
	 */
	void State::copyIncomingTransitionsOf(State* state) {
        for (auto &pair: state->getIncomingTransitionsRef()) {
            string label = pair.first;
            for (State* parent: pair.second) {
                if (!parent->hasExitingTransition(label, this->getThis())) {
                    parent->connectChild(label, this->getThis());
                }
            }
        }
    }

	/**	
	 * Copies all the incoming and outgoing transitions from the state "state"
	 * into this state.
	 * The already existing transitions are not duplicated.
	 */
	void State::copyAllTransitionsOf(State* state) {
        copyIncomingTransitionsOf(state);
        copyExitingTransitionsOf(state);
    }

	/**
	 * Checks if the state has the same transitions (incoming AND outgoing) as the state
	 * passed as a parameter.
	 * The transitions are compared by pointer; the states must therefore be
	 * actually the same.
	 */
	bool State::hasSameTransitionsOf(State* other_state) {
		if (this->m_exiting_transitions.size() != other_state->m_exiting_transitions.size()) {
			return false;
		}

		for (auto &pair: m_exiting_transitions) {
			string label = pair.first;
			set<State*> other_children = other_state->m_exiting_transitions[label];

			if (pair.second.size() != other_children.size()) {
				return false;
			}

			for (auto &child: pair.second) {
				if (other_children.count(child) <= 0) {
					return false;
				}
			}
		}

		if (this->m_incoming_transitions.size() != other_state->m_incoming_transitions.size()) {
			return false;
		}

		for (auto &pair: m_incoming_transitions) {
			string label = pair.first;
			set<State*> other_parents = other_state->m_incoming_transitions[label];

			if (pair.second.size() != other_parents.size()) {
				return false;
			}

			for (auto &parent: pair.second) {
				if (other_parents.count(parent) <= 0) {
					return false;
				}
			}
		}
		return true;
	}
	
	/** 
	 * Method that checks if the transitions present in the state correspond (only
	 * at the level of name) with the transitions of the state passed as a parameter.
	 * This method is used to compare two isomorphic automata, without there being
	 * states in common between them, but only identical states.
	 */
	bool State::hasSameTransitionsNamesOf(State* other_state) {
		if (this->getExitingTransitionsCount() != other_state->getExitingTransitionsCount()) {
			return false;
		}

		for (auto &pair : m_exiting_transitions) {
			string label = pair.first;
			set<State*> other_children = other_state->m_exiting_transitions[label];

			if (pair.second.size() != other_children.size()) {
				return false;
			}

			for (auto &child : pair.second) {
				bool found_flag = false;
				for (auto other_child : other_children) {
					if (*child == *other_child) {
						found_flag = true;
						break;
					}
				}
				if (!found_flag) {
					return false;
				}
			}
		}
		/** NOTE: 
		 * In theory, if the two automata have been well formatted according to the methods offered by the model classes,
		 * the incoming transitions will always correspond to the outgoing ones. Therefore it is not necessary to check both. */

		return true;
	}

	/**	
	 * Returns the distance of this state.
	 */
    unsigned int State::getDistance() {
        return m_distance;
    }

	/**
	 * Sets the distance of this state.
	 */
    void State::setDistance(unsigned int distance) {
        m_distance = distance;
    }

	/**
	 * Sets the values of the distances starting from the current state as the root, considering each transition as a unit increment on the distance.
	 * In fact, the assignment of distances is not implemented with a recursive algorithm, but a queue is used that allows 
	 * a traversal of the automaton of type breadth-first (breadth-first traversal).
	 * 
	 * NOTE: this method is called just after the construction of an automaton, to define the distances of all the states;
	 * it is therefore important that all the states belong to the same automaton, and that all have a distance not yet initialized (ie equal to the default value).
	 */
    void State::initDistancesRecursively(int root_distance) {
        this->setDistance(root_distance);
        list<State*> updated_list;
        updated_list.push_back(this->getThis());

        while (updated_list.size() > 0) {	

            State* current_state = updated_list.front();
            updated_list.pop_front();

            for (auto &trans: current_state->getExitingTransitionsRef()) {
                for (State* child : trans.second) {
                    if (child->getDistance() == DEFAULT_VOID_DISTANCE) {
                        child->setDistance(current_state->getDistance() + 1);
                        updated_list.push_back(child);
                    }
                }
            }
        }
    }

    /**
     * Restituisce la minima distanza fra tutte le distanze dei genitori.
     */
	/**
	 * Returns the minimum distance between all the distances of the parents.
	 */
    int State::getMinimumParentsDistance() {
    	int minimum = DEFAULT_VOID_DISTANCE;
    	for (auto &pair : this->getIncomingTransitionsRef()) {
    		for (State* parent : pair.second) {
    			if (parent->m_distance < minimum) {
    				minimum = parent->m_distance;
    			}
    		}
    	}
    	return minimum;
    }

	/**
	 * Returns a string with all the information related to the state.
	 */
	string State::toString() const {
		string result = "";

		// Name of the state
		result += "\033[33;1m" + getThis()->getName() + "\033[0m";

		// Distance of the state
		result += " (dist = " + std::to_string(m_distance) + ")";

		// If the state is final, it is marked with a TAG
		if (getThis()->isFinal()) {
			result += " [FINAL]";
		}

		result += "\n\t" + (std::to_string(getThis()->getExitingTransitionsCount())) + " exiting transitions:\n";
		if (!this->m_exiting_transitions.empty()) {
			// For all the exiting transitions
			for (auto &pair: m_exiting_transitions) {
				string label = pair.first;
				for (State* state: pair.second) {
					result += "\t━━┥" + SHOW(label) + "┝━━▶ " + state->getName() + "\n";
//					result += "\t━━┥" + SHOW(label) + "┝━━▶ " + state->getName() + "\033[35m[id = " + (std::to_string((long int)state)) + "]\033[0m" + "\n";
				}
			}
		}
		return result;
	};

	/**
	 * Defines a "<" comparison operator, based on the comparison of the names.
	 */
	bool State::operator<(const State &other) const	{
		return getThis()->getName() < other.getName();
	}

	/**	
	 * Defines an equality operator "==", based on the equality of the names.
	 */
	bool State::operator==(const State &other) const {
		return getThis()->getName() == other.getName();
	}

	/**
	 * Defines an inequality operator "!=", based on the comparison of the names.
	 */
	bool State::operator!=(const State &other) const {
			return getThis()->getName() != other.getName();
	}


///////////////////////////////////////////////////////////////////

	/**
	 * Static method.
	 * Creates the name of the state by concatenating the names of the states of the extension.
	 * This method is used automatically to determine the name of a ConstructedState object
	 * every time its extension is assigned or modified.
	 */
	string ConstructedState::createNameFromExtension(const Extension &ext) {
		if (ext.empty()) {
			return EMPTY_EXTENSION_NAME;
		}

		string name = "{";

		// For each state in the extension
		for (State* s: ext) {
			name += s->getName() + ',';
		}
		// Remove the last comma
		name.pop_back();
		name += "}";

		return name;
	}

	/**
	 * Static method.
	 * Subtracts from the first extension the states of the second extension.
	 * Considering the extensions as sets, it performs a set difference and returns the result.
	 */
	Extension ConstructedState::subtractExtensions(const Extension &ext1, const Extension &ext2) {
		Extension result = set<State*, State::Comparator>();

		for (State* s : ext1) {
			if (ext2.count(s) == 0) {
				result.insert(s);
			}
		}

		return result;
	}

	/**
	 * Static method.
	 * Computes the epsilon closure of an extension, that is, of a set of states (usually a set of states of an NFA).
	 */
	Extension ConstructedState::computeEpsilonClosure(const Extension &ext) {
		Extension result = set<State*, State::Comparator>(ext);
		list<State*> queue = list<State*>();
		for (State* s : ext) {
			queue.push_back(s);
		}

		while (!queue.empty()) {
			// Extract the state to process
			State* current = queue.front();
			queue.pop_front();
			// Compute the states reachable through epsilon transitions
			set<State*> closure = current->getChildren(EPSILON);
			// For each state in the closure
			for (State* epsilon_child : closure) {
				// We add the epsilon child to the closure, if it's not already present
				if (result.insert(epsilon_child).second) {
					// Then we add it to the queue, to process it later
					queue.push_back(epsilon_child);
				}
			}
		}

		return result;
	}

	/**
	 * Static method.
	 * Computes the epsilon closure of a single state.
	 */
	Extension ConstructedState::computeEpsilonClosure(State* state) {
		Extension result = set<State*, State::Comparator>();
		result.insert(state);
		list<State*> queue = list<State*>();
		queue.push_back(state);

		while (!queue.empty()) {
			State* current = queue.front();
			queue.pop_front();
			set<State*> closure = current->getChildren(EPSILON);
			for (State* epsilon_child : closure) {
				if (result.insert(epsilon_child).second) {
					queue.push_back(epsilon_child);
				}
			}
		}

		return result;
	}

	/**
	 * Static method.
	 * Returns "true" if and only if there is at least one NFA state inside
	 * the extension marked as a final state.
	 */
	bool ConstructedState::hasFinalStates(const Extension &ext) {
		for (State* state_nfa : ext) {
			if (state_nfa->isFinal()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Constructor of the class ConstructedState.
	 * Assigns to the state the extension passed as parameter and sets the name of the state.
	 * Before the construction the constructor of the parent class "State" is called using
	 * two static methods that operate on the extension to obtain the name of the state
	 * and the boolean value representing whether the state is final or not.
	 */
	ConstructedState::ConstructedState(Extension &extension)
		: State(ConstructedState::createNameFromExtension(extension), ConstructedState::hasFinalStates(extension)) {

		this->m_extension = extension;
	}

	/**
	 * Destructor of the class ConstructedState.
	 */
	ConstructedState::~ConstructedState() {
		this->m_extension.clear();
	}

	/**
	 * Sets the state with the marking value passed as parameter.
	 */
	void ConstructedState::setMarked(bool mark) {
		this->m_mark = mark;
	}

	/**
	 * Returns true if the state is marked.
	 */
	bool ConstructedState::isMarked() {
		return this->m_mark;
	}

	/**
	 * Checks if the state has a specific extension passed as parameter.
	 * The comparison is performed by comparing the string containing all the names of the extension.
	 */
	bool ConstructedState::hasExtension(const Extension &ext) {
		return (getName() == createNameFromExtension(ext));
	}

	/**
	 * Returns the extension of the state, that is, the set of State
	 * from which this state was created.
	 */
	const Extension& ConstructedState::getExtension() {
		return m_extension;
	}

	/**
	 * Returns all the labels of the transitions outgoing from the states of the extension.
	 */
	set<string>& ConstructedState::getLabelsExitingFromExtension() {
		set<string> *labels = new set<string>;
		DEBUG_ASSERT_TRUE(labels->size() == 0);

		for (State* member : m_extension) {
			DEBUG_LOG("For the state of extension \"%s\"", member->getName().c_str());
			for (auto &pair: member->getExitingTransitionsRef()) {
				DEBUG_LOG("Number of transitions marked with label %s: %lu", pair.first.c_str(), pair.second.size());
				if (pair.second.size() > 0) {
					DEBUG_LOG("Adding the label \"%s\"", pair.first.c_str());
					labels->insert(pair.first);
				}
			}
		}
		DEBUG_LOG("Final lenght of the label set: %lu", labels->size());
		return *labels;
	}

	/**
	 * Returns the l-closure given the label string:
	 * for all the states of the extension computes the l-closure,
	 * then computes the epsilon-closure of the states reached.
	 * It is assumed, therefore, that the extension present in the state is always epsilon-closed.
	 */
	Extension ConstructedState::computeLClosureOfExtension(string label) {
		Extension l_closure;
		for (State* member : this->m_extension) {
			for (State* child : member->getChildren(label)) {
				l_closure.insert(child);
			}
		}
		return ConstructedState::computeEpsilonClosure(l_closure);
	}

	/**
	 * Returns the ell-closure of the state, given the label.
	 * In other words, it computes:
	 * - the set of all the ell-children of this state
	 * - the epsilon closure of the ell-children
	 * 
	 * NOTE: This method assumes that the state does not have epsilon-transitions outgoing. This is because it is used in algorithms (QSC) that should guarantee this condition.
	 */
	Extension ConstructedState::computeLClosure(string label) {
		Extension l_closure;
		for (State* child : this->getChildren(label)) {
			l_closure.insert(child);
		}
		return ConstructedState::computeEpsilonClosure(l_closure);
	}

	/**
	 * Replaces the extension of this state with another one.
	 * 
	 * NOTE: this method also causes the change of the name of the state, based on the states of the NFA that are contained in the new extension.
	 */
	void ConstructedState::replaceExtensionWith(Extension &new_ext) {
		this->m_extension = new_ext;
		this->m_name = createNameFromExtension(m_extension);
		this->m_final = hasFinalStates(m_extension);
	}

	/**
	 * Checks if the state is "empty", that is, if its extension is empty.
	 */
	bool ConstructedState::isExtensionEmpty() {
		return m_extension.empty();
	}

	/**
	 * Checks if the state is safe with respect to a given singularity, represented by:
	 * - A state
	 * - A label
	 *
	 * A state is <safe> if and only if one of the following conditions is verified:
	 * - It is the initial state of the automaton (that is, its distance is zero).
	 * - There exists an incoming transition in the state that satisfies both:
	 * 		- the transition must be different from the transition represented by the singularity
	 *		- the state of origin must have a distance less than or equal to the state of the singularity
	 *
	 * ATTENTION: It is assumed that the state BELONGS to the closure of the singularity.
	 */
	bool ConstructedState::isSafe(State* singularity_state, string singularity_label) {
		if (this->getDistance() == 0) { // If the state is the initial state, it is safe
			return true;
		}

    	for (auto &pair : this->getIncomingTransitionsRef()) {
			string label = pair.first;

			if (label == singularity_label) {
				for (State* parent : pair.second) {
					if (parent != singularity_state
						&& parent->getDistance() <= singularity_state->getDistance()) {
						return true;
					}
				}
			}
			else {
	    		for (State* parent : pair.second) {
	    			if (parent->getDistance() <= singularity_state->getDistance()) {
						return true;
	    			}
	    		}
			}
    	}
    	return false;
	}

	/**
	 * A state is <unsafe> if and only if it is not <safe>.
	 */
	bool ConstructedState::isUnsafe(State* singularity_state, string singularity_label) {
		return !this->isSafe(singularity_state, singularity_label);
	}

	/**
	 * This method clones the state, that is, it creates a new state with the same name, distance, finality.
	 * 
	 * ATTENTION: it does NOT clone the transitions.
	 * Cloning them would require the knowledge of all the automaton states, which is not available.
	 */
	State* State::clone() {
		State* clone = new State(this->m_name, this->m_final);
		clone->setDistance(this->m_distance);
		return clone;
	}

	/**
	 * This method clones the state, that is, it creates a new state with the same name, distance, finality.
	 * 
	 * ATTENTION: it does NOT clone the transitions.
	 * ATTENTION: it does NOT clone the extension. The extension is just COPIED, that is, the same states are referenced.
	*/
	ConstructedState* ConstructedState::clone() {
		ConstructedState* clone = new ConstructedState(this->m_extension);
		clone->setFinal(this->m_final);
		clone->setDistance(this->m_distance);
		return clone;
	}

}
