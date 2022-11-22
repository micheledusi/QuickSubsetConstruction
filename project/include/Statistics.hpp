/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Statistics.hpp
 *
 * 
 * This header file contains the definition of the Statistics class, a class
 * that is used to collect statistics about the results of the determinization tests.
 *
 * The aggregated statistics comprehends different information about the result of the determinization process
 * or the process itself, such as the number of states, the number of transitions, the number of singularities and so on.
 * For further information, see the list below.
 */

#ifndef INCLUDE_STATISTICS_HPP_
#define INCLUDE_STATISTICS_HPP_

#include <string>

namespace quicksc {

    /**
     * These statistics are collected from a single determinization test.
     * They're processed and aggregated with common operations like the average, the sum, the maximum and so on.
     * 
     * \note Every statistic has to be expressed as a double value.
     */
    enum ResultStat {
        SOL_SIZE,		// Size of the solution automaton obtained from the algorithm
        SOL_GROWTH,		// Ratio between the solution size and the original automaton size
        SOL_TR_COUNT,   // Number of transitions in the solution automaton

        RESULTSTAT_END,
    };

    /**
     * These statistics are collected from a single determinization test and refer to a single algorithm.
     * They're processed and aggregated with common operations like the average, the sum, the maximum and so on.
     * 
     * \note Every statistic has to be expressed as a double value.
     */
    enum AlgorithmStat {
        CORRECTNESS,	// Correctness percentage of the solution automaton
        EXECUTION_TIME,	// Time spent by the algorithm to compute the solution automaton
        EMPIRICAL_GAIN,	// Experimental gain of the algorithm, i.e. the ratio between the execution time and the minimum time

        ALGORITHMSTAT_END,
    };

    /**
     * These statistics are collected from a single determinization test and refer to a single execution of a determinization algorithm.
     * They're processed and aggregated with common operations like the average, the sum, the maximum and so on.
     * Since they depend and are built from the algorithm itself (specifically, from the instance of the DeterminizationAlgorithm class), they're defined
     * as strings, and not enum. This way, every algorithm can define its own statistics.
     * 
     * \note Every statistic has to be expressed as a double value.
     */
    using RuntimeStat = std::string;

} /* namespace quicksc */

#endif /* INCLUDE_STATISTICS_HPP_ */
