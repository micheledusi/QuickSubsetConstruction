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
#define EXPECTED_IMPACT					"EXP_IMPACT     [.] "
#define EXPECTED_GAIN					"EXP_GAIN       [.] "
#define NUMBER_SINGULARITIES_CHECKUP	"START_SING     [#] "
#define NUMBER_SINGULARITIES_SCENARIO_0	"S0_SING        [#] "
#define NUMBER_SINGULARITIES_SCENARIO_1	"S1_SING        [#] "
#define NUMBER_SINGULARITIES_SCENARIO_2	"S2_SING        [#] "
#define NUMBER_SINGULARITIES_TOTAL		"TOT_SING       [#] "
#define LEVEL_SINGULARITIES_CHECKUP		"STA_SING_LEVEL [#] "
#define LEVEL_SINGULARITIES_TOTAL		"TOT_SING_LEVEL [#] "
#define CLONING_TIME					"CLONING_TIME   [ns]"
#define RESTRUCTURING_TIME				"RESTRUCT_TIME  [ns]"
#define DISTANCE_RELOCATION_TIME		"RELOC_TIME     [ns]"

#define SCALE_FACTOR_QSC 1.3

using namespace std;

namespace quicksc {

	class QuickSubsetConstruction : public DeterminizationAlgorithm {

	private:
		SingularityList* m_singularities;

		void cleanInternalStatus();

		void runLevelRelocation(list<pair<BidirectionalState*, int>> relocation_sequence);
		void runLevelRelocation(BidirectionalState* state, int new_level);

		void addSingularityToList(BidirectionalConstructedState* singularity_state, string singularity_label);

	public:
		QuickSubsetConstruction(Configurations* configurations);
		~QuickSubsetConstruction();

		void resetRuntimeStatsValues();
		vector<RuntimeStat> getRuntimeStatsList();

		Automaton* prepareInputAutomaton(Automaton* nfa) const;
		Automaton* run(Automaton* nfa);

	};

} /* namespace quicksc */

#endif /* INCLUDE_QUICKSUBSETCONSTRUCTION_HPP_ */
