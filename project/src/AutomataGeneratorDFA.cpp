/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataGeneratorDFA.hpp
 *
 *
 * Source file for the class AutomataGeneratorDFA, which is responsible for generating deterministic finite automata.
 * The AutomataGeneratorDFA class is derived from the AutomataGenerator class.
 *
 * The current provided methods are:
 * - generateRandomAutomaton, to generate a random automaton, withouth any constraint on topology, but with a given size and percentage of transitions.
 * - generateStratifiedAutomaton, to generate a stratified automaton, namely, an automaton where the "max_distance" is the number of desired strata.
 * 	 If the "max_distance" is unset, the number of strata will be equal to the size.
 */

#include "AutomataGeneratorDFA.hpp"

#include "Debug.hpp"

namespace quicksc {

	/**
	 * Constructor.
	 */
	DFAGenerator::DFAGenerator(Alphabet alphabet, Configurations* configurations) : AutomataGenerator(alphabet, configurations) {}

	/**
	 * Destructor.
	 */
	DFAGenerator::~DFAGenerator() {}


	Automaton* DFAGenerator::generateRandomAutomaton() {
		Automaton* dfa = new Automaton();

		// States generation
		this->generateStates(*dfa);
		DEBUG_ASSERT_TRUE( this->getSize() == dfa->size() );

		// Reference to the initial state
		State *initial_state = dfa->getStatesList().front(); // I'm assuming the first state is the initial state, which is correct
		dfa->setInitialState(initial_state);

		// Transitions generation

		/*
		 * Note: 
		 * The creation of the transitions is done in two steps, in order to guarantee the connection of the final automaton.
		 * 
		 * PHASE (0): Calculation of the number of transitions
		 * 
		 * 0.1) The number of transitions to generate is calculated, considering the maximum number
		 * (that is, the number of transitions of a complete graph, for each possible label) and
		 * the desired percentage of transitions.
		 * 0.2) If this number is not sufficient to cover all the states, the number is brought to N-1
		 * (where N is the number of states of the automaton). This number, as the algorithm has been built,
		 * guarantees the connection.
		 */
		unsigned long int transitions_number = this->computeDeterministicTransitionsNumber();
		DEBUG_ASSERT_TRUE( transitions_number >= dfa->size() - 1 );

		/* PHASE (1): Creation of a covering tree
		 *
		 * 1.1) A queue of "reachable" nodes is prepared, to which the initial state of the automaton is added. */
		vector<State*> reached_states;
		reached_states.push_back(initial_state);

		/* 1.2) A map keeps track of the labels used for each state, so that states with outgoing transitions
		 * marked by the same label are not created. */
		map<State*, Alphabet> unused_labels;
		for (State* state : dfa->getStatesVector()) {
			unused_labels[state] = Alphabet(this->getAlphabet());
		}

		/* 1.3) At the same time, the nodes that have not yet been marked as "reachable" are kept track of.
		 * At the beginning, all the states belong to this list, except the initial node. */
		list<State*> unreached_states_queue = dfa->getStatesList();
		unreached_states_queue.pop_front();

		/* 1.4) A random "reachable" state and a random "unreachable" state are extracted, and a transition
		 * is created from the first to the second.
		 * An important condition is that the "from" state still has outgoing labels available to generate the transition.
		 * If this is not the case, it is directly excluded from the list of reachable states
		 * because it is no longer of any use. */
		while (!unreached_states_queue.empty()) {
			State* from = this->getRandomStateWithUnusedLabels(reached_states, unused_labels);
			State* to = unreached_states_queue.front();

		/* 1.5) In addition, a random label (and removed) is extracted from those not used in the "from" state. */
			string label = this->extractRandomUnusedLabel(unused_labels, from);

			dfa->connectStates(from, to, label);

		/* 1.6) The second state is marked as "reachable". It is therefore extracted from the second queue and inserted into the first. */
			unreached_states_queue.pop_front();
			reached_states.push_back(to);

		/* 1.7) If the current number of transitions is less than N-1, return to point (1.3). */
		}

		/* PHASE (2): Reaching the required percentage
		 *
		 * 2.1) Two random states are extracted from the automaton. A transition is created between them.
		 * 2.2) Point (2.1) is repeated until the number of transitions is reached or exceeded
		 * calculated during phase (0).
		 */
		for (	unsigned long int transitions_created = this->getSize() - 1;
				transitions_created < transitions_number;
				transitions_created++) {

			State* from = this->getRandomStateWithUnusedLabels(reached_states, unused_labels);
			State* to = this->getRandomState(*dfa);
			string label = this->extractRandomUnusedLabel(unused_labels, from);
			dfa->connectStates(from, to, label);
		}

		// Setting the distance from the initial state
		dfa->getInitialState()->initDistancesRecursively(0);

		return dfa;
	}

	/** 
	 * Method that returns a stratified automaton.
	 * The number of layers is given by the maximum distance (parameter "max_distance") set in the generator.
	 * Each layer has the same size, with the exception of the layer with distance 0 (containing only the initial node) and the last (containing the remaining states, not necessarily in number equal to the rest).
	 * It is guaranteed that:
	 * - The automaton has the number of states set by the "size" parameter.
	 * - The automaton is deterministic.
	 * - The automaton has the percentage of transitions set with respect to the total.
	 * - Each layer contains nodes with the same distance.
	 * - Each node has transitions to nodes of the same layer or the next layer.
	 */
	Automaton* DFAGenerator::generateStratifiedAutomaton() {
		Automaton* dfa = new Automaton();

		// States generation
		this->generateStates(*dfa);
		DEBUG_ASSERT_TRUE( this->getSize() == dfa->size() );

		// Reference to the initial state
		State *initial_state = dfa->getStatesList().front();	// I'm assuming the first state is the initial state, which is correct
		dfa->setInitialState(initial_state);

		// Check if the maximum distance is set
		if (this->getMaxDistance() == UNDEFINED_VALUE || this->getMaxDistance() >= this->getSize()) {
			// If not, the maximum distance is set to the number of states
			// The resulting automaton will be a long chain of states
			this->setMaxDistance(this->getSize() - 1);
		}

		/** Computing the number of nodes for each layer
		 * The number will be given by the number of non-initial nodes = (N - 1),
		 * where N is the size ("size") of the automaton, divided by the maximum
		 * distance and rounded up.
		 */
		unsigned int strata_size = (unsigned int)((this->getSize() - 1) / this->getMaxDistance());
		/** Computing the number of layers that will have one more node, due to the integer division.
		 */
		unsigned int slightly_bigger_strata_number = (this->getSize() - 1) % this->getMaxDistance();

		/**
		 * Checking that it is possible to create a deterministic automaton with that number of nodes for each layer with the labels available in the alphabet.
		 */
		DEBUG_LOG("Alpha size = %lu", this->getAlphabet().size());
		DEBUG_LOG("Strata size = %u", strata_size);
		DEBUG_ASSERT_FALSE(this->getAlphabet().size() < (strata_size + ((slightly_bigger_strata_number) ? 1 : 0)));
		if (this->getAlphabet().size() < (strata_size + ((slightly_bigger_strata_number) ? 1 : 0))) {
			DEBUG_LOG_ERROR("Impossibile creare un automa deterministico con un numero di nodi per strato così alto e un numero di label insufficiente");
			std::cout << "Impossibile creare un automa deterministico con un numero di nodi per strato così alto e un numero di label insufficiente\n";
			return NULL;
		}

		// Splitting the states into layers/strata

		// Layers initialization
		vector<vector<State*>> strata = vector<vector<State*>>();
		vector<State*> states = dfa->getStatesVector();

		// Remove the initial state and insert it as the first layer (at distance 0)
		strata.push_back(vector<State*>());
		strata[0].push_back(*(states.begin()));
		states.erase(states.begin());

		// All other states are divided equally
		int stratus_index = 1;
		strata.push_back(vector<State*>());
		for (State* state : states) {
			// Insert the state in the current stratum
			strata[stratus_index].push_back(state);

			// If the maximum size for the stratum has been reached
			if (strata[stratus_index].size() > (strata_size + ((stratus_index > slightly_bigger_strata_number) ? -1 : 0))) {
				// Go to the next one
				stratus_index++;
				strata.push_back(vector<State*>());
			}
		}

		IF_DEBUG_ACTIVE(
		for (auto stratus : strata) {
			std::cout << "STRATO { ";
			for (State* state : stratus) {
				std::cout << state->getName() << " ";
			}
			std::cout << "}\n";
		})

		// A map keeps track of the labels used for each state, so that states with outgoing transitions marked by the same label are not created.
		map<State*, Alphabet> unused_labels;
		for (State* state : dfa->getStatesVector()) {
			unused_labels[state] = Alphabet(this->getAlphabet());
		}

		/* Satisfaction of the REACHABILITY
		 * The iteration on a stratum [i] assumes that the nodes of the stratum are
		 * connected with incoming transitions to those of the previous stratum.
		 */
		for (int stratus_index = 1; stratus_index <= this->getMaxDistance(); stratus_index++) {
			// Making the states of the current stratum as reached
			for (State* state : strata[stratus_index]) {
				State* parent = this->getRandomStateWithUnusedLabels(strata[stratus_index - 1], unused_labels);
				string label = extractRandomUnusedLabel(unused_labels, parent);
				dfa->connectStates(parent, state, label);
			}
		}

		// Distance setting
		initial_state->initDistancesRecursively(0);

		// Satisfaction of the TRANSITION PERCENTAGE
		/* The number of transitions to create is computed, considering the maximum number as the number of transitions in a complete graph.
		 * Then, the ratio is computed.
		 */
		unsigned long int transitions_number = this->computeDeterministicTransitionsNumber();
		DEBUG_ASSERT_TRUE( transitions_number >= dfa->size() - 1 );

		for (	unsigned long int transitions_created = this->getSize() - 1;
				transitions_created < transitions_number;
				transitions_created++) {

			// Extract the parent state
			State* from = this->getRandomStateWithUnusedLabels(states, unused_labels);
			unsigned int from_dist = from->getDistance();
			// Computing the distance of the reached state (thus the stratum)
			unsigned int to_dist = (rand() % 2) ? (from_dist) : (from_dist + 1);
			if (to_dist > this->getMaxDistance()) {
				to_dist = this->getMaxDistance();
			}
			// Extract the child state
			State* to = this->getRandomStateWithUnusedLabels(strata[to_dist], unused_labels);
			string label = this->extractRandomUnusedLabel(unused_labels, from);
			dfa->connectStates(from, to, label);
		}

		return dfa;
	}
	
	/**
	 * Generates a list of State objects, each with a unique name, and inserts them into the DFA passed as a parameter.
	 * The states do not have transitions. The states are in number equal to the size expected by the parameters of the DFAGenerator object.
	 */
	void DFAGenerator::generateStates(Automaton& dfa) {
		// Flag for the existence of at least one final state
		bool hasFinalStates = false;

		// States generation
		for (int s = 0; s < this->getSize(); s++) {
			// The name of the state is the number of the state
			string name = this->generateUniqueName();
			// Set a state as final based on the percentage
			bool final = (this->generateNormalizedDouble() < this->getFinalProbability());
			hasFinalStates |= final;

			// Create the state and add it to the DFA
			State *state = new State(name, final);
			dfa.addState(state);
		}
		DEBUG_ASSERT_TRUE(dfa.size() == this->getSize());

		// Forcing the existence of at least one final state
		if (!hasFinalStates) {
			this->getRandomState(dfa)->setFinal(true);
		}
	}

	/**
	 * Extracts a random state from the automaton.
	 */
	State* DFAGenerator::getRandomState(Automaton& dfa) {
		vector<State*> states = dfa.getStatesVector();
		return states.at(rand() % states.size());
	}

	/**
	 * Extracts a random state from the list passed as a parameter, ensuring that it has unused labels available for the creation of transitions.
	 * Note: It does not remove the label from the map of uses.
	 */
	State* DFAGenerator::getRandomStateWithUnusedLabels(vector<State*> &states, map<State*, Alphabet> &unused_labels) {
		if (states.empty()) {
			DEBUG_LOG_ERROR("Impossibile estrarre uno stato da una lista vuota");
			return NULL;
		}

		State* from;
		bool from_state_has_unused_labels = false; // Peximistic flag 
		do {
			// Random index extraction
			int random_index = rand() % states.size();
			from = states[random_index];

			// Check for the existence of unused labels that are still available
			if (unused_labels[from].size() > 0) {
				DEBUG_LOG("Ho trovato lo stato %s con %lu labels non utilizzate", from->getName().c_str(), unused_labels[from].size());
				from_state_has_unused_labels = true;
			} else {
				// Delete the state from the list of states from which to draw
				DEBUG_LOG("Elimino lo stato %s con poiché non ha labels inutilizzate", from->getName().c_str());
				int previous_size = states.size();
				states.erase(states.begin() + random_index);
				DEBUG_ASSERT_TRUE( previous_size < states.size());
			}

		} while (!from_state_has_unused_labels);

		return from;
	}

	/**
	 * Extracts (and removes) a random label from the list of unused labels of a specific state.
	 */
	string DFAGenerator::extractRandomUnusedLabel(map<State*, Alphabet> &unused_labels, State* state) {
		if (unused_labels[state].empty()) {
			DEBUG_LOG_ERROR( "Non è stata trovata alcuna label inutilizzata per lo stato %s", state->getName().c_str() );
			return NULL;
		}
		int label_random_index = rand() % unused_labels[state].size();
		string extracted_label = unused_labels[state][label_random_index];
		DEBUG_LOG("Estratta l'etichetta %s dallo stato %s", extracted_label.c_str(), state->getName().c_str());

		// Delete the used label
		unused_labels[state].erase(unused_labels[state].begin() + label_random_index);
		return extracted_label;
	}

} /* namespace quicksc */
