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
		void generateStates(Automaton* nfa);
		State* getRandomState(Automaton* nfa);
		State* getRandomState(vector<State*>& states);
		State* getRandomStateWithUnusedLabels(vector<State*>& states, map<State*, Alphabet>& unused_labels);
		State* getRandomStateWithUnusedLabels(map<State*, Alphabet>& unused_labels);
		string getRandomLabel();
		string extractRandomUnusedLabel(map<State*, Alphabet>& unused_labels, State* state);

	public:
		NFAGenerator(Alphabet alphabet, Configurations* configurations);
		~NFAGenerator();

		Automaton* generateRandomAutomaton();						// Done
		Automaton* generateStratifiedAutomaton();					// Done
		Automaton* generateStratifiedWithSafeZoneAutomaton();		// Done
		Automaton* generateAcyclicAutomaton();						// Done
		Automaton* generateDopedAutomaton();						// Done
	};

} /* namespace quicksc */

#endif /* INCLUDE_AUTOMATAGENERATORNFA_HPP_ */
