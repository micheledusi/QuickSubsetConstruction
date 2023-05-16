/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataGeneratorDFA.hpp
 *
 *
 * This header file contains the definition of the DFAGenerator class, child of the AutomataGenerator class.
 * 
 * The DFAGenerator class is used to generate a DFA (Deterministic Finite Automaton) from a given alphabet and a set of parameters.
 * The DFA is generated with random values; that means that every call returns a different DFA.
 * 
 * The DFA will be used to test the determinization algorithms.
 */

#ifndef INCLUDE_AUTOMATAGENERATORDFA_HPP_
#define INCLUDE_AUTOMATAGENERATORDFA_HPP_

#include "AutomataGenerator.hpp"
#include "Automaton.hpp"

namespace quicksc {

	class DFAGenerator : public AutomataGenerator {

	private:
		template<class S> void generateStates(Automaton& dfa);
		State* getRandomState(Automaton& dfa);
		State* getRandomStateWithUnusedLabels(vector<State*>& states, map<State*, Alphabet>& unused_labels);
		string extractRandomUnusedLabel(map<State*, Alphabet>& unused_labels, State* state);

	public:
		DFAGenerator(Alphabet alphabet, Configurations* configurations);
		~DFAGenerator();

		Automaton* generateRandomAutomaton();
		// Automaton* generateStratifiedAutomaton();

	};

} /* namespace quicksc */

// Include template implementation

namespace quicksc {
	
	/**
	 * Generates a list of State objects, each with a unique name, and inserts them into the DFA passed as a parameter.
	 * The states do not have transitions. The states are in number equal to the size expected by the parameters of the DFAGenerator object.
	 */
	template<class S> void DFAGenerator::generateStates(Automaton& dfa) {
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
			State* state = new S(name, final);
			dfa.addState(state);
		}
		// Forcing the existence of at least one final state
		if (!hasFinalStates) {
			this->getRandomState(dfa)->setFinal(true);
		}
	}

}

#endif /* INCLUDE_AUTOMATAGENERATORDFA_HPP_ */
