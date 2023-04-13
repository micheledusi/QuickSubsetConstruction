/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataGeneratorNFA.cpp
 *
 * 
 * Source file for the class AutomataGeneratorNFA, which implements the AutomataGenerator interface.
 * 
 * The provided methods are:
 * - generateStratifiedWithSafeZoneAutomaton, which generates a stratified automaton with a safe zone. The "max-distance" parameter is used 
 * to set the number of desired strata. If the "max-distance" parameter is unset, it will be set to the cardinality of the automaton.
 * Plus, all the states within the safe zone will not have non-deterministic transitions, i.e. epsilon transitions or multiple transitions with the same label.
 */

#include "AutomataGeneratorNFA.hpp"

#include <cmath>
#include "AutomataGeneratorDFA.hpp"
#include "Configurations.hpp"

//#define DEBUG_MODE
#include "Debug.hpp"

#define RANDOM_PERCENTAGE ((double) rand() / (RAND_MAX))
#define INTRA_STRATUM_TRANSITIONS_PERCENTAGE 0.5

namespace quicksc {

	/**
	 * Constructor.
	 */
	NFAGenerator::NFAGenerator(Alphabet alphabet, Configurations* configurations) : AutomataGenerator(alphabet, configurations) {}

	/**
	 * Destructor.
	 */
	NFAGenerator::~NFAGenerator() {}

	/**
	 * Generates a completely random automaton.
	 * However, it respects the number of states, the percentage of epsilon transitions and the branching factor.
	 * It does not use the "SafeZoneDistance" nor the "maxDistance" parameters.
	 */
	Automaton* NFAGenerator::generateRandomAutomaton() {
		Automaton* nfa = new Automaton();

		// States generation
		this->generateStates(nfa);
		vector<State*> states = nfa->getStatesVector();
		DEBUG_ASSERT_TRUE( this->getSize() == nfa->size() );

		// Satisfaction of the REACHABILITY property
		for (int i = 1; i < states.size(); i++) {
			// Gets a random label, optionally epsilon
			string random_label = getRandomLabel();

			// Connecting:
			// FROM = a generic state before the current one in the vector order
			// TO = the current state of the vector at index i
			nfa->connectStates(states[rand() % i], states[i], random_label);
		}

		// Satisfaction of the NUMBER OF TRANSITIONS property

		/* The number of transitions to generate is calculated, considering the maximum number of transitions
		 * (i.e. the number of transitions of a complete graph, for each possible label) and the desired percentage of transitions.
		 */
		unsigned long int transitions_number = this->computeDeterministicTransitionsNumber();
		DEBUG_ASSERT_TRUE( transitions_number >= nfa->size() - 1 );

		for (	unsigned long int transitions_created = this->getSize() - 1;
				transitions_created < transitions_number;
				transitions_created++) {

			string label = getRandomLabel();

			// The indices are extracted
			int random_index_1 = (rand() % (states.size() - 1)) + 1;
			int random_index_2 = (rand() % (states.size() - 1)) + 1;

			// Creates the connection
			nfa->connectStates(
					states[random_index_1], 	// FROM
					states[random_index_2],		// TO
					label);
			/* Note: at the moment I have no way to guarantee that the connection does not already exist, in case 2 */
		}

		// Setting distances
		nfa->setInitialState(states[0]);
		states[0]->initDistancesRecursively(0);

		return nfa;
	}

	/**
	 * Generates a stratified automaton.
	 * It does not use the "SafeZoneDistance" parameter.
	 */
	Automaton* NFAGenerator::generateStratifiedAutomaton() {
		Automaton* nfa = new Automaton();

		this->generateStates(nfa);
		DEBUG_ASSERT_TRUE( this->getSize() == nfa->size() );

		// Obtaining a reference to the initial state
		State *initial_state = nfa->getStatesList().front();
		nfa->setInitialState(initial_state);

		// Checking the "maxDistance" parameter
		if (this->getMaxDistance() == UNDEFINED_VALUE) {
			// In this case, I set the maximum distance to the number of states - 1
			// The automaton that will be generated will be a long chain of states.
			this->setMaxDistance(this->getSize() - 1);
		}

		// Verify if the maximum distance satisfies the conditions on the number of states
			// Condition [1]: The number of states must be greater than the maximum distance
		if (this->getSize() <= this->getMaxDistance()) {
			DEBUG_LOG_ERROR("Impossibile generare un automa con %lu stati e distanza massima pari a %u", this->getSize(), this->getMaxDistance());
			throw "Impossibile generare un automa NFA con una distanza massima maggiore al numero di stati";
		} 	// Conditiion [2]: The number of states must not exceed the maximum value that allows to maintain the determinism in the automaton:
		else if ( (this->getMaxDistance() + 1) * log(this->getAlphabet().size()) < log(this->getSize() * (this->getAlphabet().size() - 1) + 1) ) {
			DEBUG_LOG_ERROR("Impossibile generare un automa con %lu stati e distanza massima pari a %u: numero di stati eccessivo", this->getSize(), this->getMaxDistance());
			throw "Impossibile generare un automa NFA con un numero di stati eccessivi per essere disposti deterministicamente entro la distanza massima";
		}
		/* Note: In theory a non-deterministic automaton can avoid to verify the condition [2], because it can reuse the same label for
		 * more transitions. The constraint [2], therefore, would be related to DFA automata.
		 * However, to allow a sufficiently large safe-zone (i.e. a sufficiently large deterministic zone) I must verify the condition even in
		 * the extreme case (i.e. in the case safe-zone = whole automaton). In intermediate cases the calculation would be too complicated.
		 */

		// Subdivision of the states in layers

		vector<vector<State*>> strata = vector<vector<State*>>();

		vector<State*> states = nfa->getStatesVector();

		// Creating as many strata as the maximum distance + 1
		for (int d = 0; d <= this->getMaxDistance(); d++) {
			strata.push_back(vector<State*>());
		}

		// Fillin the strata with the states, going from left to right
		int stratum_starting_index = 0;
		int stratum_index = stratum_starting_index;
		for (int s = 0; s < states.size(); s++) {
			// Addind the state to the current stratum
			strata[stratum_index].push_back(states[s]);
			DEBUG_LOG("Aggiungo lo stato %d allo strato %d", s, stratum_index);

			// Verify if the current stratum has reached the maximum number of states
			if ( log( strata[stratum_index].size() ) >= stratum_index * log( this->getAlphabet().size() )) {
				DEBUG_LOG("Lo strato %d ha raggiunto il numero massimo di stati al suo interno", stratum_index);
				stratum_starting_index++;
				DEBUG_LOG("Al successivo ciclo partirò dallo strato di indice %d", stratum_starting_index);
				/*
				 * Note: why this check on the cycle works, I rely on the fact that each stratum is more capable than the previous one,
				 * so they will be exhausted in order. So I can increase only one index that memorizes the first stratum still not
				 * full, instead of a flag for each that says whether a stratum is full or not.
				 */
			}

			// To the next stratum
			stratum_index++;

			// Check if I have to restart from the beginning
			if (stratum_index >= strata.size()) {
				DEBUG_LOG("Sono arrivato all'ultimo strato, ricomincerò dallo strato %d", stratum_starting_index);
				stratum_index = stratum_starting_index;
			}
		}

		DEBUG_LOG("Gli strati ottenuti sono %lu, e comprendono i seguenti stati:", strata.size());
		IF_DEBUG_ACTIVE(
		for (auto stratus : strata) {
			std::cout << "STRATO { ";
			for (State* state : stratus) {
				std::cout << state->getName() << " ";
			}
			std::cout << "}\n";
		})

		/* Satisfaction of the REACHABILITY
		 * The iteration on a stratum [i] assumes that the nodes of the stratum are
		 * connected with incoming transitions to those of the previous stratum.
		 */
		for (stratum_index = 1; stratum_index <= this->getMaxDistance(); stratum_index++) {
			// Rendo gli stati di questo strato raggiungibili
			for (State* state : strata[stratum_index]) {
				// Estraggo valori casuali e creo la connessione (Transizione)
				State* parent = this->getRandomState(strata[stratum_index - 1]);
				string random_label = getRandomLabel();
				nfa->connectStates(parent, state, random_label);
			}
		}

		// Distance setting
		initial_state->initDistancesRecursively(0);

		// Satisfaction of the TRANSITION PERCENTAGE
		/* The number of transitions to create is computed, considering the maximum number as the number of transitions in a complete graph.
		 * Then, the ratio is computed.
		 */
		unsigned long int transitions_number = this->computeDeterministicTransitionsNumber();
		DEBUG_ASSERT_TRUE( transitions_number >= nfa->size() - 1 );

		for (	unsigned long int transitions_created = this->getSize() - 1;
				transitions_created < transitions_number;
				transitions_created++) {

			// Extracting a random stratum from which to create the transition
			stratum_index = rand() % (this->getMaxDistance() + 1);

			// Extracting a random parent state
			State* from = this->getRandomState(strata[stratum_index]);
			// Extracting a random label, which can be EPSILON
			string label = getRandomLabel();

			// Compute the distance of the reached state (so the stratum of belonging)
			unsigned int to_dist = (RANDOM_PERCENTAGE <= INTRA_STRATUM_TRANSITIONS_PERCENTAGE) ?
					(stratum_index) :
					(stratum_index + 1);
			
			if (to_dist > this->getMaxDistance()) {
				to_dist = this->getMaxDistance();
			}
			State* to = this->getRandomState(strata[to_dist]);

			DEBUG_ASSERT_NOT_NULL(from);
			DEBUG_ASSERT_NOT_NULL(to);
			nfa->connectStates(from, to, label);
		}

		return nfa;
	}

	/**
	 * Generates a NFA with layers, where the non-determinism points appear only from a certain distance onwards.
	 */
	Automaton* NFAGenerator::generateStratifiedWithSafeZoneAutomaton() {
		Automaton* nfa = new Automaton();

		this->generateStates(nfa);
		DEBUG_ASSERT_TRUE( this->getSize() == nfa->size() );

		State *initial_state = nfa->getStatesList().front();
		nfa->setInitialState(initial_state);

		if (this->getMaxDistance() == UNDEFINED_VALUE) {
			this->setMaxDistance(this->getSize() - 1);
		}

		if (this->getSize() <= this->getMaxDistance()) {
			DEBUG_LOG_ERROR("Impossibile generare un automa con %lu stati e distanza massima pari a %u", this->getSize(), this->getMaxDistance());
			throw "Impossibile generare un automa NFA con una distanza massima maggiore al numero di stati";
		}
		else if ( (this->getMaxDistance() + 1) * log(this->getAlphabet().size()) < log(this->getSize() * (this->getAlphabet().size() - 1) + 1) ) {
			DEBUG_LOG_ERROR("Impossibile generare un automa con %lu stati e distanza massima pari a %u: numero di stati eccessivo", this->getSize(), this->getMaxDistance());
			throw "Impossibile generare un automa NFA con un numero di stati eccessivi per essere disposti deterministicamente entro la distanza massima";
		}

		vector<vector<State*>> strata = vector<vector<State*>>();
		vector<vector<State*>> safe_zone_strata = vector<vector<State*>>();

		vector<State*> states = nfa->getStatesVector();

		for (int d = 0; d <= this->getMaxDistance(); d++) {
			strata.push_back(vector<State*>());
		}
		for (int d = 0; d <= this->getSafeZoneDistance(); d++) {
			safe_zone_strata.push_back(vector<State*>());
		}

		int stratum_starting_index = 0;
		int stratum_index = stratum_starting_index;
		for (int s = 0; s < states.size(); s++) {
			strata[stratum_index].push_back(states[s]);
			DEBUG_LOG("Aggiungo lo stato %d allo strato %d", s, stratum_index);

			if ( log( strata[stratum_index].size() ) >= stratum_index * log( this->getAlphabet().size() )) {
				DEBUG_LOG("Lo strato %d ha raggiunto il numero massimo di stati al suo interno", stratum_index);
				stratum_starting_index++;
				DEBUG_LOG("Al successivo ciclo partirò dallo strato di indice %d", stratum_starting_index);
			}

			stratum_index++;

			if (stratum_index >= strata.size()) {
				DEBUG_LOG("Sono arrivato all'ultimo strato, ricomincerò dallo strato %d", stratum_starting_index);
				stratum_index = stratum_starting_index;
			}
		}

		for (int d = 0; d < safe_zone_strata.size() && d < strata.size(); d++) {
			for (int i = 0; i < strata[d].size(); i++) {
				safe_zone_strata[d].push_back(strata[d][i]);
			}
		}

		DEBUG_LOG("Gli strati ottenuti sono %lu, e comprendono i seguenti stati:", strata.size());
		IF_DEBUG_ACTIVE(
		for (auto stratus : strata) {
			std::cout << "STRATO { ";
			for (State* state : stratus) {
				std::cout << state->getName() << " ";
			}
			std::cout << "}\n";
		})

		/* A map keeps track of the labels used for each state of the safe zone.
		 * In this way we ensure the determinism avoiding duplicates in the labels.
		 * It requires the determinism to ALL AND ONLY the states with distance LESS than the SafeZoneDistance.
		 */
		map<State*, Alphabet> unused_labels = map<State*, Alphabet>();
		unsigned int limit = (this->getSafeZoneDistance() < strata.size()) ? this->getSafeZoneDistance() : strata.size();
		for (stratum_index = 0; stratum_index < limit; stratum_index++) {
			for (State* state : strata[stratum_index]) {
				unused_labels[state] = Alphabet(this->getAlphabet());
			}
		}

		// Satisfies the REACHABILITY property
		for (stratum_index = 1; stratum_index <= this->getMaxDistance(); stratum_index++) {
			// Making all the states of this stratum reachable from the previous one

			// CASE 1
			// The stratum is within the safe zone, I must guarantee the determinism
			if (stratum_index <= this->getSafeZoneDistance()) {
				for (State* state : strata[stratum_index]) {
					State* parent = this->getRandomStateWithUnusedLabels(strata[stratum_index - 1], unused_labels);
					string label = extractRandomUnusedLabel(unused_labels, parent);
					nfa->connectStates(parent, state, label);
				}
			}
			// CASE 2
			// The stratum is outside the safe zone, I can extract a random label (even EPSILON)
			else {
				for (State* state : strata[stratum_index]) {
					State* parent = this->getRandomState(strata[stratum_index - 1]);

					string random_label;
					if (RANDOM_PERCENTAGE <= this->getEpsilonProbability()) {
						random_label = EPSILON;
					} else {
						random_label = this->getAlphabet()[rand() % this->getAlphabet().size()];
					}

					nfa->connectStates(parent, state, random_label);
				}
			}
		}

		initial_state->initDistancesRecursively(0);


		unsigned long int transitions_number = this->computeDeterministicTransitionsNumber();
		DEBUG_ASSERT_TRUE( transitions_number >= nfa->size() - 1 );

		for (	unsigned long int transitions_created = this->getSize() - 1;
				transitions_created < transitions_number;
				transitions_created++) {

			stratum_index = rand() % (this->getMaxDistance() + 1);

			State* from;
			string label;

			// CASE 1
			// The stratum is within the safe zone, I must guarantee the determinism
			if (stratum_index < this->getSafeZoneDistance()) {

				from = this->getRandomStateWithUnusedLabels(unused_labels);
				label = this->extractRandomUnusedLabel(unused_labels, from);
				stratum_index = from->getDistance();

			}
			// CASE 2
			// The stratum is outside the safe zone, I can extract a random label (even EPSILON)
			else {

				from = this->getRandomState(strata[stratum_index]);
				if (RANDOM_PERCENTAGE <= this->getEpsilonProbability()) {
					label = EPSILON;
				} else {
					label = this->getAlphabet()[rand() % this->getAlphabet().size()];
				}

			}

			unsigned int to_dist = (RANDOM_PERCENTAGE <= INTRA_STRATUM_TRANSITIONS_PERCENTAGE) ?
					(stratum_index) :
					(stratum_index + 1);
			if (to_dist > this->getMaxDistance()) {
				to_dist = this->getMaxDistance();
			}
			State* to = this->getRandomState(strata[to_dist]);

			DEBUG_ASSERT_NOT_NULL(from);
			DEBUG_ASSERT_NOT_NULL(to);
			nfa->connectStates(from, to, label);
		}

		return nfa;
	}

	/**
	 * Generates a NFA without cycles.
	 * It does not use the "SafeZoneDistance" nor the "maxDistance" parameters.
	 */
	Automaton* NFAGenerator::generateAcyclicAutomaton() {
		
		Automaton* nfa = new Automaton();

		this->generateStates(nfa);
		vector<State*> states = nfa->getStatesVector();
		DEBUG_ASSERT_TRUE( this->getSize() == nfa->size() );

		for (int i = 1; i < states.size(); i++) {
			string random_label = getRandomLabel();
			nfa->connectStates(states[rand() % i], states[i], random_label);
		}

		unsigned long int transitions_number = this->computeDeterministicTransitionsNumber();
		DEBUG_ASSERT_TRUE( transitions_number >= nfa->size() - 1 );

		for (	unsigned long int transitions_created = this->getSize() - 1;
				transitions_created < transitions_number;
				transitions_created++) {

			string label = getRandomLabel();

			int random_index_1 = (rand() % (states.size() - 1)) + 1;
			int random_index_2 = (rand() % (states.size() - 1)) + 1;

			int from_index;
			int to_index;

			if (random_index_1 > random_index_2) {
				from_index = random_index_2;
				to_index = random_index_1;
			} else {
				from_index = random_index_1;
				to_index = random_index_2;
			}

			nfa->connectStates(
					states[from_index], 	// FROM
					states[to_index],		// TO
					label);
		}

		// Sets the initial state and the states level recursively
		nfa->setInitialState(states[0]);

		return nfa;
	}

	/**
	 * Generates a NFA starting from a DFA and "doping" it, by adding non-determinism.
	 * It assumes that the DFA has at least two states.
	 */
	Automaton* NFAGenerator::generateWeakAutomaton() {
		// Create a DFA
		AutomataGenerator* dfa_generator = new DFAGenerator(this->getAlphabet(), this->m_configurations);
		Automaton* nfa = dfa_generator->generateRandomAutomaton();

		// Get two random different states and a label
		vector<State*> states = nfa->getStatesVector();
		State *s1, *s2;
		string label;

		// Decide whether to add an epsilon transition or a non-deterministic transition
		if (RANDOM_PERCENTAGE <= this->getEpsilonProbability()) {
			// Add an epsilon transition
			// The label is epsilon
			label = EPSILON;
			// Two random different states are chosen, among the states of the DFA
			s1 = this->getRandomState(states);
			do {	
				s2 = this->getRandomState(states);
			} while (*s1 == *s2); // The two states must be different

		} else {
			// Otherwise, we add a non-deterministic transition
			bool choice_ok = false;
			do {
				// The statea are chosen randomly among the states of the DFA
				s1 = this->getRandomState(states);
				s2 = this->getRandomState(states);	

				// If the first state has NO outgoing transitions, we skip it
				if (s1->getExitingTransitionsCount() == 0) {
					continue;
				}
				map<string, set<State*>> exiting_transitions = s1->getExitingTransitionsRef();
				// We choose a random label from the used labels of s1
				auto it = exiting_transitions.begin();
				std::advance(it, rand() % exiting_transitions.size());
				label = it->first;
				// If it DOES NOT EXIST a transition with the chosen label from s1 to s2, then we can add it
				// If it DOES EXIST the transition, then we try again
			} while (s1->hasExitingTransition(label, s2));
		}

		// Connect them with an epsilon transition
		s1->connectChild(label, s2);
		DEBUG_ASSERT_TRUE( s1->hasExitingTransition(label, s2) );

		return nfa;
	}

	/**
	 * Generates a NFA with a specific topology (the Maslov topology).
	 * 
	 * This type of automaton is a NFA with N states.
	 * The first state is the initial state, and the last state is the ONLY final state.
	 * It has only two labels: "a" and "b".
	 * Each state has two outgoing transitions, one with label "a" and one with label "b", directed to the next state.
	 * This has the effect of creating a chain of states, where each state has two outgoing transitions.
	 * There are two exceptions:
	 * - The first state has two outgoing transitions, one with label "a" and one with label "b", directed to the first state itself.
	 *   Also, it has a third outgoing transition, with label "a", directed to the second state.
	 * - The last state has NO outgoing transitions.
	*/
	Automaton* NFAGenerator::generateMaslovAutomaton() {
		// Creating the NFA object
		Automaton* nfa = new Automaton();
		// Generating the states
		this->generateStates(nfa);
		vector<State*> states = nfa->getStatesVector();
		DEBUG_ASSERT_TRUE( this->getSize() == nfa->size() );

		// Connecting the states
		// Starting from the second state, we connect it the next state with both labels
		for (int i = 1; i < states.size() - 1; i++) {
			nfa->connectStates(states[i], states[i + 1], "a");
			nfa->connectStates(states[i], states[i + 1], "b");
		}
		// Managing the first state
		nfa->connectStates(states[0], states[0], "a");
		nfa->connectStates(states[0], states[0], "b");
		nfa->connectStates(states[0], states[1], "a");

		// Setting the initial state
		nfa->setInitialState(states[0]);

		return nfa;
	}

	/**
	 * Generates a list of State objects, each one with a unique name, and inserts them into the NFA passed as parameter.
	 * The states do not have transitions. The states are as many as the size specified by the NFAGenerator parameters.
	 */
	void NFAGenerator::generateStates(Automaton* nfa) {
		bool hasFinalStates = false;

		for (int s = 0; s < this->getSize(); s++) {
			string name = this->generateUniqueName();
			bool final = (this->generateNormalizedDouble() < this->getFinalProbability());
			hasFinalStates |= final;

			State *state = new State(name, final);
			nfa->addState(state);
		}
		DEBUG_ASSERT_TRUE((nfa->size()) == this->getSize());

		if (!hasFinalStates) {
			this->getRandomState(nfa)->setFinal(true);
		}
	}

	/**
	 * Extracts a random state from the NFA passed as parameter.
	 * NOTE: This method requires the construction of the vector of states of the automaton at each call,
	 * which may not be computationally too efficient.
	 */
	State* NFAGenerator::getRandomState(Automaton* nfa) {
		vector<State*> states = nfa->getStatesVector();
		return states.at(rand() % states.size());
	}

	/**
	 * Extracts a random state from the vector of states passed as parameter.
	 */
	State* NFAGenerator::getRandomState(vector<State*>& states) {
		return states.at(rand() % states.size());
	}

	/**
	 * Returns a random state from the list passed as parameter, ensuring that it has still unused labels available for the creation of transitions.
	 * 
	 * NOTE: It does not remove the label from the map of usages.
	 */
	State* NFAGenerator::getRandomStateWithUnusedLabels(vector<State*> &states, map<State*, Alphabet> &unused_labels) {
		vector<State*> states_aux = vector<State*>(states);

		State* from;
		bool from_state_has_unused_labels = false;
		do {
			if (states_aux.empty()) {
				DEBUG_LOG_ERROR("Impossibile estrarre uno stato da una lista vuota");
				throw "Impossibile estrarre uno stato da una lista vuota";
			}

			int random_index = rand() % states_aux.size();
			from = states_aux[random_index];

			if (unused_labels.count(from) > 0 && unused_labels.at(from).size() > 0) {
				DEBUG_LOG("Ho trovato lo stato %s con %lu labels non utilizzate", from->getName().c_str(), unused_labels.at(from).size());
				from_state_has_unused_labels = true;
			}
			else {
				DEBUG_LOG("Lo stato %s non ha più label inutilizzate; eviterò di selezionarlo nelle iterazioni successive", from->getName().c_str());
				states_aux.erase(states_aux.begin() + random_index);
			}

		} while (!from_state_has_unused_labels);

		return from;
	}

	State* NFAGenerator::getRandomStateWithUnusedLabels(map<State*, Alphabet> &unused_labels) {
		State* from;
		bool from_state_has_unused_labels = false; 
		do {
			if (unused_labels.empty()) {
				DEBUG_LOG_ERROR("Impossibile estrarre uno stato da una lista vuota");
				throw "Impossibile estrarre uno stato da una lista vuota";
			}

			int random_index = rand() % unused_labels.size();
			int i = 0;
			for (auto &pair : unused_labels) {
				if (i == random_index) {
					from = pair.first;
					if (pair.second.size() > 0) {
						DEBUG_LOG("Ho trovato lo stato %s con %lu labels non utilizzate", from->getName().c_str(), unused_labels.at(from).size());
						from_state_has_unused_labels = true;
						return from;
					}

					break;

				} else {
					i++;
				}
			}

			DEBUG_LOG("Lo stato %s non ha più label inutilizzate; eviterò di selezionarlo nelle iterazioni successive", from->getName().c_str());
			unused_labels.erase(from);

		} while (!from_state_has_unused_labels);
		/*
		 * NOTE: in theory, here we assume that the condition remains always false and that the cycle is repeated.
		 * If it becomes true, the state is returned before.
		 */

		return from;
	}

	/**
	 * Returns a random label (as string) selected from the alphabet.
	 * It can return an empty label (EPSILON) with the probability specified by "EpsilonProbability".
	 */
	string NFAGenerator::getRandomLabel() {
		if (RANDOM_PERCENTAGE <= this->getEpsilonProbability()) {
			return EPSILON;
		} else {
			return this->getAlphabet()[rand() % this->getAlphabet().size()];
		}
	}

	/**
	 * Extracts (and removes) a random label from the list of unused labels of a specific state.
	 */
	string NFAGenerator::extractRandomUnusedLabel(map<State*, Alphabet> &unused_labels, State* state) {
		if (unused_labels[state].empty()) {
			DEBUG_LOG_ERROR( "Non è stata trovata alcuna label inutilizzata per lo stato %s", state->getName().c_str() );
			return NULL;
		}
		int label_random_index = rand() % unused_labels[state].size();
		string extracted_label = unused_labels[state][label_random_index];
		DEBUG_LOG("Estratta l'etichetta %s dallo stato %s", extracted_label.c_str(), state->getName().c_str());

		unused_labels[state].erase(unused_labels[state].begin() + label_random_index);
		return extracted_label;
	}

} /* namespace quicksc */
