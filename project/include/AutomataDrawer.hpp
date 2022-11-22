/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataDrawer.hpp
 *
 * 
 * This header file contains the definition of the AutomataDrawer class.
 * The AutomataDrawer class is used to draw the automaton in two ways:
 * - in the console, using the ASCII characters;
 * - in a file, using the Graphviz library.
 */

#ifndef INCLUDE_AUTOMATADRAWER_HPP_
#define INCLUDE_AUTOMATADRAWER_HPP_

#include <string>

#include "Automaton.hpp"

namespace quicksc {

	/**
	 * Parent class for the Automata drawing.
	 * It's specialized in the DFADrawer and NFADrawer classes.
	 */
	class AutomataDrawer {

	private:
		Automaton* m_automaton;

	public:
		AutomataDrawer(Automaton* automaton);
		virtual ~AutomataDrawer();

		string asString();
		void asDotFile(string filename);

	};

} /* namespace quicksc */

#endif /* INCLUDE_AUTOMATADRAWER_HPP_ */
