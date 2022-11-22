/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Alphabet.hpp
 *
 *
 * Header file for the definition of the Alphabet concept. 
 * This definition is used in the automata generation process and in the automaton definition.
 * 
 * An alphabet is a set of "symbols". Every symbol is implemented, in this projects, as a string.
 * This solution allows a better versatility and it doesn't limit the cardinality of the alphabet to the 
 * latin alphabet.
 * 
 * This header file contains also the definition of the epsilon label, used to represent the epsilon transition.
 * The epsilon label is never contained in the alphabet of the automaton.
 */

#ifndef INCLUDE_ALPHABET_HPP_
#define INCLUDE_ALPHABET_HPP_

#include <string>
#include <vector>

// Epsilon label definition
#define EPSILON ""	// By making it the empty string, it figures as the first element of the alphabet (and the first element in transitions)
#define EPSILON_PRINT "\033[1;34mε\033[0m"	// The epsilon label is printed as the "epsilon" letter
#define SHOW( label ) ((label == EPSILON) ? (EPSILON_PRINT) : (label))

namespace quicksc {

	/**
	 * Alphabet definition as set of strings.
	 */
	using Alphabet = std::vector<std::string>;

}

#endif /* INCLUDE_ALPHABET_HPP_ */
