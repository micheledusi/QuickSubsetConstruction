/*
 * ProblemSolver.cpp
 *
 * Modulo che si occupa di risolvere problemi "ad alto livello".
 * I problemi vengono generati tramite un ProblemGenerator, vengono
 * risolti da i due algoritmi previsti all'interno del programma, e
 * i risultati vengono archiviati all'interno di un ResultCollector.
 */

#include "ProblemSolver.hpp"

#include <cstdio>
#include <chrono>
#include <list>

#include "Debug.hpp"
#include "Properties.hpp"

using namespace std;

namespace quicksc {

	/**
	 * Macro function per concatenare la valutazione dei due argomenti.
	 */
	#define CONCAT( x, y ) _CONCAT( x, y )
	#define _CONCAT( x, y ) x ## y

	/**
	 * Macro function che si occupa di misurare il tempo impiegato per eseguire un blocco di codice.
	 * Memorizza il tempo in una variabile (dichiarata internamente alla macro) il cui nome può essere
	 * inserito come parametro.
	 */
	#define MEASURE_MILLISECONDS( ms_result ) 											\
		unsigned long int ms_result = 0; 												\
		auto CONCAT( ms_result, _start ) = chrono::high_resolution_clock::now(); 		\
		for (	int CONCAT( ms_result, _for_counter ) = 0; 								\
				CONCAT( ms_result, _for_counter ) < 1;									\
				CONCAT( ms_result, _for_counter++ ),									\
				ms_result = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - CONCAT( ms_result, _start )).count() )

	/**
	 * Costruttore.
	 * Richiede le configurazioni del programma, la lista di algoritmi di determinizzazione con cui risolvere i problemi
	 * L'algoritmo con cui si effettuano i confronti viene di default assegnato al primo algoritmo della lista.
	 */
	ProblemSolver::ProblemSolver(Configurations* configurations, const vector<DeterminizationAlgorithm*>& algorithms)
			: algorithms(algorithms) {

		// Creazione del generatore e dell'analizzatore di risultati
		this->generator = new ProblemGenerator(configurations);
		this->collector = new ResultCollector(configurations, this->algorithms);

		// Uso il primo algoritmo della lista come algoritmo di riferimento
		this->benchmark_algorithm_pointer = this->algorithms.front();
	}

	/**
	 * Distruttore.
	 * Distrugge i riferimenti alle due classi relative agli algoritmi da confrontare.
	 */
	ProblemSolver::~ProblemSolver() {
		DEBUG_MARK_PHASE("Eliminazione del risolutore") {
			delete this->generator;
			delete this->collector;
//			delete this->algorithms; // NOPE perché reference
		}
	}

	/**
	 * Restituisce il collettore di risultati.
	 */
	ResultCollector* ProblemSolver::getResultCollector() {
		return this->collector;
	}

	/**
	 * Risolve un singolo problema di determinizzazione passato come parametro.
	 * La risoluzione avviene attraverso tre algoritmi:
	 * - Subset Construction
	 * - Quick Subset Construction
	 * - Embedded Subset Construction
	 */
	void ProblemSolver::solve(DeterminizationProblem* problem) {
		DEBUG_ASSERT_NOT_NULL(problem);
		Result* result = new Result();
		result->original_problem = problem;
		result->benchmark_algorithm = this->benchmark_algorithm_pointer;

		for (DeterminizationAlgorithm* algo : this->algorithms) {
			// Reset delle statistiche di esecuzione
			algo->resetRuntimeStatsValues();

			DEBUG_MARK_PHASE("Esecuzione dell'algoritmo") {
				// Fase di costruzione
				MEASURE_MILLISECONDS( time ) {
					result->solutions[algo] = algo->run(problem->getNFA()); // Chiamata all'algoritmo
				}
				// Statistiche
				result->times[algo] = time;
			}
			result->runtime_stats[algo] = algo->getRuntimeStatsValues();
		}

		this->collector->addResult(result);
	}

	/**
	 * Risolve una singola istanza di un generico problema, agendo a seconda del tipo del problema.
	 * Nella pratica delega la risoluzione ai metodi specifici per la tipologia del problema.
	 *
	 * Nota: al momento è prevista un'unica tipologia di problemi: problemi di determinizzazione.
	 + I precedenti problemi di traduzione, che si applicavano all'algoritmo ESC, sono stati rimossi.
	 */
	void ProblemSolver::solve(Problem* problem) {
		DEBUG_ASSERT_NOT_NULL(problem);
		switch (problem->getType()) {

		case Problem::DETERMINIZATION_PROBLEM :
			// Al momento questa è l'unica possibilità.
			return this->solve((DeterminizationProblem*) problem);

		default :
			DEBUG_LOG_ERROR("Impossibile identificare il valore %d come istanza dell'enum ProblemType", problem->getType());
			throw "Valore sconosciuto per l'enum ProblemType";
		}
	}

	/**
	 * Risolver un singolo problema generato casualmente mediante
	 * il generatore passato come argomento al costruttore.
	 */
	void ProblemSolver::solve() {
		Problem* problem = this->generator->generate();
		DEBUG_ASSERT_NOT_NULL(problem);
		this->solve(problem);
	}

	#define BARWIDTH 70
	#define BAR_COLOR(bar_char) COLOR_CYAN(bar_char)
	void printProgressBar(float progress) {
		std::cout << "[";
		int pos = BARWIDTH * progress;
		for (int i = 0; i < BARWIDTH; ++i) {
			if (i < pos) std::cout << BAR_COLOR("=");
			else if (i == pos) std::cout << BAR_COLOR(">");
			else std::cout << " ";
		}
		std::cout << "] " << int(progress * 100.0) << " %\r";
		std::cout.flush();
	}

	/**
	 * Risolve una sequenza di problemi generati casualmente.
	 */
	void ProblemSolver::solveSeries(unsigned int number) {
		DEBUG_MARK_PHASE("Risoluzione di una serie di problemi") {
		std::cout << "Solving " << std::to_string(number) << " problems...\n";
		printProgressBar(0);
		for (int i = 0; i < number; i++) {
			this->solve();
			printProgressBar(float(i+1) / number);
			DEBUG_LOG_SUCCESS("Risolto il problema (%d)!", (i+1));
		}
		std::cout << std::endl;
		}
	}

} /* namespace quicksc */
