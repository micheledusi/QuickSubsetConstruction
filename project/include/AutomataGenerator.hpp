/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * AutomataGenerator.hpp
 *
 *
 * This header file contains the definition of the AutomataGenerator class.
 * The AutomataGenerator class is used to generate the automaton of different types and with different characteristics,
 * according to the parameters passed to the constructor.
 * 
 * The AutomataGenerator class is parametrized by the type of the automaton to generate (class Automaton).
 * The generic methos "generateRandomAutomaton" returns a generic automaton, that has to be specified in the concrete implementations of the class.
 */

#ifndef INCLUDE_AUTOMATAGENERATOR_HPP_
#define INCLUDE_AUTOMATAGENERATOR_HPP_

#include <vector>
#include <string>

#include "Alphabet.hpp"
#include "Automaton.hpp"
#include "Configurations.hpp"

#define UNDEFINED_VALUE -1

namespace quicksc {

	typedef enum {
		AUTOMATON_RANDOM,
		AUTOMATON_STRATIFIED,
		AUTOMATON_STRATIFIED_WITH_SAFE_ZONE,
		AUTOMATON_ACYCLIC,
		AUTOMATON_WEAK
	} AutomatonType;

	class AutomataGenerator {

	private:
		Alphabet m_alphabet;
		AutomatonType m_automaton_structure;
		unsigned long int m_size;
		string m_name_prefix;
		double m_transition_percentage;
		double m_epsilon_probability = 0;
		double m_final_probability;
		double m_max_distance = UNDEFINED_VALUE;
		double m_safe_zone_distance = UNDEFINED_VALUE;

		unsigned int m_namesCounter = 0;

	protected:
		static const char* default_name_prefix;
		
		Configurations* m_configurations;

		void resetNames();
		string generateUniqueName();
		double generateNormalizedDouble();
		string getRandomLabelFromAlphabet();
		unsigned long int computeDeterministicTransitionsNumber();

	public:
		AutomataGenerator(Alphabet alphabet, Configurations* configurations);
		virtual ~AutomataGenerator();

		Alphabet getAlphabet();
		AutomatonType getAutomatonStructure();
		unsigned long int getSize();
		string getNamePrefix();
		double getTransitionPercentage();
		double getEpsilonProbability();
		double getFinalProbability();
		unsigned int getMaxDistance();
		void setMaxDistance(unsigned int max_distance);
		unsigned int getSafeZoneDistance();

		Automaton* generateAutomaton();
		virtual Automaton* generateRandomAutomaton();
		virtual Automaton* generateStratifiedAutomaton();
		virtual Automaton* generateStratifiedWithSafeZoneAutomaton();
		virtual Automaton* generateAcyclicAutomaton();
		virtual Automaton* generateWeakAutomaton();

	};

} /* namespace quicksc */

#endif /* INCLUDE_AUTOMATAGENERATOR_HPP_ */
