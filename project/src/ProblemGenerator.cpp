/*
 * ProblemGenerator.hpp
 *
 * Project: TranslatedAutomata
 *
 * Implementazione della classe ProblemGenerator, avente la responsabilità
 * di generare istanze di problemi secondo differenti caratteristiche.
 * Ogni problema prevede:
 * - Un NFA di partenza, costruito su un alfabeto predefinito e secondo le configurazioni richieste
 *
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
	 * Costruttore della "struttura" DeterminizationProblem.
	 * Richiede un automa NFA.
	 */
	DeterminizationProblem::DeterminizationProblem(Automaton* nfa)
	: Problem(DETERMINIZATION_PROBLEM) {

		DEBUG_ASSERT_NOT_NULL(nfa);
		this->m_nfa = nfa;
	}

	/**
	 * Distruttore.
	 * Un problema possiede i propri attributi; pertanto, quando viene eliminato,
	 * richiama il distruttore di tali attributi.
	 */
	DeterminizationProblem::~DeterminizationProblem() {
		delete this->m_nfa;
	}

	/**
	 * Restituisce l'automa NFA del problema di determinizzazione.
	 */
	Automaton* DeterminizationProblem::getNFA() {
		return this->m_nfa;
	}

	/**
	 * Costruttore di un generatore di problemi.
	 * Si occupa di istanziare i generatori delegati.
	 * Inoltre, imposta alcuni parametri per la randomicità del programma.
	 */
	ProblemGenerator::ProblemGenerator(Configurations* configurations) {
		// Istanzio un nuovo gestore di randomicità
		RandomnessManager* random = new RandomnessManager();
		//random->setSeed(1613950262);
		random->printSeed();
		// Una volta terminato il setup dei semi randomici, posso eliminarlo.
		delete random;

		// Impostazione dell'alfabeto comune
		AlphabetGenerator* alphabet_generator = new AlphabetGenerator();
		alphabet_generator->setCardinality((configurations->valueOf<unsigned int>(AlphabetCardinality)));
		DEBUG_LOG("Cardinalità dell'alfabeto impostata a: %u", alphabet_generator->getCardinality());
		this->m_alphabet = alphabet_generator->generate();
		delete alphabet_generator;

		this->m_problem_type = (Problem::ProblemType) configurations->valueOf<int>(ProblemType);

		// Istanzio i generatori delegati
		switch (this->m_problem_type) {

		case Problem::DETERMINIZATION_PROBLEM :
			//this->m_dfa_generator = NULL;
			this->m_nfa_generator = new NFAGenerator(this->m_alphabet, configurations);
			break;

		default :
			DEBUG_LOG_ERROR("Impossibile interpretare il valore %d come istanza dell'enum ProblemType", this->m_problem_type);
			break;

		}

	}

	/**
	 * Distruttore.
	 */
	ProblemGenerator::~ProblemGenerator() {
		DEBUG_MARK_PHASE("Eliminazione dell'oggetto ProblemGenerator") {
			//if (this->m_dfa_generator != NULL) delete this->m_dfa_generator;
			if (this->m_nfa_generator != NULL) delete this->m_nfa_generator;
		}
	}

	/**
	 * Genera un nuovo problema del tipo specifico richiesto, richiamando il metodo apposito.
	 */
	Problem* ProblemGenerator::generate() {
		switch (this->m_problem_type) {

		case Problem::DETERMINIZATION_PROBLEM :
			return this->generateDeterminizationProblem();

		default :
			DEBUG_LOG_ERROR("Valore %d non riconosciuto all'interno dell'enumerazione ProblemType", this->m_problem_type);
			return NULL;
		}
	}

	/**
	 * Genera un problema di determinizzazione, richiamando i generatori delegati.
	 */
	DeterminizationProblem* ProblemGenerator::generateDeterminizationProblem() {
		DEBUG_LOG("Generazione dell'automa NFA");
		Automaton* automaton = this->m_nfa_generator->generateAutomaton();

		return new DeterminizationProblem(automaton);
	}

	/* Classe RandomnessManager */

	/**
	 * Costruttore.
	 * Genera un nuovo seme.
	 */
	RandomnessManager::RandomnessManager() {
		this->m_seed = 0;	// Inizializzazione fittizia, viene sovrascritto nella funzione successiva
		this->newSeed();
	}

	/**
	 * Distruttore
	 */
	RandomnessManager::~RandomnessManager() {};

	/**
	 * Genera un nuovo seme causale sulla base dell'istante in cui ci si trova.
	 */
	void RandomnessManager::newSeed() {
		this->m_seed = time(0);
		srand(this->m_seed);
		DEBUG_LOG("Impostazione di un nuovo seme casuale: %lu", this->m_seed);
	}

	/**
	 * Restituisce il seme attuale.
	 */
	unsigned long int RandomnessManager::getSeed() {
		return this->m_seed;
	}

	/**
	 * Imposta il valore passato come argomento come nuovo seme.
	 */
	void RandomnessManager::setSeed(unsigned long int new_seed) {
		this->m_seed = new_seed;
		srand(this->m_seed);
	}

	/**
	 * Stampa il seme attuale.
	 */
	void RandomnessManager::printSeed() {
		printf("Seme attuale = " COLOR_BLUE("%lu") "\n", this->m_seed);
	}

} /* namespace quicksc */
