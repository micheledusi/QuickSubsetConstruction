/**
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * DeterminizationWithEpsilonRemovalAlgorithm.hpp
 * 
 * 
 * This header file contains the declaration of the class DeterminizationWithEpsilonRemovalAlgorithm.
 * This class is a generic subclass of DeterminizationAlgorithm; it uses two different algorithm
 * to perform the determinization:
 * - one for the epsilon removal, i.e. an instance of EpsilonRemovalAlgorithm.
 * - one for the determinization, i.e. another instance of DeterminizationAlgorithm. In our case, the latter can be SC, ESC or QSC.
 */

#ifndef DETERMINIZATION_WITH_EPSILON_REMOVAL_ALGORITHM_HPP
#define DETERMINIZATION_WITH_EPSILON_REMOVAL_ALGORITHM_HPP

#include "Automaton.hpp"
#include "Statistics.hpp"
#include "DeterminizationAlgorithm.hpp"
#include "EpsilonRemovalAlgorithm.hpp"

namespace quicksc {

    class DeterminizationWithEpsilonRemovalAlgorithm : public DeterminizationAlgorithm {

    private:
        EpsilonRemovalAlgorithm* m_epsilon_removal_algorithm;
        DeterminizationAlgorithm* m_determinization_algorithm;

    public:
        DeterminizationWithEpsilonRemovalAlgorithm(EpsilonRemovalAlgorithm* epsilon_removal_algorithm, DeterminizationAlgorithm* determinization_algorithm);
        ~DeterminizationWithEpsilonRemovalAlgorithm();

        void resetRuntimeStatsValues();
        vector<RuntimeStat> getRuntimeStatsList();

        Automaton* run(Automaton* nfa);

    };

}


#endif // DETERMINIZATION_WITH_EPSILON_REMOVAL_ALGORITHM_HPP