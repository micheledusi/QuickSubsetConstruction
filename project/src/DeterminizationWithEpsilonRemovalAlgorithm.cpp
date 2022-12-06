/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * DeterminizationWithEpsilonRemovalAlgorithm.cpp
 *
 *
 * This file implements a determinization algorithm that uses the epsilon removal algorithm.
 * In the implementation logic, this class is a generic subclass of DeterminizationAlgorithm; it uses two different algorithm
 * to perform the determinization: 
 * - one for the epsilon removal, i.e. an instance of EpsilonRemovalAlgorithm.
 * - one for the determinization, i.e. another instance of DeterminizationAlgorithm. In our case, the latter can be SC, ESC or QSC.
 */

#include "DeterminizationWithEpsilonRemovalAlgorithm.hpp"

#include <chrono>

#include "AutomataDrawer.hpp"

//#define DEBUG_MODE
#include "Debug.hpp"

namespace quicksc {

	/**
	 * Macro function that measures the time spent to execute a block of code.
	 * It stores the time in a variable (declared internally to the macro) whose name can be inserted as a parameter.
	 */
	#define MEASURE_MILLISECONDS( ms_result ) 											\
		unsigned long int ms_result = 0; 												\
		auto CONCAT( ms_result, _start ) = chrono::high_resolution_clock::now(); 		\
		for (	int CONCAT( ms_result, _for_counter ) = 0; 								\
				CONCAT( ms_result, _for_counter ) < 1;									\
				CONCAT( ms_result, _for_counter++ ),									\
				ms_result = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - CONCAT( ms_result, _start )).count() )

    /**
     * Base constructor.
     * It requires two different algorithms:
     * - one for the epsilon removal, i.e. an instance of EpsilonRemovalAlgorithm.
     * - one for the determinization, i.e. another instance of DeterminizationAlgorithm. 
     * 
     * This constructor instantiates the name and abbreviation of the algorithm as the concatenation of the two algorithms used.
     */
    DeterminizationWithEpsilonRemovalAlgorithm::DeterminizationWithEpsilonRemovalAlgorithm(EpsilonRemovalAlgorithm* epsilon_removal_algorithm, DeterminizationAlgorithm* determinization_algorithm) 
    : DeterminizationAlgorithm(
        epsilon_removal_algorithm->abbr() + "+" + determinization_algorithm->abbr(), 
        determinization_algorithm->name() + " with " + epsilon_removal_algorithm->name()
        ), m_epsilon_removal_algorithm(epsilon_removal_algorithm), m_determinization_algorithm(determinization_algorithm) {}

    /**
     * Destructor.
     * ATTENTION: It deletes the two algorithms used.
     */
    DeterminizationWithEpsilonRemovalAlgorithm::~DeterminizationWithEpsilonRemovalAlgorithm() {
        delete this->m_epsilon_removal_algorithm;
        delete this->m_determinization_algorithm;
    }

    void DeterminizationWithEpsilonRemovalAlgorithm::resetRuntimeStatsValues() {
		// Calling parent method
		DeterminizationAlgorithm::resetRuntimeStatsValues();
		// Initializing the values
		map<RuntimeStat, double> stats = this->getRuntimeStatsValuesRef();
		for (RuntimeStat stat : this->getRuntimeStatsList()) {
			stats[stat] = (double) 0;
		}

        // Calling method on the inner determinization algorithm
        this->m_determinization_algorithm->resetRuntimeStatsValues();
    }

    vector<RuntimeStat> DeterminizationWithEpsilonRemovalAlgorithm::getRuntimeStatsList() {
        vector<RuntimeStat> runtime_stats = DeterminizationAlgorithm::getRuntimeStatsList();
        // Adding elements from the inner determinization algorithm
        for (RuntimeStat stat : this->m_determinization_algorithm->getRuntimeStatsList()) {
            runtime_stats.push_back("in-" + stat); // Adding "in-" to distinguish the inner stats from the outer ones
        }
        runtime_stats.push_back(EPSILON_REMOVAL_TIME);
        runtime_stats.push_back(DETERMINIZATION_TIME);
        return runtime_stats;
    }

    /** @override **/
    map<RuntimeStat, double> DeterminizationWithEpsilonRemovalAlgorithm::getRuntimeStatsValues() {
        // Adding elements from the inner determinization algorithm to the map
        for (RuntimeStat inner_stat : this->m_determinization_algorithm->getRuntimeStatsList()) {
            this->getRuntimeStatsValuesRef()["in-" + inner_stat] = this->m_determinization_algorithm->getRuntimeStatsValues()[inner_stat];
        }
        return this->getRuntimeStatsValuesRef();
    }

    /**
     * This method performs the determinization of the given NFA.
     * It uses the two algorithms passed to the constructor.
     * 
     * @param nfa The NFA to determinize, optionally with epsilon transitions.
     * @return The DFA obtained by determinization.
     */
    Automaton* DeterminizationWithEpsilonRemovalAlgorithm::run(Automaton* nfa) {
        Automaton* nfa_clone = nfa->clone();
        Automaton* nfa_without_epsilons, *dfa;  // Declarations

        DEBUG_MARK_PHASE("Epsilon removal with <%s>", this->m_epsilon_removal_algorithm->name().c_str()) {
            // Note: the epsilon removal algorithm is applied to the clone of the NFA
            // This procedure works directly on the NFA, so we need to clone it
            MEASURE_MILLISECONDS( er_time ) {
                nfa_without_epsilons = this->m_epsilon_removal_algorithm->run(nfa_clone);
            }
            this->getRuntimeStatsValuesRef()[EPSILON_REMOVAL_TIME] = er_time;
        }

        DEBUG_LOG("NFA without epsilons:");
        IF_DEBUG_ACTIVE(AutomataDrawer* drawer = new AutomataDrawer(nfa_without_epsilons); )
        DEBUG_LOG("%s", drawer->asString().c_str());

        DEBUG_MARK_PHASE("Determinization with <%s>", this->m_determinization_algorithm->name().c_str()) {
            MEASURE_MILLISECONDS( det_time ) {
                dfa = this->m_determinization_algorithm->run(nfa_without_epsilons);
            }
            this->getRuntimeStatsValuesRef()[DETERMINIZATION_TIME] = det_time;
        }

        delete nfa_without_epsilons;  // The NFA without epsilon transitions is removed
        return dfa;
    }

}