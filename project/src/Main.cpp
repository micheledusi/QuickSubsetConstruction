/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Main.cpp
 *
 *
 * This file contains the main function of the program.
 * It is used to test the algorithms implemented in the project, managing the execution flow in a centralized way.
 * At the moment, it compares the exectution of two algorithms: the Quick Subset Construction algorithm and the Subset Construction algorithm.
 * It tests both on a determinizationi problem, i.e. a problem where a NFA has to be converted into a DFA.
 * 
 * For further information, refer to the respective classes.
 *
 */

#include <iostream>
#include <tuple>

#include "Automaton.hpp"
#include "AutomataDrawer.hpp"
#include "DeterminizationAlgorithm.hpp"
#include "DeterminizationWithEpsilonRemovalAlgorithm.hpp"
#include "EmbeddedSubsetConstruction.hpp"
#include "ProblemSolver.hpp"
#include "Properties.hpp"
#include "QuickSubsetConstruction.hpp"
#include "SubsetConstruction.hpp"

#include "Debug.hpp"

using std::set;

using namespace quicksc;

int main(int argc, char **argv) {

	DEBUG_MARK_PHASE( "Quick Subset Construction - Main" ) {

		Configurations* config;
		DEBUG_MARK_PHASE("Configurations loading") {
			config = new Configurations();
			config->load(CONFIG_FILENAME);
		}

		vector<DeterminizationAlgorithm*> algorithms;
		DEBUG_MARK_PHASE("Algorithms loading") {
			DeterminizationAlgorithm* sc = new SubsetConstruction();
			//DeterminizationAlgorithm* esc = new EmbeddedSubsetConstruction(config);
			DeterminizationAlgorithm* qsc = new QuickSubsetConstruction(config);
			EpsilonRemovalAlgorithm* ner = new NaiveEpsilonRemovalAlgorithm();
			EpsilonRemovalAlgorithm* ger = new GlobalEpsilonRemovalAlgorithm();
			DeterminizationAlgorithm* sc_with_ner = new DeterminizationWithEpsilonRemovalAlgorithm(ner, sc);
			DeterminizationAlgorithm* sc_with_ger = new DeterminizationWithEpsilonRemovalAlgorithm(ger, sc);
			DeterminizationAlgorithm* qsc_with_ner = new DeterminizationWithEpsilonRemovalAlgorithm(ner, qsc);
			DeterminizationAlgorithm* qsc_with_ger = new DeterminizationWithEpsilonRemovalAlgorithm(ger, qsc);

			algorithms.push_back(sc);
			//algorithms.push_back(esc);
			algorithms.push_back(qsc);
			algorithms.push_back(sc_with_ner);
			algorithms.push_back(sc_with_ger);
			algorithms.push_back(qsc_with_ner);
			algorithms.push_back(qsc_with_ger);
		}

		do {
			std::cout << std::endl << "_______________________________________________________________________|" << std::endl << std::endl << std::endl;

			// Creating the problem solver instance
			ProblemSolver solver = ProblemSolver(config, algorithms);

			// Solving the series of problems. The length of the series is specified in the configuration file (as the "Testcases" number)
			solver.solveSeries(config->valueOf<int>(Testcases));

			// Presenting the results and the statistics
			solver.getResultCollector()->presentResults();
			std::cout << std::endl;
		} while (config->nextTestCase());
		

		/*
		// Previous version of a program to create a specific automaton and test the algorithms on it.

		Automaton* nfa = new Automaton();
		State* s0 = new State("0");
		State* s1 = new State("1");
		State* s2 = new State("2", true);
		State* s3 = new State("3");
		State* s4 = new State("4");
		
		nfa->addState(s0);
		nfa->addState(s1);
		nfa->addState(s2);
		nfa->addState(s3);
		nfa->addState(s4);
		
		s0->connectChild(EPSILON, s2);
		s0->connectChild("d", s1);
		
		s1->connectChild(EPSILON, s2);
		s1->connectChild(EPSILON, s3);
		
		s2->connectChild("e", s3);
		
		s3->connectChild(EPSILON, s3);
		s3->connectChild("b", s4);
		
		s4->connectChild(EPSILON, s3);
		s4->connectChild("d", s3);
		s4->connectChild("d", s1);

		s3->connectChild("b", s2);
		
		nfa->setInitialState(s0);
		// s0->initDistancesRecursively(0);	// Called automatically by Automaton::setInitialState()

		AutomataDrawer nfa_drawer = AutomataDrawer(nfa);
		std::cout << nfa_drawer.asString() << std::endl;
		
		//QuickSubsetConstruction* qsc = new QuickSubsetConstruction(config);
		DeterminizationAlgorithm* erem_sc = new DeterminizationWithEpsilonRemovalAlgorithm(new GlobalEpsilonRemovalAlgorithm(), new SubsetConstruction());
		Automaton* dfa = erem_sc->run(nfa);
		
		AutomataDrawer dfa_drawer = AutomataDrawer(dfa);
		std::cout << dfa_drawer.asString() << std::endl;
		// */
	}

	return 0;
}
