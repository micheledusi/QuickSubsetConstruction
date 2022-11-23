/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * DeterminizationAlgorithm.cpp
 *
 *
 * This file implements the DeterminizationAlgorithm abstract class.
 * The definition of the abstract class requires one method that will be implemented by the subclasses.
 * In addition, the constructor requires to specify an abbreviation for the algorithm and a complete name.
 */

#include "DeterminizationAlgorithm.hpp"

#include <algorithm>

namespace quicksc {

    /**
     * Base constructor.
     * It instantiates the name and abbreviation of the algorithm.
     */
    DeterminizationAlgorithm::DeterminizationAlgorithm(string abbr, string name) {
        //transform(abbr.begin(), abbr.end(), abbr.begin(), ::tolower);
        this->m_abbr = abbr;
        this->m_name = name;

        // Resetting the statistics
        this->m_runtime_stats_values = map<RuntimeStat, double>();
        this->resetRuntimeStatsValues();
    }

    /**
     * Empty destructor.
     */
    DeterminizationAlgorithm::~DeterminizationAlgorithm() {}

    const string& DeterminizationAlgorithm::abbr() {
        return this->m_abbr;
    };

    const string& DeterminizationAlgorithm::name() {
        return this->m_name;
    };

    /**
     * Resets the runtime statistics.
     * The runtime statistics, by definition, are related to a single execution. They are not maintained from one execution to the next. 
     * For this reason, this method is called before each start of the algorithm using the "run" method.
     */
    void DeterminizationAlgorithm::resetRuntimeStatsValues() {
        this->m_runtime_stats_values = map<RuntimeStat, double>();
    };

    /**
     * Returns a vector of runtime statistics calculated by the algorithm.
     * Each subclass must implement (if it uses this functionality) this method so that it returns the statistics used.
     */
	vector<RuntimeStat> DeterminizationAlgorithm::getRuntimeStatsList() {
        return vector<RuntimeStat>();
    }
    
    /**
     * Returns the runtime statistics map, as value.
     * This is calculated at each execution, based on the results of the execution. 
     * The runtime statistics are special statistics that are computed directly by the algorithm, and not a posteriori.
     */
    map<RuntimeStat, double> DeterminizationAlgorithm::getRuntimeStatsValues() {
        return this->m_runtime_stats_values;
    };

    /**
     * Returns a reference to the runtime statistics map.
     * This is calculated at each execution, based on the results of the execution. 
     * The runtime statistics are special statistics that are computed directly by the algorithm, and not a posteriori.
     */
    map<RuntimeStat, double>& DeterminizationAlgorithm::getRuntimeStatsValuesRef() {
        return this->m_runtime_stats_values;
    };

}
