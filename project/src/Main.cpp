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

	DEBUG_MARK_PHASE( "Quick Subset Construction - Main" ) {

		Configurations* config;
		DEBUG_MARK_PHASE("Configurations loading") {
			config = new Configurations();
			config->load(CONFIG_FILENAME);
		}

		vector<DeterminizationAlgorithm*> algorithms;
		DEBUG_MARK_PHASE("Algorithms loading") {
			algorithms.push_back(new SubsetConstruction());
			//algorithms.push_back(new EmbeddedSubsetConstruction(config));
			algorithms.push_back(new QuickSubsetConstruction(config));
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
		Previous version of a program to create a specific automaton and test the algorithms on it.

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
		*/
	}

	return 0;
}
