/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * SubsetConstruction.hpp
 *
 * 
 * This header file contains the definition of the SubsetConstruction class,
 * namely, the class implementing the canonical Subset Construction algorithm.
 * The class is derived from the DeterminizationAlgorithm class.
 */

#ifndef INCLUDE_SUBSETCONSTRUCTION_HPP_
#define INCLUDE_SUBSETCONSTRUCTION_HPP_

#include "Automaton.hpp"
#include "DeterminizationAlgorithm.hpp"

namespace quicksc {

	class SubsetConstruction : public DeterminizationAlgorithm {

	public:
		SubsetConstruction();
		~SubsetConstruction();
		
		Automaton* prepareInputAutomaton(Automaton* nfa) const;
		Automaton* run(Automaton* nfa);

	};
}

#endif /* INCLUDE_SUBSETCONSTRUCTION_HPP_ */
