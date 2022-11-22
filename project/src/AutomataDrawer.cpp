/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataDrawer.cpp
 *
 * 
 * This module manages the representation of an automaton in a graphical way, may it be in console or in a file.
 */

#include <fstream>

#include "AutomataDrawer.hpp"

#include "Debug.hpp"
#include "State.hpp"

namespace quicksc {


	/**
	 * Constructor.
	 * Requires the automaton to be drawn as input.
	 * Precondition: the automaton must not be null.
	 */
	AutomataDrawer::AutomataDrawer(Automaton* automaton) {
		this->m_automaton = automaton;
	}

	/**
	 * Destructor.
	 * 
	 * \todo Check if the automaton must be deleted.
	 * \todo Check if this does not cause memory leaks or other problems.
	 */
	AutomataDrawer::~AutomataDrawer() {
		DEBUG_LOG("Sto eliminando l'oggetto \"Automata Drawer\"");
	}

	/**
	 * Returns a textual description of all the states belonging to the automaton, highlighting the initial state.
	 * It is important that each state has been instantiated correctly.
	 * The initial state must have been set correctly.
	 * 
	 * The textual description can be displayed on the terminal. Note: ANSI escape characters are used.
	 */
	string AutomataDrawer::asString() {
    	string result = "";
    	result += "AUTOMATON (size = " + std::to_string(this->m_automaton->size()) + ")\n";
        result += "Initial state: " + this->m_automaton->getInitialState()->getName() + '\n';

        // For each state, print its name and its transitions
        for (auto s : this->m_automaton->getStatesVector()) {
            result += s->toString();
        }

        return result;
	}

	/**
	 * Writes the textual description of all the states belonging to the automaton, highlighting the initial state.
	 * It is important that each state has been instantiated correctly. Make sure that the initial state is set correctly.
	 * 
	 * The output is written to a file as a DOT description of the automaton.
	*/
	void AutomataDrawer::asDotFile(string filename) {
		std::ofstream out(filename, std::ios_base::out | std::ios_base::trunc);

		if (!out.is_open()) {
			DEBUG_LOG_ERROR("Impossibile scrivere il file \"%s\"", filename.c_str());
			return;
		}

		// Prefix for the automaton representation
		out << "digraph finite_state_machine {\n"
				"rankdir=LR;\n"
				"size=\"8,5\"\n";

		// Representation of all the states
		for (auto state : this->m_automaton->getStatesVector()) {
			// Checks if the state is a final state
			if (state->isFinal()) {
				out << "node [shape = doublecircle, label = \"" << state->getName() << "\", fontsize = 10] \"" << state->getName() << "\";\n";
			} else {
				out << "node [shape = circle, label = \"" << state->getName() << "\", fontsize = 10] \"" << state->getName() << "\";\n";
			}
		}

		// Representation of the initial state
		out << "node [shape = point]; init\n";
		out << "init -> \"" << this->m_automaton->getInitialState()->getName() << "\"\n";

		// Representation of all the transitions
		for (auto state : this->m_automaton->getStatesVector()) {
			for (auto &pair : state->getExitingTransitions()) {
				for (auto child : pair.second) {
					out << "\"" << state->getName() << "\" -> \"" << child->getName() << "\" [ label = \"" << pair.first << "\" ];\n";
				}
			}
		}

		out << "}";
		out.close();
	}

} /* namespace quicksc */
