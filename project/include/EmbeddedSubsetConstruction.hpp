/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * EmbeddedSubsetConstruction.hpp
 *
 * 
 * This header file contains the definition of the class EmbeddedSubsetConstruction,
 * implementing the algorithm "Embedded Subset Construction", which is a previous version of the Quick Subset Construction algorithm,
 * used especially for determinizing automata coming from a translation of alphabets.
 * 
 * It remains in this project for historical reasons.
 */

#ifndef INCLUDE_EMBEDDEDSUBSETCONSTRUCTION_HPP_
#define INCLUDE_EMBEDDEDSUBSETCONSTRUCTION_HPP_

#include <utility>

#include "Automaton.hpp"
#include "DeterminizationAlgorithm.hpp"
#include "Singularity.hpp"
#include "Configurations.hpp"

using namespace std;

namespace quicksc {

	class EmbeddedSubsetConstruction : public DeterminizationAlgorithm {

	private:
		Automaton* m_nfa;
		Automaton* m_dfa;
		SingularityList* m_singularities;

		bool m_active_removing_label;
		bool m_active_automaton_pruning;
		bool m_active_distance_check_in_translation;

		void cleanInternalStatus();

		void runAutomatonCheckup(Automaton* automaton);
		void runSingularityProcessing();

		void runDistanceRelocation(list<pair<State*, int>> relocation_sequence);
		void runDistanceRelocation(State* state, int new_distance);
		void runExtensionUpdate(ConstructedState* state, Extension& new_extension);
		void runAutomatonPruning(Singularity* singularity);

		void addSingularityToList(ConstructedState* singularity_state, string singularity_label);

	public:
		EmbeddedSubsetConstruction(Configurations* configurations);
		~EmbeddedSubsetConstruction();

		Automaton* run(Automaton* nfa);

	};

} /* namespace quicksc */

#endif /* INCLUDE_EMBEDDEDSUBSETCONSTRUCTION_HPP_ */
