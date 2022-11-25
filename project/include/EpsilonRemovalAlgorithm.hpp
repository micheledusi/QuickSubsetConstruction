/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * EpsonilonRemovalAlgorithm.hpp
 *
 * 
 * This header file contains the definition of the class EpsilonRemovalAlgorithm,
 * a generic algorithm for removing epsilon transitions from an e-NFA.
 */

#ifndef INCLUDE_EPSILONREMOVALALGORITHM_HPP_
#define INCLUDE_EPSILONREMOVALALGORITHM_HPP_

#include "Automaton.hpp"

using namespace std;

namespace quicksc {

    class EpsilonRemovalAlgorithm {

    private:
        string m_name;
        string m_abbr;

    public:
        EpsilonRemovalAlgorithm(string abbr, string name);
        virtual ~EpsilonRemovalAlgorithm();
        
        const string& abbr();
        const string& name();

        virtual Automaton* run(Automaton* e_nfa) = 0;

    };

    /**
     * This class implements the "epsilon removal" algorithm in a simple way.
     * Each epsilon transition is managed one by one, without aggregating interventions over transitions.
     */
    class NaiveEpsilonRemovalAlgorithm : public EpsilonRemovalAlgorithm {

    public:
        NaiveEpsilonRemovalAlgorithm();
        virtual ~NaiveEpsilonRemovalAlgorithm();

        Automaton* run(Automaton* e_nfa);

    };

    /**
     * This class implements the "epsilon removal" algorithm in a more soficied way.
     * Epsilon transitions are managed with the epsilon-closure, aggregating interventions over transitions.
     * This algorithm can delete multiple epsilon transitions at once.
     */
    class ClosuringEpsilonRemovalAlgorithm : public EpsilonRemovalAlgorithm {

    public:
        ClosuringEpsilonRemovalAlgorithm();
        virtual ~ClosuringEpsilonRemovalAlgorithm();

        Automaton* run(Automaton* e_nfa);

    };

} /* namespace quicksc */


#endif /* INCLUDE_EPSILONREMOVALALGORITHM_HPP_ */