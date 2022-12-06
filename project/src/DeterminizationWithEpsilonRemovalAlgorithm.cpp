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

#include "AutomataDrawer.hpp"

//#define DEBUG_MODE

#include "Debug.hpp"

namespace quicksc {

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
        this->m_determinization_algorithm->resetRuntimeStatsValues();
    }

    vector<RuntimeStat> DeterminizationWithEpsilonRemovalAlgorithm::getRuntimeStatsList() {
        return this->m_determinization_algorithm->getRuntimeStatsList();
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
            nfa_without_epsilons = this->m_epsilon_removal_algorithm->run(nfa_clone);
        }

        DEBUG_LOG("NFA without epsilons:");
        IF_DEBUG_ACTIVE(AutomataDrawer* drawer = new AutomataDrawer(nfa_without_epsilons); )
        DEBUG_LOG("%s", drawer->asString().c_str());

        DEBUG_MARK_PHASE("Determinization with <%s>", this->m_determinization_algorithm->name().c_str()) {
            dfa = this->m_determinization_algorithm->run(nfa_without_epsilons);
        }

        delete nfa_without_epsilons;  // The NFA without epsilon transitions is removed
        return dfa;
    }

}