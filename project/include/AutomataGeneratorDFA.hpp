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
		void generateStates(Automaton& dfa);
		State* getRandomState(Automaton& dfa);
		State* getRandomStateWithUnusedLabels(vector<State*>& states, map<State*, Alphabet>& unused_labels);
		string extractRandomUnusedLabel(map<State*, Alphabet>& unused_labels, State* state);

	public:
		DFAGenerator(Alphabet alphabet, Configurations* configurations);
		~DFAGenerator();

		Automaton* generateRandomAutomaton();
		Automaton* generateStratifiedAutomaton();

	};

} /* namespace quicksc */

#endif /* INCLUDE_AUTOMATAGENERATORDFA_HPP_ */
