/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * DeterminizationAlgorithm.hpp
 *
 *
 * This header file contains the declaration of the DeterminizationAlgorithm class.
 * The DeterminizationAlgorithm class is an abstract class representing the generic determinization algorithm,
 * i.e. an algorithm that takes as input a non-deterministic finite automaton and returns a deterministic finite automaton.
 * 
 * It offers a method that takes as input a NFA and returns a DFA.
 * The additional two methods are used to get the name of the algorithm.
 */

#ifndef INCLUDE_DETERMINIZATIONALGORITHM_HPP_
#define INCLUDE_DETERMINIZATIONALGORITHM_HPP_

#include <vector>

#include "Automaton.hpp"
#include "Statistics.hpp"

namespace quicksc {

	class DeterminizationAlgorithm {

    private:
        string m_name;
        string m_abbr;

		map<RuntimeStat, double> m_runtime_stats_values;

	protected:
		map<RuntimeStat, double>& getRuntimeStatsValuesRef();

	public:
        DeterminizationAlgorithm(string abbr, string name);
        virtual ~DeterminizationAlgorithm();            	// Normal virtual method (destructor) -> it can be overwritten

        const string& abbr();
        const string& name();

		virtual void resetRuntimeStatsValues();
		virtual vector<RuntimeStat> getRuntimeStatsList();
		map<RuntimeStat, double> getRuntimeStatsValues();

		virtual Automaton* run(Automaton* nfa) = 0;       	// Pure virtual method -> it must be implemented by the subclasses

	};
}

#endif /* INCLUDE_DETERMINIZATIONALGORITHM_HPP_ */
