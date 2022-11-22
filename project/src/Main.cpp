/*
 * Main.cpp
 *
 * Project: TranslatedAutomata
 *
 * Main del progetto. Gestisce il flusso di esecuzione in maniera
 * centralizzata, richiamando le funzioni necessarie.
 * Attualmente si occupa di generare un automa secondo i parametri impostati,
 * quindi genera una traduzione e infine esegue l'algoritmo "Embedded
 * Subset Construction".
 * Si vedano le rispettive classi per informazioni più dettagliate.
 *
 */

#include <iostream>
#include <tuple>

#include "Automaton.hpp"
#include "AutomataDrawer.hpp"
#include "Debug.hpp"
#include "DeterminizationAlgorithm.hpp"
#include "EmbeddedSubsetConstruction.hpp"
#include "ProblemSolver.hpp"
#include "Properties.hpp"
#include "QuickSubsetConstruction.hpp"
#include "SubsetConstruction.hpp"

using std::set;

using namespace quicksc;

int main(int argc, char **argv) {

	DEBUG_MARK_PHASE( "Translated Automaton - Main" ) {

		Configurations* config;
		DEBUG_MARK_PHASE("Caricamento delle configurazioni") {
			// Creazione delle configurazioni
			config = new Configurations();
			config->load(CONFIG_FILENAME);
		}

		vector<DeterminizationAlgorithm*> algorithms;
		DEBUG_MARK_PHASE("Creazione degli algoritmi") {
			algorithms.push_back(new SubsetConstruction());
			//algorithms.push_back(new EmbeddedSubsetConstruction(config));
			algorithms.push_back(new QuickSubsetConstruction(config));
		}

		do {
			std::cout << std::endl << "_______________________________________________________________________|" << std::endl << std::endl << std::endl;

			// Creazione del sistema di risoluzione
			ProblemSolver solver = ProblemSolver(config, algorithms);

			// Risoluzione effettiva
			solver.solveSeries(config->valueOf<int>(Testcases));

			// Presentazione delle statistiche risultanti
			solver.getResultCollector()->presentResults();
			std::cout << std::endl;

		} while (config->nextTestCase());

		// Automaton* nfa = new Automaton();
		// State* s5 = new State("5");
		// State* s6 = new State("6");
		// State* s7 = new State("7");
		// State* s8 = new State("8");
		// State* s9 = new State("9", true);
		//
		// s5->connectChild(EPSILON, s7);
		// s5->connectChild("d", s6);
		//
		// s6->connectChild(EPSILON, s7);
		// s6->connectChild(EPSILON, s8);
		//
		// s7->connectChild("e", s8);
		//
		// s8->connectChild(EPSILON, s8);
		// s8->connectChild("b", s9);
		//
		// s9->connectChild(EPSILON, s8);
		// s9->connectChild("d", s8);
		// s9->connectChild("d", s7);
		//
		// nfa->addState(s5);
		// nfa->addState(s6);
		// nfa->addState(s7);
		// nfa->addState(s8);
		// nfa->addState(s9);
		//
		// nfa->setInitialState(s5);
		// s5->initDistancesRecursively(0);
		//
		// QuickSubsetConstruction* qsc = new QuickSubsetConstruction(config);
		// Automaton* dfa = qsc->run(nfa);
		//
		// AutomataDrawer drawer = AutomataDrawer(dfa);
		// std::cout << drawer.asString() << std::endl;

	}

	return 0;
}
