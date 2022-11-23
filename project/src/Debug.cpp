/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Debug.hpp
 *
 *
 * This file implements the debug functions within the Debug module.
 * Usually, the debug functions are macro functions that are defined in the header file "Debug.hpp".
 * This source file is used to define some of the debug functions that cannot be defined as macro functions,
 * because they are too complex.
 * 
 * For further information about the debug functions, or for using them as an indipendent module 
 * in your C/C++ project, please refer to the GitHub repository: 
 * <see cref="https://github.com/MicheleDusi/MacroDebugger.git" />
 * 
 */

#include "Debug.hpp"

#include <list>

using std::list;

namespace quicksc {

	list<int> tickets_stack = list<int>();
	int number_of_elements = 0;

	/**
	 * Function used to mark the phases in the debug library.
	 * It allows the computation of the list of numbers related to the various phases of which we are keeping track.
	 */
	string debugComputeTicketsList() {
		string result = std::to_string(*tickets_stack.begin());

		auto cursor = ++tickets_stack.begin();
		for (int i = 1; i < number_of_elements; i++) {
			result += "." + std::to_string(*cursor);
			cursor++;
		}

		return result;
	}

	/**
	 * Function used to mark the phases in the debug library.
	 * It is called when entering a phase; it has two main functionalities:
	 * - Modifies a global variable (stack) by writing to it that we have entered a
	 * 	 new phase; in other words, it marks that we have "descended one level".
	 * - Returns a string that contains the trace of all the phases passed so far.
	 */
	string debugAcquireTicket() {

		// Adds an element to the stack
		number_of_elements++;

		// If the stack overgoes the size of the tickets
		if (number_of_elements > tickets_stack.size()) {
			// Adds a new ticket to the stack
			tickets_stack.push_back(1);
		} else {
			// Otherwise, it increments the current ticket
			auto cursor = tickets_stack.begin();
			for (int i = 0; i < number_of_elements - 1; i++) {
				cursor++;
			}
			(*cursor)++;
		}

		// Returns the textual representation of the updated list
		return debugComputeTicketsList();
	}

	/**
	 * Function used to mark the phases in the debug library.
	 * It is called when exiting a phase; it has two main functionalities:
	 * - Modifies a global variable (stack) by writing to it that we have exited the current phase;
	 *  in other words, it marks that we have "risen one level".
	 * - Returns a string that contains the trace of all the phases passed so far.
	 */
	string debugReleaseTicket() {
		// Returns the textual representation of the updated list
		string s = debugComputeTicketsList();

		// Considering one level less
		number_of_elements--;

		// If we rise too fast, we forget the internal indices
		if (tickets_stack.size() > number_of_elements + 1) {
			tickets_stack.pop_back();
		}

		return s;

	}

}




