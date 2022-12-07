/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * QuickSubsetConstruction.hpp
 *
 *
 * This header file contains the declaration of the QuickSubsetConstruction class, implementing the Quick Subset Construction algorithm.
 * This algorithm is a variant of the Subset Construction algorithm, where the determinization follows a conservative approach.
 */

#ifndef INCLUDE_QUICKSUBSETCONSTRUCTION_HPP_
#define INCLUDE_QUICKSUBSETCONSTRUCTION_HPP_

#include <utility>

#include "Automaton.hpp"
#include "DeterminizationAlgorithm.hpp"
#include "Singularity.hpp"
#include "Configurations.hpp"

// Runtime Statistics
#define IMPACT							"IMPACT         [%] "
#define NUMBER_SINGULARITIES_CHECKUP	"START_SING     [#] "
#define NUMBER_SINGULARITIES_SCENARIO_0	"S0_SING        [#] "
#define NUMBER_SINGULARITIES_SCENARIO_1	"S1_SING        [#] "
#define NUMBER_SINGULARITIES_SCENARIO_2	"S2_SING        [#] "
#define NUMBER_SINGULARITIES_TOTAL		"TOT_SING       [#] "
#define LEVEL_SINGULARITIES_CHECKUP		"STA_SING_LEVEL [#] "
#define LEVEL_SINGULARITIES_TOTAL		"TOT_SING_LEVEL [#] "
#define CLONING_TIME					"CLONING_TIME   [ms]"
#define RESTRUCTURING_TIME				"RESTRUCT_TIME  [ms]"
#define DISTANCE_RELOCATION_TIME		"RELOC_TIME     [ms]"

using namespace std;

namespace quicksc {

	class QuickSubsetConstruction : public DeterminizationAlgorithm {

	private:
		SingularityList* m_singularities;

		void cleanInternalStatus();

		void runDistanceRelocation(list<pair<State*, int>> relocation_sequence);
		void runDistanceRelocation(State* state, int new_distance);

		void addSingularityToList(ConstructedState* singularity_state, string singularity_label);

	public:
		QuickSubsetConstruction(Configurations* configurations);
		~QuickSubsetConstruction();

		void resetRuntimeStatsValues();
		vector<RuntimeStat> getRuntimeStatsList();

		Automaton* run(Automaton* nfa);

	};

} /* namespace quicksc */

#endif /* INCLUDE_QUICKSUBSETCONSTRUCTION_HPP_ */
