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
	 * Initializes the outgoing transitions sets as empty.
	 */
	State::State(const string& name, bool final) 
		: m_name(name)
		, m_final(final)
		, m_exiting_transitions(map<string, set<State*>>()) 
	{
		DEBUG_LOG( "New object State created correctly" );
	}

	/** 
	 * Copy constructor of the class State.
	 * NOTE: It initializes the outgoing transitions sets as empty, 
	 * so it does not copy the transitions of the other state.
	*/
	State::State(const State& other) 
		: m_name(other.m_name)
		, m_final(other.m_final)
		, m_exiting_transitions(map<string, set<State*>>()) 
	{
		DEBUG_LOG( "New object State created correctly by copying from other state" );
	}

	/**
	 * Constructor of the class BidirectionalState.
	*/
	BidirectionalState::BidirectionalState(const string& name, bool final) 
		: State(name, final)
		, m_incoming_transitions(map<string, set<BidirectionalState*>>()) 
	{
		DEBUG_LOG( "New object BidirectionalState created correctly" );
	}

	/**
	 * Copy constructor of the class BidirectionalState.
	 * NOTE: It initializes the outgoing and incoming transitions sets as empty, 
	 * so it does not copy the transitions of the other state.
	*/
	BidirectionalState::BidirectionalState(const State& other)
		: State(other)
		, m_incoming_transitions(map<string, set<BidirectionalState*>>())
	{
		if (other.isBidirectional()) {
			const BidirectionalState* other_bidirectional = dynamic_cast<const BidirectionalState*>(&other);
			m_level = other_bidirectional->m_level;
		}
		DEBUG_LOG( "New object BidirectionalState created correctly by copying from other state" );
	}

	/**
	 * Constructor of the class ConstructedState.
	 * Assigns to the state the extension passed as parameter and sets the name of the state.
	 * Before the construction the constructor of the parent class "State" is called using
	 * two static methods that operate on the extension to obtain the name of the state
	 * and the boolean value representing whether the state is final or not.
	 */
	ConstructedState::ConstructedState(Extension &extension) 
		: State(
			ConstructedState::createNameFromExtension(extension),
			ConstructedState::hasFinalStates(extension))
		, m_extension(extension)
	{
		DEBUG_LOG( "New object ConstructedState created correctly" );
	}

	BidirectionalConstructedState::BidirectionalConstructedState(Extension &extension) 
		: State(
			ConstructedState::createNameFromExtension(extension),
			ConstructedState::hasFinalStates(extension))
		, BidirectionalState(
			ConstructedState::createNameFromExtension(extension), 
			ConstructedState::hasFinalStates(extension))
		, ConstructedState(extension) 
	{
		DEBUG_LOG( "New object BidirectionalConstructedState created correctly" );
	}

	/**
	 * Destructor of the class State.
	 */
	State::~State() {
		DEBUG_LOG( "Destructing the State object \"%s\"", m_name.c_str() );
	}

	/**
	 * Getter for the name of the state.
	 */
	const string& State::getName() const {
		return m_name;
	}

	/**
	 * Setter for the name of the state.
	 * PROTECTED METHOD: it can be called only by the subclasses
	*/
	void State::setName(const string& name) {
		m_name = name;
	}

	/**
	 * Returns true if the state is final, false otherwise.
	 */
	bool State::isFinal() const {
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
	 * Returns the state reached by a transition with a specific label.
	 * Basically, it does the same operations of the "getChildren" method 
	 * but it takes ONLY THE FIRST NODE.
	 * If no child is found related to the label passed as argument, a null value is returned.
	 */
	State* State::getChild(const string& label) const {
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
	 * Returns the set of states reached by all the transitions
	 * starting from this node and labeled with the label "l" passed as parameter.
	 * Basically, it returns the l-closure of this node.
	 */
	set<State*> State::getChildren(const string& label) const {
		if (this->hasExitingTransition(label)) {
			auto it = this->m_exiting_transitions.find(label);
			if (it == this->m_exiting_transitions.end()) {
				return set<State*>();
			}
			else {
				return it->second;
			}
		} else {
			return set<State*>();
		}
	}

	/**
	 * Returns a reference to the vector of children according to a certain label.
	 * Attention: it is necessary to have children with such a label. If this is not the case, the behavior of this method is undefined.
	 */
	const set<State*>& State::getChildrenRef(const string& label) const {
		DEBUG_LOG("Calling method \"getChildrenRef\" on label %s", label.c_str());
		for (State* child : this->getExitingTransitions()[label]) {
			DEBUG_LOG("----> Child: %s", child->getName().c_str());
		}

		DEBUG_LOG("I have children with label %s", label.c_str());
		return this->getExitingTransitions()[label];
	}
	
	/**
	 * Returns the state "parent" of this, according to a certain label.
	 * It returns ONLY THE FIRST PARENT FOUND, with no criteria of choice.
	*/
	BidirectionalState* BidirectionalState::getParent(const string& label) const {
		if (this->hasIncomingTransition(label)) {
			IF_DEBUG_ACTIVE(
				if (this->getParentsRef(label).size() > 1) {
					DEBUG_LOG_ERROR("The DFA node \"%s\" has more than one child", this->getName().c_str());
				}
			)
			return *(this->getParentsRef(label).begin());
		}
		else {
			return NULL;
		}
	}

	/**
	 * Returns the states that have a transition marked by a specific label
	 * that points to this state. In practice, all the "parent" states according to a certain
	 * label.
	 */
	set<BidirectionalState*> BidirectionalState::getParents(const string& label) const {
		if (this->hasIncomingTransition(label)) {
			auto it = this->m_incoming_transitions.find(label);
			if (it == this->m_incoming_transitions.end()) {
				return set<BidirectionalState*>();
			}
			else {
				return it->second;
			}
		} else {
			return set<BidirectionalState*>();
		}
	}

	/**
	 * Returns a reference to the vector of parents according to a certain label.
	 * Attention: it is necessary to have parents with such a label. If this is not the case, the behavior of this method is undefined.
	 */
	const set<BidirectionalState*>& BidirectionalState::getParentsRef(const string& label) const {
		auto it = this->m_incoming_transitions.find(label);
		if (it == this->m_incoming_transitions.end()) {
			throw std::invalid_argument("The state " + this->getName() + " has no parents with label " + label);
			// return set<BidirectionalState*>();
		}
		else {
			return it->second;
		}
	}

	/**	
	 * Checks if the subject state has an OUTGOING transition
	 * marked with the label passed as a parameter.
	 */
	bool State::hasExitingTransition(const string& label) const {
		auto it_label = this->m_exiting_transitions.find(label);
		return (it_label != this->m_exiting_transitions.end()) && !(it_label->second.empty());
	}

	/**	
	 * Checks if the subject state has an OUTGOING transition that goes
	 * to the state "child" and that is marked with the label "label".
	 */
	bool State::hasExitingTransition(const string& label, State* child) const {
		auto it_label = this->m_exiting_transitions.find(label);
		if (it_label == this->m_exiting_transitions.end()) {
			return false;
		}
		// Otherwise, the label exists
		return (it_label->second.find(child) != it_label->second.end());
	}

	/**	
	 * Checks if the subject state has an INCOMING transition
	 * marked with the label passed as a parameter.
	 */
	bool BidirectionalState::hasIncomingTransition(const string& label) const {
		auto it_label = this->m_incoming_transitions.find(label);
		return (it_label != this->m_incoming_transitions.end()) && !(it_label->second.empty());
	}

	/**	
	 * Checks if the subject state has an INCOMING transition
	 * that starts from the state "parent" and that is marked with the label "label".
	 */
	bool BidirectionalState::hasIncomingTransition(const string& label, BidirectionalState* parent) const {
		auto it_label = this->m_incoming_transitions.find(label);
		if (it_label == this->m_incoming_transitions.end()) {
			return false;
		}
		// Otherwise, the label exists
		return (it_label->second.find(parent) != it_label->second.end());
	}
	
	/**
	 * Checks if the state has the same EXITING transitions as the state passed as a parameter.
	 * 
	 * The transitions are compared in the following way:
	 * 1) Check if both states implement the same type of transitions (bidirectional or not)
	 * 2) Check if both states have the same number of transitions
	 * 3) Check if both states have the same number of children for each transition
	 * 4) Check if both states have the same children for each transition (AT POINTER LEVEL)
	 * 
	 * The last note means that the two states must be linked to the same OBJECTS.
	 * If you need a level-name comparison (where object can also be different), you can use the 
	 * method "hasSameTransitionsNamesOf(State* other_state)".
	 */
	bool State::hasSameTransitionsOf(State* other_state) const {
		// Check if both states implement the same type of transitions
		if (this->isBidirectional() != other_state->isBidirectional()) {
			return false;
		}
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
		return true;
	}
	
	/**
	 * Checks if the state has the same EXITING and ENTERING transitions as the state passed 
	 * as a parameter.
	 * 
	 * The transitions are compared in the following way:
	 * 1) Check if both states implement the same type of transitions (bidirectional or not)
	 * 2) Check if both states have the same number of EXITING transitions
	 * 3) Check if both states have the same number of children for each transition
	 * 4) Check if both states have the same children for each transition (AT POINTER LEVEL)
	 * 5) Check if both states have the same number of ENTERING transitions
	 * 6) Check if both states have the same number of parents for each transition
	 * 7) Check if both states have the same parents for each transition (AT POINTER LEVEL)
	 * 
	 * The last note means that the two states must be linked to the same OBJECTS.
	 * If you need a level-name comparison (where object can also be different), you can use the 
	 * method "hasSameTransitionsNamesOf(State* other_state)".
	 */
	bool BidirectionalState::hasSameTransitionsOf(State* other_state) const {
		if (!other_state->isBidirectional()) {
			return false;
		}
		BidirectionalState* other_bidirectional_state = dynamic_cast<BidirectionalState*>(other_state);

		bool exiting_check = State::hasSameTransitionsOf(other_state);
		if (!exiting_check) {
			return false;
		} else {

			// Check incoming transitions
			if (this->m_incoming_transitions.size() != other_bidirectional_state->m_incoming_transitions.size()) {
				return false;
			}

			for (auto &pair: m_incoming_transitions) {
				string label = pair.first;
				set<BidirectionalState*> other_parents = other_bidirectional_state->m_incoming_transitions[label];

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
	}

	/**
	 * Checks if the state has the same EXITING transitions as the state passed as a parameter.
	 * The transitions are compared at NAME LEVEL, that means that children must have the same
	 * name, but can be different objects.
	 * 
	 * For this, the method is usually called when the two states are in different automata.
	 * It is a lighter version of the method "hasSameTransitionsOf(State* other_state)"
	 * 
	 * NOTE: The method does not check if the two states implement the same type of transitions.
	 * This means that both states can be bidirectional or not, but the method will not check it.
	 * It won't even check the INCOMING transitions, assuming that they're coherent within the 
	 * automaton (and thus, the check on the exiting transitions is enough).
	 */
	bool State::hasSameTransitionsNamesOf(State* other_state) const {
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
		return true;
	}

	/**	
	 * Returns the map of outgoing transitions from this state.
	 */
	map<string, set<State*>> State::getExitingTransitions() const {
		return m_exiting_transitions;
	}

	/**	
	 * Returns the map of incoming transitions into this state.
	 */
	map<string, set<BidirectionalState*>> BidirectionalState::getIncomingTransitions() const {
		return m_incoming_transitions;
	}

	/**
	 * Returns a reference to the map of transitions, that is, the address of the memory
	 * where the map is saved.
	 * Returning an address allows you to use this method as an lvalue in an assignment, for example.
	 */
	const map<string, set<State*>>& State::getExitingTransitionsRef() const {
		return m_exiting_transitions;
	}

	/**	
	 * Returns a reference to the map of incoming transitions, that is, the address of the memory
	 * where the map is saved.
	 * Returning an address allows you to use this method as an lvalue in an assignment, for example.
	 */
	const map<string, set<BidirectionalState*>>& BidirectionalState::getIncomingTransitionsRef() const {
		return m_incoming_transitions;
	}

	/**	
	 * Counts the outgoing transitions from the state.
	 * For each label, counts the amount of transitions referred to by that label outgoing
	 * from the current state.
	 */
	int State::getExitingTransitionsCount() const {
		int count = 0;
		for (auto &pair: m_exiting_transitions) {
			count += pair.second.size();
		}
		return count;
	}

	/**
	 * Counts the incoming transitions into the state.
	 */
	int BidirectionalState::getIncomingTransitionsCount() const {
		int count = 0;
		for (auto &pair: m_incoming_transitions) {
			count += pair.second.size();
		}
		return count;
	}

	/**
	 * Links the current state to the state passed as parameter, with a transition
	 * labeled with the label "label" passed as parameter.
	 * 
	 * NOTE: if the transition already exists, it is *not* added again.
	 * 
	 * @param state The state to be linked to the current state.
	 * @param label The label of the transition.
	 * @return True if the transition has been added, false otherwise.
	 */
	bool State::connectChild(const string& label, State* child)	{
		if (this->isBidirectional() != child->isBidirectional()) {
			throw "Cannot connect a bidirectional state to a non-bidirectional state";
		}

		// If the current state has no outgoing transitions labeled with "label",
		if (this->m_exiting_transitions.count(label) == 0) {
			// We setup a new SET of children labeled with "label"
			this->m_exiting_transitions.insert({label, set<State*>()});
		}
		
		// If the transition does not exist
		if (!this->hasExitingTransition(label, child)) {
			// We add the transition in both sense
			this->m_exiting_transitions[label].insert(child);
			return true;
		}
		// Otherwise, we do nothing: the transition already exists
		return false;
	}

	/**
	 * Links the current state to the state passed as parameter, with a transition
	 * labeled with the label "label" passed as parameter.
	 * 
	 * NOTE: if the transition already exists, it is *not* added again.
	 * 
	 * @param state The state to be linked to the current state.
	 * @param label The label of the transition.
	 * @return True if the transition has been added, false otherwise.
	 */
	bool BidirectionalState::connectChild(const string& label, State* child) {

		BidirectionalState* bidir_child = dynamic_cast<BidirectionalState*>(child);
		// Connect PARENT ---> CHILD  (exiting transition)
		if (this->State::connectChild(label, bidir_child)) {

			// If the child state has no incoming transitions labeled with "label",
			if (bidir_child->m_incoming_transitions.count(label) == 0) {
				// We setup a new set of parents labeled with "label"
				bidir_child->m_incoming_transitions.insert({label, set<BidirectionalState*>()});
			}
			// Then we add the transition CHILD <--- PARENT (incoming transition)
			if (!bidir_child->hasIncomingTransition(label, this)) {
				// We add the transition in both sense
				bidir_child->m_incoming_transitions[label].insert(this);
				return true;
			}
		}
		return false;
	}

	/**
	 * Disconnects two states, removing the transition that starts from this
	 * state and arrives in the one passed as parameter.
	 * 
	 * Precondition: it is assumed that such a transition exists.
	 */
	bool State::disconnectChild(const string& label, State* child) {
		if (!this->hasExitingTransition(label)) {
			DEBUG_LOG("There are no exiting transitions with label %s", label.c_str());
			return false;
		}
		// Search for the child to disconnect
		auto iterator = this->m_exiting_transitions[label].find(child);

		if (iterator != this->m_exiting_transitions[label].end()) {
			DEBUG_ASSERT_TRUE(this->hasExitingTransition(label, child));
			this->m_exiting_transitions[label].erase(iterator);
			DEBUG_ASSERT_FALSE(this->hasExitingTransition(label, child));
			return true;
		} else {
			DEBUG_LOG_FAIL("The child state %s has not been found for the label %s", child->getName().c_str(), label.c_str());
			return false;
		}
	}

	/**
	 * Disconnects two states, removing the transition that starts from this
	 * state and arrives in the one passed as parameter.
	 * 
	 * Precondition: it is assumed that such a transition exists.
	 */
	bool BidirectionalState::disconnectChild(const string& label, State* child) {
		BidirectionalState* bidirectional_child = dynamic_cast<BidirectionalState*>(child);
		if (!this->hasExitingTransition(label)) {
			DEBUG_LOG("There are no exiting transitions with label %s", label.c_str());
			return false;
		}
		// Search for the child to disconnect
		auto iterator = this->m_exiting_transitions[label].find(bidirectional_child);

		if (iterator != this->m_exiting_transitions[label].end()) {
			// Disconnect PARENT ---> CHILD  (exiting transition)
			DEBUG_ASSERT_TRUE(this->hasExitingTransition(label, bidirectional_child));
			this->m_exiting_transitions[label].erase(iterator);
			DEBUG_ASSERT_FALSE(this->hasExitingTransition(label, bidirectional_child));
			// Disconnect CHILD <--- PARENT  (incoming transition)
			DEBUG_ASSERT_TRUE(bidirectional_child->hasIncomingTransition(label, this));
			bidirectional_child->m_incoming_transitions[label].erase(this);
			DEBUG_ASSERT_FALSE(bidirectional_child->hasIncomingTransition(label, this));
			// Return true
			return true;
		} else {
			DEBUG_LOG_FAIL("The child state %s has not been found for the label %s", child->getName().c_str(), label.c_str());
			return false;
		}
	}

	/**
	 * Removes ALL the transitions of this state.
	 * It uses the "detachChild" method to remove the transitions, thus
	 * this method affects also the connected states, in order to maintain the 
	 * coherence of the graph/automaton.
	 * 
	 * However, if the state has no incoming transition reference, they cannot be removed.
	 */
	void State::detachAllTransitions() {
		for (auto pair_it = m_exiting_transitions.begin(); pair_it != m_exiting_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto child_it = pair_it->second.begin(); child_it != pair_it->second.end(); ) {
				this->disconnectChild(label, *(child_it++));
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}
	}

	/**
	 * Removes all the transitions from this state.
	 * The transitions are also updated on the nodes that
	 * were previously connected.
	 */
	void BidirectionalState::detachAllTransitions() {
		for (auto pair_it = m_exiting_transitions.begin(); pair_it != m_exiting_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto child_it = pair_it->second.begin(); child_it != pair_it->second.end(); ) {
				this->disconnectChild(label, *(child_it++));
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}

		for (auto pair_it = m_incoming_transitions.begin(); pair_it != m_incoming_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto parent_iterator = pair_it->second.begin(); parent_iterator != pair_it->second.end(); ) {
				if (*parent_iterator != this) {
					(*(parent_iterator++))->disconnectChild(label, this);
				}
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}
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
	void BidirectionalState::copyIncomingTransitionsOf(BidirectionalState* state) {
        for (auto &pair: state->getIncomingTransitionsRef()) {
            string label = pair.first;
            for (BidirectionalState* parent: pair.second) {
                if (!parent->hasExitingTransition(label, this)) {
                    parent->connectChild(label, this);
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
        copyExitingTransitionsOf(state);
    }

	/**	
	 * Copies all the incoming and outgoing transitions from the state "state"
	 * into this state.
	 * The already existing transitions are not duplicated.
	 */
	void BidirectionalState::copyAllTransitionsOf(State* state) {
		if (this->isBidirectional() != state->isBidirectional()) {
			throw std::invalid_argument("The states are not of the same type (bidirectional or not)");
		}
        copyExitingTransitionsOf(state);
		BidirectionalState* bidirectional_state = dynamic_cast<BidirectionalState*>(state);
        copyIncomingTransitionsOf(bidirectional_state);
    }
	
	/**
	 * Returns a string with all the information related to the state.
	 */
	string State::toString() const {
		string result = "";

		// Name of the state
		result += "\033[33;1m" + this->getName() + "\033[0m";

		/* // Level of the state, if present
		if (this->isBidirectional()) {
			BidirectionalState* bidirectional_state = dynamic_cast<BidirectionalState*>(this);
			result += " (level = " + std::to_string(bidirectional_state->getLevel()) + ")";
		} */

		// If the state is final, it is marked with a TAG
		if (this->isFinal()) {
			result += " [FINAL]";
		}

		result += "\n\t" + (std::to_string(this->getExitingTransitionsCount())) + " exiting transitions:\n";
		if (!this->m_exiting_transitions.empty()) {
			// For all the exiting transitions
			for (auto &pair: m_exiting_transitions) {
				string label = pair.first;
				for (State* state: pair.second) {
					result += "\t--|" + SHOW(label) + "|--> " + state->getName() + "\n";
//					result += "\t━━┥" + SHOW(label) + "┝━━▶ " + state->getName() + "\033[35m[id = " + (std::to_string((long int)state)) + "]\033[0m" + "\n";
				}
			}
		}
		return result;
	};


	/**
	 * This method clones the state, that is, it creates a new state with the same:
	 * - name;
	 * - final flag.
	 * 
	 * ATTENTION: it does NOT clone the transitions. Cloning them would require the knowledge of all the automaton states, which is not available.
	 */
	State* State::clone() {
		State* clone = new State(this->m_name, this->m_final);
		return clone;
	}

	/**
	 * This method clones the state, that is, it creates a new state with the same:
	 * - name;
	 * - level;
	 * - final flag.
	 * 
	 * ATTENTION: it does NOT clone the transitions. Cloning them would require the knowledge of all the automaton states, which is not available.
	 */
	BidirectionalState* BidirectionalState::clone() {
		BidirectionalState* clone = new BidirectionalState(this->getName(), this->isFinal());
		clone->setLevel(this->m_level);
		return clone;
	}

	/**
	 * This method clones the state, that is, it creates a new state with the same:
	 * - name;
	 * - finality;
	 * - extension (*see note below).
	 * 
	 * ATTENTION: it does NOT clone the transitions.
	 * ATTENTION: it does NOT clone the extension. The extension is just COPIED, that is, the same states are referenced.
	*/
	ConstructedState* ConstructedState::clone() {
		ConstructedState* clone = new ConstructedState(this->m_extension);
		return clone;
	}

	/**
	 * This method clones the state, that is, it creates a new state with the same:
	 * - name;
	 * - level;
	 * - finality;
	 * - extension (*see note below).
	 * 
	 * ATTENTION: it does NOT clone the transitions.
	 * ATTENTION: it does NOT clone the extension. The extension is just COPIED, that is, the same states are referenced.
	*/
	BidirectionalConstructedState* BidirectionalConstructedState::clone() {
		Extension extension = this->getExtension();
		BidirectionalConstructedState* clone = new BidirectionalConstructedState(extension);
		clone->setLevel(this->m_level);
		return clone;
	}

	/**
	 * Defines a "<" comparison operator, based on the comparison of the names.
	 */
	bool State::operator<(const State &other) const	{
		return this->getName() < other.getName();
	}

	/**	
	 * Defines an equality operator "==", based on the equality of the names.
	 */
	bool State::operator==(const State &other) const {
		return this->getName() == other.getName();
	}

	/**
	 * Defines an inequality operator "!=", based on the comparison of the names.
	 */
	bool State::operator!=(const State &other) const {
			return this->getName() != other.getName();
	}

	/**	
	 * Returns the level of this state.
	 */
    unsigned int BidirectionalState::getLevel() const {
        return m_level;
    }

	/**
	 * Sets the level of this state.
	 */
    void BidirectionalState::setLevel(unsigned int level) {
        m_level = level;
    }

	/**
	 * Sets the values of the levels starting from the current state as the root, 
	 * considering each transition as a unit increment on the level.
	 * In fact, the assignment of levels is not implemented with a recursive algorithm, 
	 * but a queue is used that allows a traversal of the automaton of type 
	 * breadth-first (breadth-first traversal).
	 * 
	 * NOTE: this method is called just after the construction of an automaton, 
	 * to define the levels of all the states; therefore, it is important that:
	 * 1) all the states belong to the same automaton
	 * 2) all the states have a levels not yet initialized (i.e. equal to the default value).
	 * 
	 * ATTENTION: please, reset the levels of all the states before calling this method.
	 * Otherwise, it won't work.
	 */
    void BidirectionalState::initLevelsRecursively(int root_level) {
        this->setLevel(root_level);
        list<BidirectionalState*> updated_list;
        updated_list.push_back(this);

        while (updated_list.size() > 0) {	

            BidirectionalState* current_state = updated_list.front();
            updated_list.pop_front();

            for (auto &trans: current_state->getExitingTransitionsRef()) {
                for (State* child : trans.second) {
					BidirectionalState* bidir_child = dynamic_cast<BidirectionalState*>(child);
					// If the level is not yet initialized
                    if (bidir_child->getLevel() == DEFAULT_VOID_LEVEL) {
                        bidir_child->setLevel(current_state->getLevel() + 1);
						// States are added in order of traversing, i.e. breadth-first
						// Therefore, they have increasing levels
                        updated_list.push_back(bidir_child);
                    }
                }
            }
        }
    }

	/**
	 * Returns the minimum level between all the levels of the parents.
	 */
    int BidirectionalState::getMinimumParentsLevel() {
    	int minimum = DEFAULT_VOID_LEVEL;
    	for (auto &pair : this->getIncomingTransitionsRef()) {
    		for (State* parent : pair.second) {
				BidirectionalState* bidir_parent = dynamic_cast<BidirectionalState*>(parent);
    			if (bidir_parent->getLevel() < minimum) {
    				minimum = bidir_parent->getLevel();
    			}
    		}
    	}
    	return minimum;
    }

	/**
	 * Static method.
	 * Creates the name of the state by concatenating the names of the states 
	 * of the extension. This method is used automatically to determine the 
	 * name of a ConstructedState object every time its extension is assigned 
	 * or modified.
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
	 * Sets the state with the marking value passed as parameter.
	 */
	void ConstructedState::setMarked(bool mark) {
		this->m_mark = mark;
	}

	/**
	 * Returns true if the state is marked.
	 */
	bool ConstructedState::isMarked() const {
		return this->m_mark;
	}

	/**
	 * Checks if the state has a specific extension passed as parameter.
	 * The comparison is performed by comparing the string containing all the names of the extension.
	 */
	bool ConstructedState::hasExtension(const Extension &ext) const {
		return (getName() == createNameFromExtension(ext));
	}

	/**
	 * Returns the extension of the state, that is, the set of State
	 * from which this state was created.
	 */
	const Extension& ConstructedState::getExtension() const {
		return m_extension;
	}

	/**
	 * Returns all the labels of the transitions outgoing from the states of the extension.
	 */
	set<string>& ConstructedState::getLabelsExitingFromExtension() const {
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
	Extension ConstructedState::computeLClosureOfExtension(const string& label) const {
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
	 * NOTE: This method assumes that the state does not have epsilon-transitions outgoing. 
	 * This is because it is used in algorithms (QSC) that should guarantee this condition.
	 */
	Extension ConstructedState::computeLClosure(const string& label) const {
		Extension l_closure;
		for (State* child : this->getChildren(label)) {
			l_closure.insert(child);
		}
		return ConstructedState::computeEpsilonClosure(l_closure);
	}

	/**
	 * Checks if the state is "empty", that is, if its extension is empty.
	 */
	bool ConstructedState::isExtensionEmpty() const {
		return m_extension.empty();
	}

	/**
	 * Replaces the extension of this state with another one.
	 * 
	 * NOTE: this method also causes the change of the name of the state, 
	 * based on the states of the NFA that are contained in the new extension.
	 */
	void ConstructedState::replaceExtensionWith(Extension &new_ext) {
		this->m_extension = new_ext;
		this->setName(createNameFromExtension(m_extension));
		this->setFinal(hasFinalStates(m_extension));
	}

	/**
	 * Checks if the state is safe with respect to a given singularity, represented by:
	 * - A state
	 * - A label
	 *
	 * A state is <safe> if and only if one of the following conditions is verified:
	 * - It is the initial state of the automaton (that is, its level is zero).
	 * - There exists an incoming transition in the state that satisfies both:
	 * 		- the transition must be different from the transition represented by the singularity
	 *		- the state of origin must have a level less than or equal to the state of the singularity
	 *
	 * ATTENTION: It is assumed that the state BELONGS to the closure of the singularity.
	 */
	bool BidirectionalConstructedState::isSafe(BidirectionalConstructedState* singularity_state, const string& singularity_label) const {
		if (this->getLevel() == 0) { // If the state is the initial state, it is safe
			return true;
		}

    	for (auto &pair : this->getIncomingTransitionsRef()) {
			string label = pair.first;

			if (label == singularity_label) {
				for (State* parent : pair.second) {
					BidirectionalConstructedState* bc_parent = dynamic_cast<BidirectionalConstructedState*>(parent);
					if (bc_parent != singularity_state
						&& bc_parent->getLevel() <= singularity_state->getLevel()) {
						return true;
					}
				}
			}
			else {
	    		for (State* parent : pair.second) {
					BidirectionalConstructedState* bc_parent = dynamic_cast<BidirectionalConstructedState*>(parent);
	    			if (bc_parent->getLevel() <= singularity_state->getLevel()) {
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
	bool BidirectionalConstructedState::isUnsafe(BidirectionalConstructedState* singularity_state, const string& singularity_label) const {
		return !this->isSafe(singularity_state, singularity_label);
	}

}
