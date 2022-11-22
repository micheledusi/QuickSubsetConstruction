/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AlphabetGenerator.hpp
 *
 *
 * This header file contains the definition of the AlphabetGenerator class.
 */

#ifndef INCLUDE_ALPHABETGENERATOR_HPP_
#define INCLUDE_ALPHABETGENERATOR_HPP_

#include "Alphabet.hpp"

namespace quicksc {

	class AlphabetGenerator {

	private:
		const char *m_letters;
		unsigned int m_cardinality;

	public:
		static const char *default_letters;
		static const unsigned int default_cardinality;

		AlphabetGenerator();
		virtual ~AlphabetGenerator();

		void setLetters(char letters[]);
		void setCardinality(unsigned int cardinality);
		const char* getLetters();
		unsigned int getCardinality();

		Alphabet generate();

	};

} /* namespace quicksc */

#endif /* INCLUDE_ALPHABETGENERATOR_HPP_ */
