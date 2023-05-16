/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataGeneratorNFA.hpp
 *
 * 
 * This header file contains the definition of the NFAGenerator class, child of the AutomataGenerator class.
 * 
 * The NFAGenerator class is used to generate a NFA (Non-Deterministic Finite Automaton) from a given alphabet and a set of parameters.
 * The NFA is generated with random values; that means that every call returns a different NFA.
 * 
 * The NFA will be used to test the determinization algorithms.
 */

#ifndef INCLUDE_AUTOMATAGENERATORNFA_HPP_
#define INCLUDE_AUTOMATAGENERATORNFA_HPP_

#include "AutomataGenerator.hpp"

namespace quicksc {

	class NFAGenerator: public AutomataGenerator {

	private:
		template <class S> void generateStates(Automaton* nfa);
		State* getRandomState(Automaton* nfa);
		template <class S> S* getRandomState(vector<S*>& states);
		template <class S> S* getRandomStateWithUnusedLabels(vector<S*>& states, map<S*, Alphabet>& unused_labels);
		template <class S> S* getRandomStateWithUnusedLabels(map<S*, Alphabet>& unused_labels);
		string getRandomLabel();
		template <class S> string extractRandomUnusedLabel(map<S*, Alphabet>& unused_labels, S* state);

	public:
		NFAGenerator(Alphabet alphabet, Configurations* configurations);
		~NFAGenerator();

		Automaton* generateRandomAutomaton();						// Done
		Automaton* generateStratifiedAutomaton();					// Done
		Automaton* generateStratifiedWithSafeZoneAutomaton();		// Done
		Automaton* generateAcyclicAutomaton();						// Done
		Automaton* generateWeakAutomaton();							// Done
		Automaton* generateMaslovAutomaton();						// Done
	};

} /* namespace quicksc */

#include "Debug.hpp"
// Include template implementation

namespace quicksc {


	/**
	 * Generates a list of State objects, each one with a unique name, and inserts them into the NFA passed as parameter.
	 * The states do not have transitions. The states are as many as the size specified by the NFAGenerator parameters.
	 */
	template<class S> void NFAGenerator::generateStates(Automaton* nfa) {
		// Flag for the existence of at least one final state
		bool hasFinalStates = false;

		for (int s = 0; s < this->getSize(); s++) {
			string name = this->generateUniqueName();
			bool final = (this->generateNormalizedDouble() < this->getFinalProbability());
			hasFinalStates |= final;

			// Create the state of the templated type and add it to the NFA
			State *state = new S(name, final);
			nfa->addState(state);
		}
		DEBUG_ASSERT_TRUE((nfa->size()) == this->getSize());

		if (!hasFinalStates) {
			this->getRandomState(nfa)->setFinal(true);
		}
	}

	/**
	 * Extracts a random state from the vector of states passed as parameter.
	 */
	template <class S> S* NFAGenerator::getRandomState(vector<S*>& states) {
		return states.at(rand() % states.size());
	}

	/**
	 * Returns a random state from the list passed as parameter, ensuring that it has still unused labels available for the creation of transitions.
	 * 
	 * NOTE: It does not remove the label from the map of usages.
	 */
	template <class S> S* NFAGenerator::getRandomStateWithUnusedLabels(vector<S*> &states, map<S*, Alphabet> &unused_labels) {
		vector<S*> states_aux = vector<S*>(states);

		S* from;
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

	template <class S> S* NFAGenerator::getRandomStateWithUnusedLabels(map<S*, Alphabet> &unused_labels) {
		S* from;
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
	 * Extracts (and removes) a random label from the list of unused labels of a specific state.
	 */
	template <class S> string NFAGenerator::extractRandomUnusedLabel(map<S*, Alphabet> &unused_labels, S* state) {
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

}

#endif /* INCLUDE_AUTOMATAGENERATORNFA_HPP_ */
