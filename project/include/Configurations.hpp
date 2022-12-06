/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Configurations.hpp
 *
 * This header file contains the definition of all the configurations parameters used in the project.
 */

#ifndef INCLUDE_CONFIGURATIONS_HPP_
#define INCLUDE_CONFIGURATIONS_HPP_

#include <iostream>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

namespace quicksc {

	typedef enum {
		Testcases,
		RandomSeed,

		ProblemType,

		AlphabetCardinality,
		EpsilonPercentage,
		AutomatonStructure,
		AutomatonSize,
		AutomatonFinalProbability,
		AutomatonTransitionsPercentage,
		AutomatonMaxDistance,
		AutomatonSafeZoneDistance,

		ActiveAutomatonPruning,
		ActiveRemovingLabel,
		ActiveDistanceCheckInTranslation,

		PrintStatistics,
		LogStatistics,

		PrintOriginalAutomaton,
		PrintSolutionAutomaton,
		DrawOriginalAutomaton,
		DrawSolutionAutomaton,

		SETTINGID_END

	} SettingID;

	typedef enum {
		INT,
		DOUBLE,
		BOOL,
	} SettingType;

	typedef union {
		int integer;
		double real;
		bool flag;
	} Value;

	/**
	 * Abstract class for a setting value; it's specialized in the subclasses AtomicSettingValue and CompositeSettingValue,
	 * in accordion to the Composite pattern.
	 */
	class SettingValue {
	public:
		SettingValue() {};
		virtual ~SettingValue() {};
		virtual SettingType getType() = 0;
		virtual Value getValue() = 0;
		virtual string getValueString() = 0; // Returns the value assigned to the paramenter as a string
		virtual string toString() = 0;		 // Returns the setting object as a string
		virtual bool nextCase() = 0;
	};

	/**
	 * The AtomicSettingValue class is used to represent a setting value that has only a single value.
	 * It's a leaf in the Composite pattern, that is, it does not hold any other atomic or composite setting's value.
	 */
	class AtomicSettingValue : public SettingValue {

	private:
		SettingType m_type;
		Value m_value;

	public:
		AtomicSettingValue(int value);
		AtomicSettingValue(double value);
		AtomicSettingValue(bool value);
		~AtomicSettingValue() {};

		SettingType getType();
		Value getValue();
		string getValueString();	// Returns the value assigned to the paramenter as a string
		string toString();			// Returns the setting object as a string
		bool nextCase();

	};

	/**
	 * The CompositeSettingValue class is used to represent a setting value that has more than one value. 
	 * More specifically, its value can assume different values over the course of the execution of the program.
	 * It has an intern state (the "m_current_value_index" private attribute) representing the current value of the setting, and it can be updated to the next value with the "nextCase" method.
	 */
	class CompositeSettingValue : public SettingValue {

	private:
		int m_current_value_index; // Indice del valore corrente
		vector<SettingValue*> m_multivalue; // Vettore dei valori

	public:
		CompositeSettingValue(vector<int> values);
		CompositeSettingValue(vector<double> values);
		~CompositeSettingValue() {};

		SettingType getType();
		Value getValue();
		string getValueString();	// Returns the value assigned to the paramenter as a string
		string toString();			// Returns the setting object as a string
		bool nextCase();

	};

	/**
	 * This class represents a list of parameter (settings) and their values.
	 * The settings are stored in an array of the Setting struct, and the values are stored in a map.
	 * 
	 * For now, all the values and setting are initialized at runtime, at start of the program, according to what's written in the "load" method.
	 */
	class Configurations {

	private:

		/**
		 * This struct represents a single parameter, with its ID and its name.
		 * It does NOT represent the value of the parameter; for that, there's the SettingValue class.
		 * That is done in order to separate the two concepts (the idea of a parameter and its value).
		 */
		struct Setting {
			SettingID m_id;		// The identifier of the setting; it's an enum value
			string m_name;		// The complete name, birefly explaining its function
			string m_abbr;		// The abbreviation of the name, which is unique
			bool m_test_param;	// This flag indicated whether the parameter is a test parameter or not, i.e. if it's used in the testing, and thus if it should be printed
		};

		static const Setting settings_list[];		// Static list of all the configurations, initialized compile-time or at the beginning

		int m_session_index;		// Configuration session index
		vector<map<SettingID, SettingValue*>> m_settings_instances;		// Values of each setting, for each session

		static const Setting& getSetting(const SettingID& id);

		/**
		 * This method loads a single setting in the map of settings of the current session
		 */
		template <typename T> void load(const SettingID& id, T value) {
			this->m_settings_instances[this->m_session_index][id] = new AtomicSettingValue(value);
		};

		/**
		 * This method loads a single setting with multiple values in the map of settings of the current session
		 * If the vector has length 1, then the setting is automatically loaded as an atomic setting.
		 */
		template <typename T> void load(const SettingID& id, vector<T> values) {
			if (values.size() == 1) {
				this->load(id, values[0]);
			} else {
				this->m_settings_instances[this->m_session_index][id] = new CompositeSettingValue(values);
			}
		}

	public:
		Configurations();
		~Configurations();

		void loadDefault();
		void load(string filename);

		static string nameOf(const SettingID& id);
		static string abbreviationOf(const SettingID& id);
		static bool isTestParam(const SettingID& id);
		string getValueString();
		string toString();
		string toString(const SettingID& id);
		bool nextTestCase();

		template <class T> T valueOf(const SettingID& id);

	};

} /* namespace quicksc */

#endif /* INCLUDE_CONFIGURATIONS_HPP_ */
