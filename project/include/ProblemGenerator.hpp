/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * ProblemGenerator.hpp
 *
 *
 * This header file contains the declaration of multiple classes:
 * - Problem, which represents a generic problem to be solved with some testing;
 * - DeterminizationProblem, which represents a problem that has to be solved directly by a determinization algorithm (more info later).
 * - ProblemGenerator, which is a class representing the generic problem generator;
 * - RandomnessManager, a utility class that manages the randomness of the problem generator.
 *
 */

#ifndef INCLUDE_PROBLEMGENERATOR_HPP_
#define INCLUDE_PROBLEMGENERATOR_HPP_

#include "AutomataGeneratorDFA.hpp"
#include "AutomataGeneratorNFA.hpp"

namespace quicksc {

	/**
	 * This class represents a generic problem to be solved in this framework with some algorithm.
	 * It is an abstract class; for now, the only concrete class is DeterminizationProblem.
	 * More types of problems may be added in the future (e.g. translation problems from DFA to DFA with a translated alphabet, a procedure that might need a determinization phase).
	 */
	class Problem {

	public:
		typedef enum {
			DETERMINIZATION_PROBLEM,
		} ProblemType;

	private:
		const ProblemType m_type;

	public:
		Problem(ProblemType type) : m_type(type) {};
		~Problem() {};

		ProblemType getType() { return this->m_type; };

	};

	/**
	 * This class represents a specific type of problem, with:
	 * - a NFA as an input;
	 * - a DFA as an output.
	 * The problem is to determinize the input NFA into the output DFA.
	 * This problem can be solved by a determinization algorithm (such as Subset Construction or Quick Subset Construction).
	 */
	class DeterminizationProblem : public Problem {

	private:
		Automaton* m_nfa;

	public:
		DeterminizationProblem(Automaton* nfa);
		~DeterminizationProblem();

		Automaton* getNFA();
	};

	/**
	 * This class has the role of generating the problems on which the algorithms will be tested.
	 * For now, it only creates a DeterminizationProblem with the method "generateDeterminizationProblem", but it may be extended in the future.
	 * 
	 * It uses two other generator classes:
	 * - NFAGenerator, which generates random NFA;
	 * - AlphabetGenerator, which generates random alphabets.
	 */
	class ProblemGenerator {

	private:
		Problem::ProblemType m_problem_type;
		Alphabet m_alphabet;
		NFAGenerator* m_nfa_generator;
		//DFAGenerator* m_dfa_generator;

	public:
		ProblemGenerator(Configurations* configurations);
		~ProblemGenerator();

		Problem* generate();
		DeterminizationProblem* generateDeterminizationProblem();

	};

	/**
	 * This class offers some utility methods to manage the randomness of the problem generator.
	 */
	class RandomnessManager {

	private:
		unsigned long int m_seed;

	public:
		RandomnessManager();
		RandomnessManager(Configurations* configurations);
		~RandomnessManager();

		void newRandomSeed();
		unsigned long int getSeed();
		void setSeed(unsigned long int new_seed);
		void printSeed();
	};

} /* namespace quicksc */

#endif /* INCLUDE_PROBLEMGENERATOR_HPP_ */
