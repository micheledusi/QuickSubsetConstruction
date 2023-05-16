/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Configurations.cpp
 *
 * 
 * This file contains some generic configurations for the program to run.
 */

#include "Configurations.hpp"

#include <fstream>
#include <regex>
#include <ctime>

#include "AutomataGenerator.hpp"
#include "ProblemGenerator.hpp"

//#define DEBUG_MODE
#define DEBUG_QUERY false
#define DEBUG_BUILD false
#include "Debug.hpp"

namespace quicksc {

// CLASS "SettingValue"

	/**
	 * Constructor with integer value.
	 */
	AtomicSettingValue::AtomicSettingValue(int value) {
		if (DEBUG_BUILD) { DEBUG_LOG("Instancing a SettingValue object with INTEGER value = %d", value); }
		this->m_type = INT;
		this->m_value.integer = value;
	}

	/**
	 * Constructor with real value.
	 */
	AtomicSettingValue::AtomicSettingValue(double value) {
		if (DEBUG_BUILD) { DEBUG_LOG("Instancing a SettingValue object with DOUBLE value = %f", value); }
		this->m_type = DOUBLE;
		this->m_value.real = value;
	}

	/**
	 * Constructor with boolean value.
	 */
	AtomicSettingValue::AtomicSettingValue(bool value) {
		if (DEBUG_BUILD) { DEBUG_LOG("Instancing a SettingValue object with BOOLEAN value = %d", value); }
		this->m_type = BOOL;
		this->m_value.flag = value;
	}

	/**
	 * This method returns the type of the setting.
	 */
	SettingType AtomicSettingValue::getType() {
		return this->m_type;
	}

	/**
	 * This method returns the value of the setting.
	 * It is the responsibility of the Configurations method to cast it to the correct type.
	 */
	Value AtomicSettingValue::getValue() {
		return this->m_value;
	}

	/**
	 * This method returns the value of the setting as a string.
	 * Since the casting is done internally, it is not necessary to use templates or overloading.
	 */
	string AtomicSettingValue::getValueString() {
		switch (this->m_type) {
		case INT :
			return std::to_string(this->m_value.integer);
		case DOUBLE :
			return std::to_string(this->m_value.real);
		case BOOL :
			return std::to_string(this->m_value.flag);
		default :
			DEBUG_LOG_ERROR("Cannot interpret the value %d", this->m_value.integer);
			return "null";
		}
	}

	/**
	 * Returns a string representation of the object.
	 * In case of multiple values, returns ALL the values. 
	 * The string also contains information about the type of value.
	*/
	string AtomicSettingValue::toString() {
		switch (this->m_type) {
		case INT :
			return "int:" + std::to_string(this->m_value.integer);
		case DOUBLE :
			return "double:" + std::to_string(this->m_value.real);
		case BOOL :
			return "bool:" + std::to_string(this->m_value.flag);
		default :
			DEBUG_LOG_ERROR("Cannot interpret the value %d", this->m_value.integer);
			return "null";
		}
	}

	/**
	 * Since the value is "single" (or atomic), there is no next value and the method always returns FALSE.
	 */
	bool AtomicSettingValue::nextCase() {
		return false;
	}

// CLASS "SettingMultiValue"

	/**
	 * Constructor with an array of integers.
	 */
	CompositeSettingValue::CompositeSettingValue(vector<int> values) : SettingValue() {
		this->m_multivalue = vector<SettingValue*>();
		for (int n : values) {
			this->m_multivalue.push_back(new AtomicSettingValue(n));
		}
		this->m_current_value_index = 0;
	}

	/**
	 * Constructor with an array of doubles.
	 */
	CompositeSettingValue::CompositeSettingValue(vector<double> values) : SettingValue() {
		this->m_multivalue = vector<SettingValue*>();
		for (double d : values) {
			this->m_multivalue.push_back(new AtomicSettingValue(d));
		}
		this->m_current_value_index = 0;
	}

	SettingType CompositeSettingValue::getType() {
		return this->m_multivalue[this->m_current_value_index]->getType();
	}

	/**
	 * Returns the current value as a string.
	 * In case of multiple values, only the current value is returned as a string.
	 */
	string CompositeSettingValue::getValueString() {
		return this->m_multivalue[this->m_current_value_index]->getValueString();
	}

	/**
	 * Returns a string representation of the object.
	 * In case of multiple values, returns ALL the values. 
	 * The string also contains information about the type of value.
	*/
	string CompositeSettingValue::toString() {
		// Concate all the values
		string result = "{";
		for (SettingValue* sv : this->m_multivalue) {
			result += sv->getValueString() + ", ";
		}
		result.pop_back(); result.pop_back();
		result += "}";
		return result;
	}

	/**
	 * Returns the value at the current index.
	 * It is the responsibility of the Configuration method to cast it to the correct type.
	 */
	Value CompositeSettingValue::getValue() {
		return this->m_multivalue[this->m_current_value_index]->getValue();
	}

	/**
	 * Increments the counter and sets the object to the next value of the vector.
	 * If such a value exists, and therefore if such a value changes, TRUE is returned.
	 * Otherwise, FALSE is returned, and the counter is reset to 0 for a new cycle.
	 */
	bool CompositeSettingValue::nextCase() {
		// Check if the current value is atomic or contains multiple values itself
		if ((this->m_multivalue[this->m_current_value_index])->nextCase()) {
			// In this case, we iterate on it and return TRUE
			return true;
		}
		// If the current value has already "exhausted" its values, or has an atomic value, its method will return false.
		else {
			// We check if there are other values in the vector of this object
			// Increment the counter
			this->m_current_value_index++;
			// Check if the counter corresponds to a new value
			if (this->m_current_value_index < this->m_multivalue.size()) {
				return true;
			} else {
				// Otherwise, I have reached the end, I reset the counter and return FALSE
				this->m_current_value_index = 0;
				return false;
			}
		}
	}

// CLASS "Configurations"

	/**
	 * Constructor.
	 */
	Configurations::Configurations() {
		this->m_session_index = 0;
		this->m_settings_instances = vector<map<SettingID, SettingValue*>>();
	}

	/**
	 * Destructor.
	 */
	Configurations::~Configurations() {}

	/**
	 * Static method.
	 * Returns the setting associated with the ID.
	 *
	 * NOTE: The method relies on the fact that the structures have been initialized in the static array
	 * in order, and therefore it is possible to access them through the index of the array.
	 * For safety, a check has been added inside this method, which -in case the ID does not match-
	 * reports a debug error.
	 */
	const Configurations::Setting& Configurations::getSetting(const SettingID& id) {
		const Configurations::Setting* setting;
		setting = &(Configurations::settings_list[static_cast<int>(id)]);
		if (setting->m_id != id) {
			if (DEBUG_QUERY) {
				DEBUG_LOG_ERROR("Il parametro richiesto con id=%d NON corrisponde al parametro di configurazione nella posizione attesa, che invece ha id=%d e nome \"%s\"",
					id, setting->m_id, setting->m_name.c_str());
			}
		}
		return *setting;
	}

	/////

	void Configurations::loadDefault() {
		// Testcase number
		load(Testcases, 2);
		// Random seed
		load(RandomSeed, (int) time(0));				// The cast is necessary for the "load"	method, because "AtomicSettingValue" accepts only integers, doubles and booleans

		// Problem properties
		load(ProblemType, Problem::DETERMINIZATION_PROBLEM);
		load(AutomatonStructure, AUTOMATON_RANDOM);
//		load(AutomatonStructure, AUTOMATON_STRATIFIED);
//		load(AutomatonStructure, AUTOMATON_STRATIFIED_WITH_SAFE_ZONE);
//		load(AutomatonStructure, AUTOMATON_ACYCLIC);

		// Environment properties
		load(AlphabetCardinality, 5);
		load(AutomatonFinalProbability, 0.1);
		load(AutomatonTransitionsPercentage, 0.2);

		// Default environment properties
		load(AutomatonSize, 100);
		load(EpsilonPercentage, 0.2);
		load(AutomatonMaxLevel, 20);
		load(AutomatonSafeZoneLevel, 10);

		// Modules and special properties
		load(ActiveAutomatonPruning, true); 				// If it's true, the automaton is pruned before the computation
		load(ActiveRemovingLabel, true); 					// If it's true, a special label is used to refer to the epsilon transitions, that has to be removed in the end
		load(ActiveLevelCheckInTranslation, false); 		// If it's true, the translation generates the singularities only if they satisfy a distance constraint [TODO: it's a bugged feature]

		load(PrintStatistics, true);
		load(LogStatistics, true);
		load(LogStatisticsMin, false);
		load(LogStatisticsAvg, true);
		load(LogStatisticsMax, false);
		load(LogStatisticsDev, true);
		load(PrintOriginalAutomaton, false);
		load(PrintSolutionAutomaton, false);
		load(DrawOriginalAutomaton, false);
		load(DrawSolutionAutomaton, false);
	}

	/**
	 * Utility function that trims a string, i.e. removes all the whitespace characters (or the specified delimiters) from the beginning and the end of the string.
	 * NOTE: the string is modified in place, not copied.
	 */
	void trim(string& s, const string& delimiters = " \f\n\r\t\v" ) {
		s.erase(
			s.find_last_not_of( delimiters ) + 1
		).erase(0,
			s.erase(
				s.find_last_not_of( delimiters ) + 1
			).find_first_not_of( delimiters )
		);
	}

	/**
	 * Loads the configurations from a file, building different sessions.
	 */
	void Configurations::load(string filename) {
		// Index reset
		this->m_session_index = 0;

		std::ifstream infile(filename);

		if (infile.fail()) {
			DEBUG_LOG_ERROR("Non è stato possibile leggere il file di configurazione <%s>", filename.c_str());
			DEBUG_LOG("Vengono caricate le impostazioni di default");
			this->m_settings_instances.push_back(map<SettingID, SettingValue*>());
			this->loadDefault();
			return;
		} else {
			DEBUG_LOG_SUCCESS("Il file di configurazione <%s> è stato aperto correttamente", filename.c_str());
		}

		string line;
		while (std::getline(infile, line)) {
			trim(line);

			// Skip empty lines
			if (line == "") {
				continue;
			}
			// Start a new session
			else if (line == "start session") {
				DEBUG_LOG("A new session has been started");
				// Creates the new array
				this->m_settings_instances.push_back(map<SettingID, SettingValue*>());
				// Load the default values
				this->loadDefault();
			}
			// End a session
			else if (line == "end session") {
				DEBUG_LOG_SUCCESS("The session has been configured correctly");
				// Index increase
				this->m_session_index += 1;
			}
			// Otherwise, we extract data
			else {

				string delimiter = "=";
				int eq_pos = line.find(delimiter);
				if (eq_pos == std::string::npos) {
					DEBUG_LOG_ERROR("Cannot interprete line \"%s\" as configuration instruction", line.c_str());
					continue;
				}

				// Extracting the token befor the equal sign, and trimming it
				string line_setting = line.substr(0, eq_pos);
				trim(line_setting);
				// Extracting the value after the equal sign, and trimming it
				string line_value = line.substr(eq_pos + delimiter.length());
				trim(line_value);

				// Iterate over the settings list to find the setting with the same name
				for (int sett_id = 0; sett_id < SETTINGID_END; sett_id++) {
					Setting setting = Configurations::settings_list[sett_id];

					// Check if the line (trimmed) starts with the abbreviation or the name
					if (line_setting == setting.m_name || line_setting == setting.m_abbr) {
						DEBUG_LOG("Identificata impostazione del valore del parametro con ID = %2d: <%s> = %s", setting.m_id, setting.m_name.c_str(), line_value.c_str());

						// Split the string of values in the corresponding values (with comma as delimiter)
						auto const reg = std::regex{","};
						auto const vec = std::vector<string>(
						    std::sregex_token_iterator{begin(line_value), end(line_value), reg, -1},
						    std::sregex_token_iterator{}
						);

						auto values = std::vector<double>();
						for (string s_val : vec) {
							values.push_back(std::stod(s_val));
						}
						load(setting.m_id, values);

						break;
					}
				}
			}
		}

		// Index reset
		this->m_session_index = 0;
	}

	/** Configurations list initialization */
	const Configurations::Setting Configurations::settings_list[] = {
			{ Testcases,					"Testcases", 								"#test", false },
			{ RandomSeed, 					"Random seed", 								"#rseed", false },
			{ ProblemType,					"Problem type", 							"problem", false },
			{ AlphabetCardinality,			"Alphabet cardinality", 					"#alpha", true },
			{ EpsilonPercentage , 			"Epsilon percentage", 						"%epsilon", true },
			{ AutomatonStructure , 			"Automaton's structure type", 				"structure", true },
			{ AutomatonSize , 				"Automaton's size (#states)",	 			"#size", true },
			{ AutomatonFinalProbability , 	"Automaton's final states probability", 	"%finals", false },
			{ AutomatonTransitionsPercentage , "Automaton's transitions percentage", 	"%transitions", true },
			{ AutomatonMaxLevel , 		"Automaton's max distance", 				"maxdist", true },
			{ AutomatonSafeZoneLevel , 	"Automaton's safe-zone distance", 			"safezonedist", true },
			{ ActiveAutomatonPruning , 		"Active \"automaton pruning\"", 			"?autompruning", false },
			{ ActiveRemovingLabel , 		"Active \"removing label\"", 				"?removlabel", false },
			{ ActiveLevelCheckInTranslation , "Active \"distance check in translation\"", "?distcheck",  false },
			{ PrintStatistics , 			"Print statistics", 						"?pstats", false },
			{ LogStatistics , 				"Log statistics in file", 					"?lstats", false },
			{ LogStatisticsMin , 			"Log in file the minimum value of a stat", 	"?lstatsmin", false},
			{ LogStatisticsAvg , 			"Log in file the average value of a stat", 	"?lstatsavg", false},
			{ LogStatisticsMax , 			"Log in file the maximum value of a stat", 	"?lstatsmax", false},
			{ LogStatisticsDev , 			"Log in file the standard deviation value of a stat", 	"?lstatsdev", false},
			{ PrintOriginalAutomaton , 		"Print original automaton", 				"?porig", false },
			{ PrintSolutionAutomaton , 		"Print solution solution", 					"?psolu", false },
			{ DrawOriginalAutomaton , 		"Draw original automaton", 					"?dorig", false },
			{ DrawSolutionAutomaton , 		"Draw solution automaton", 					"?dsolu", false },
	};

	/** 
	 * Static method.
	 * Returns the name of the configuration parameter.
	 */
	string Configurations::nameOf(const SettingID& id) {
		return Configurations::getSetting(id).m_name;
	}

	/** 
	 * Static method.
	 * Returns the identifying acronym of the configuration parameter.
	 */
	string Configurations::abbreviationOf(const SettingID& id) {
		return Configurations::getSetting(id).m_abbr;
	}
	
	/** 
	 * Static method.
	 * Returns a boolean flag indicating whether the value of the parameter must
	 * be printed at the end of the tests.
	 */
	bool Configurations::isTestParam(const SettingID& id) {
		return Configurations::getSetting(id).m_test_param;
	}

	/**
	 * Prints the CURRENT configuration values
	 */
	string Configurations::getValueString() {
		string result = "";
		for (int param = 0; param < SETTINGID_END; param++) {
			if (isTestParam((SettingID) param)) {
				result += this->m_settings_instances[this->m_session_index].at((SettingID)param)->getValueString() + ", ";
			}
		}
		return result;
	}

	/**
	 * Prints all the COMPLETE configuration values
	 */
	string Configurations::toString() {
		string result = "Configurations:\n";
		for (int param = 0; param <= SETTINGID_END; param++) {
			result += Configurations::nameOf((SettingID)param) + " = ";
			result += this->m_settings_instances[this->m_session_index].at((SettingID)param)->toString() + "\n";
		}
		return result;
	}

	/**
	 * Returns a string containing NAME and VALUE associated with the parameter passed as input.
	 */
	string Configurations::toString(const SettingID& id) {
		return (Configurations::abbreviationOf(id) + ":" + this->m_settings_instances[this->m_session_index].at(id)->toString());
	}

	/**
	 * Sets the parameters saved inside the configurations with the next combination
	 */
	bool Configurations::nextTestCase() {
		for (auto &pair : this->m_settings_instances[this->m_session_index]) {
			// If the next case is found, return TRUE
			if (pair.second->nextCase()) { // The "nextCase" method has the conditional effect of incrementing the index
				return true;
			}
		}
		// If the configuration ends, increase the session index
		this->m_session_index += 1;
		// If the index represents a valid session, continue. Otherwise, end the test and return FALSE
		return (this->m_session_index < this->m_settings_instances.size());
	}


	template <class T> T Configurations::valueOf(const SettingID& id) {
		if (DEBUG_QUERY) {
			DEBUG_ASSERT_TRUE(this->m_settings_instances[this->m_session_index].count(id));
			DEBUG_LOG("Reqiested value of: %s", settings_list[int(id)].m_name.c_str());
		}
		SettingValue* associated_value_container = (this->m_settings_instances[this->m_session_index])[id];
		Value value = associated_value_container->getValue();
		switch (associated_value_container->getType()) {
		case INT :
			if (DEBUG_QUERY) { DEBUG_LOG("Returned value: %s", std::to_string(value.integer).c_str()); }
			return static_cast<int>(value.integer);
		case DOUBLE :
			if (DEBUG_QUERY) { DEBUG_LOG("Returned value: %s", std::to_string(value.real).c_str()); }
			return static_cast<double>(value.real);
		case BOOL :
			if (DEBUG_QUERY) { DEBUG_LOG("Returned value: %s", std::to_string(value.flag).c_str()); }
			return static_cast<bool>(value.flag);
		default :
			DEBUG_LOG_ERROR("Cannot parse value %d", value.integer);
			return static_cast<int>(value.integer);
		}
	}


	template bool quicksc::Configurations::valueOf<bool>(quicksc::SettingID const&);
	template int quicksc::Configurations::valueOf<int>(quicksc::SettingID const&);
	template double quicksc::Configurations::valueOf<double>(quicksc::SettingID const&);
	template unsigned int quicksc::Configurations::valueOf<unsigned int>(quicksc::SettingID const&);

} /* namespace quicksc */
