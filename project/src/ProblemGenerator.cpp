/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * ProblemGenerator.hpp
 *
 *
 * This file implements the ProblemGenerator class.
 * The ProblemGenerator class is used to generate a series of problems to be solved by the algorithms.
 * The problems are generated randomly, according to the configuration file.
 */

#include "ProblemGenerator.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>

#include "AlphabetGenerator.hpp"
#include "Configurations.hpp"
#include "Debug.hpp"

namespace quicksc {
	
	/**
	 * Constructor of the DeterminizationProblem structure.
	 * It requires a NFA to be determinized.
	*/
	DeterminizationProblem::DeterminizationProblem(Automaton* nfa)
	: Problem(DETERMINIZATION_PROBLEM) {

		DEBUG_ASSERT_NOT_NULL(nfa);
		this->m_nfa = nfa;
	}

	/**
	 * Destructor.
	 * A problem owns its attributes; therefore, when it is deleted, it calls the destructor of such attributes.
	 */
	DeterminizationProblem::~DeterminizationProblem() {
		delete this->m_nfa;
	}

	/**
	 * Returns the NFA to be determinized.
	 */
	Automaton* DeterminizationProblem::getNFA() {
		return this->m_nfa;
	}

	/**
	 * Constructor of a problem generator.
	 * It is responsible for instantiating the delegated generators.
	 * In addition, it sets some parameters for the randomness of the program.
	 */
	ProblemGenerator::ProblemGenerator(Configurations* configurations) {
		// Instantiating the random manager
		RandomnessManager* random = new RandomnessManager();
		//random->setSeed(1613950262);
		random->printSeed();
		// Once the random manager ended its job, it can be deleted
		delete random;

		// Setting the common alphabet
		AlphabetGenerator* alphabet_generator = new AlphabetGenerator();
		alphabet_generator->setCardinality((configurations->valueOf<unsigned int>(AlphabetCardinality)));
		DEBUG_LOG("Alphabet cardinality set to: %u", alphabet_generator->getCardinality());
		this->m_alphabet = alphabet_generator->generate();
		delete alphabet_generator;

		this->m_problem_type = (Problem::ProblemType) configurations->valueOf<int>(ProblemType);

		switch (this->m_problem_type) {

		case Problem::DETERMINIZATION_PROBLEM :
			//this->m_dfa_generator = NULL;
			this->m_nfa_generator = new NFAGenerator(this->m_alphabet, configurations);
			break;

		default :
			DEBUG_LOG_ERROR("Cannot parse the value %d as instance of the enumeration ProblemType", this->m_problem_type);
			break;

		}

	}

	/**
	 * Destructor.
	 */
	ProblemGenerator::~ProblemGenerator() {
		DEBUG_MARK_PHASE("Deleting the nfa generator") {
			//if (this->m_dfa_generator != NULL) delete this->m_dfa_generator;
			if (this->m_nfa_generator != NULL) delete this->m_nfa_generator;
		}
	}

	/**
	 * Generates a new problem of the specific type requested, calling the appropriate method.
	 */
	Problem* ProblemGenerator::generate() {
		switch (this->m_problem_type) {

		case Problem::DETERMINIZATION_PROBLEM :
			return this->generateDeterminizationProblem();

		default :
			DEBUG_LOG_ERROR("Cannot parse the value %d as instance of the enumeration ProblemType", this->m_problem_type);
			return NULL;
		}
	}

	/**
	 * Generates a determinization problem, calling the delegated generators.
	 */
	DeterminizationProblem* ProblemGenerator::generateDeterminizationProblem() {
		DEBUG_LOG("NFA Generation");
		Automaton* automaton = this->m_nfa_generator->generateAutomaton();

		return new DeterminizationProblem(automaton);
	}

	/* Class RandomnessManager */

	/**
	 * Constructor.
	 * It sets the seed of the random number generator.
	 */
	RandomnessManager::RandomnessManager() {
		this->m_seed = 0;	// Fictional seed, it's overwritten by the next call to the newSeed() method
		this->newSeed();
	}

	/**
	 * Destructor.
	 */
	RandomnessManager::~RandomnessManager() {};

	/**
	 * Generates a new seed for the random number generator.
	 * The seed depends on the current time.
	 */
	void RandomnessManager::newSeed() {
		this->m_seed = time(0);
		srand(this->m_seed);
		DEBUG_LOG("Setting a new random seed: %lu", this->m_seed);
	}

	/**
	 * Returns the current seed.
	 */
	unsigned long int RandomnessManager::getSeed() {
		return this->m_seed;
	}

	/**
	 * Sets the seed of the random number generator.
	 */
	void RandomnessManager::setSeed(unsigned long int new_seed) {
		this->m_seed = new_seed;
		srand(this->m_seed);
	}

	/**
	 * Prints the current seed.
	 */
	void RandomnessManager::printSeed() {
		printf("Seme attuale = " COLOR_BLUE("%lu") "\n", this->m_seed);
	}

} /* namespace quicksc */
