/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * ResultCollector.cpp
 *
 * 
 * This source file contains the implementation of the class ResultCollector.
 */

#include "ResultCollector.hpp"

#include <fstream>
#include <math.h>

#include "AutomataDrawer.hpp"
#include "Properties.hpp"
#include "QuickSubsetConstruction.hpp"

//#define DEBUG_MODE
#include "Debug.hpp"

using namespace std;

#define COMPUTE_CORRECTNESS true
#define DEFAULT_MAX_CONVENIENCE 9999999.9999
#define DEFAULT_MAX_SCALE_FACTOR 9999999.9999

namespace quicksc {

	// Strings for the statistics visualization
	vector<string> result_stat_headlines = vector<string> {
		"SOL_SIZE       [#]   ",
		"SOL_GROWTH     [%]   ",
        "SOL_TR_COUNT   [#]   ",
	};

	// Strings for the statistics visualization
	vector<string> algorithm_stat_headlines = vector<string> {
		/* Skip
		*/
		"CORRECTNESS    [%]   ",
		"EXEC_TIME      [ns]  ",	
		"GAIN           [.]   ",	
		"UNIT_COUNT     [#]   ",
		"VELOCITY       [#/ms]",
		"SCALE_FACTOR   [.]   ",
		/* Skip
		"UNIT_TIME      [ns]  ",
		"CONVENIENCE    [.]   ",
		*/
	};

	/**
	 * Constructor.
	 */
	ResultCollector::ResultCollector(Configurations* configurations, const vector<DeterminizationAlgorithm*>& algorithms) :
	m_algorithms(algorithms) {
		this->m_results = list<Result*>();
		this->m_config_reference = configurations;
	}

	/**
	 * Destructor.
	 */
	ResultCollector::~ResultCollector() {}

	/**
	 * Private method.
	 * Returns a getter that, applied to a result, allows to extract the
	 * requested statistic as a parameter.
	 * Example:
	 * The call of getStatGetter(SOL_SIZE) returns a function that accepts
	 * as a parameter a result and returns the size of the automaton obtained
	 * as a solution.
	 */
	std::function<double(Result*)> ResultCollector::getStatGetter(ResultStat stat) {
		int aux_size = 0;
		std::function<double(Result*)> getter;
		switch(stat) {

		case SOL_SIZE :
			getter = [](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;
				return (double) (result->solutions[benchmark]->size());
			};
			break;

		case SOL_GROWTH :
			aux_size = this->m_config_reference->valueOf<unsigned int>(AutomatonSize);
			getter = [aux_size](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;
				return ((double) (result->solutions[benchmark]->size()) / aux_size) * 100;
			};
			break;

		case SOL_TR_COUNT :
			getter = [](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;
				return (double) (result->solutions[benchmark]->getTransitionsCount());
			};
			break;

		default :
			DEBUG_LOG_ERROR("Value %d unknown for the enumeration ResultStat", stat);
			getter = [](Result* result) {
				return -1;
			};
			break;
		}
		return getter;
	}

	/**
	 * Private method.
	 * Returns a getter that, applied to a result and to an algorithm, allows to extract the
	 * requested statistic as a parameter.
	 * Example:
	 * The call of getStatGetter(SC_TIME) returns a function that accepts
	 * as a parameter a result and returns the time spent by SC to obtain
	 * that result.
	 */
	std::function<double(Result*)> ResultCollector::getStatGetter(AlgorithmStat stat, DeterminizationAlgorithm* algorithm) {
		int aux_size = 0;
		std::function<double(Result*)> getter;
		switch(stat) {

		case CORRECTNESS :
			getter = [algorithm](Result* result) {
				if (COMPUTE_CORRECTNESS) {
					if (*(result->solutions[result->benchmark_algorithm]) == *(result->solutions[algorithm])) {
						return 100.0;
					} else {
						return   0.0;
					}
				}
				else {
					return -1.0;
				}
			};
			break;

		case EXECUTION_TIME :
			getter = [algorithm](Result* result) {
				return (double) (result->times[algorithm]);
			};
			break;

		case EMPIRICAL_GAIN :
			getter = [algorithm](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;

				signed long int benchmark_time = (signed long int) (result->times[benchmark]);
				signed long int algorithm_time = (signed long int) (result->times[algorithm]);

				if (benchmark_time == algorithm_time) {
					return 0.0;
				}

				double time_diff = benchmark_time - algorithm_time;

				if (benchmark_time > algorithm_time) {
					return (double) (time_diff / benchmark_time);
				} else {
					return (double) (time_diff / algorithm_time);
				}
			};
			break;

		case UNIT_COUNT :
			getter = [algorithm](Result* result) {

				DEBUG_ASSERT_FALSE(result->runtime_stats[algorithm].empty());
				IF_DEBUG_ACTIVE(
					for (auto& pair : result->runtime_stats[algorithm]) {
						DEBUG_LOG("Statistics %s has value = %f", pair.first.c_str(), pair.second);
					}
				)

				// Checking if the algorithm has a runtime statistics called "NUMBER SINGULARITIES TOTAL"
				if (result->runtime_stats[algorithm].count(NUMBER_SINGULARITIES_TOTAL)) {
					// If so, we return that value
					DEBUG_LOG("The algorithm has the runtime statistics called \"NUMBER SINGULARITIES TOTAL\"");
					return (double) (result->runtime_stats[algorithm][NUMBER_SINGULARITIES_TOTAL]);
				}
				else {
					DEBUG_LOG("The algorithm has no singularities, so we take the transitions count");
					DEBUG_ASSERT_NOT_NULL(result->solutions[algorithm]);
					// Otherwise, we return the number of transitions of the solution
					DEBUG_LOG("Number of transitions = %d", result->solutions[algorithm]->getTransitionsCount());
					return (double) (result->solutions[algorithm]->getTransitionsCount());
				}
			};
			break;

		case VELOCITY :
			getter = [this, algorithm](Result* result) {
				// Uses the getter of the "UNIT_COUNT" statistic, supposing it's already computed
				auto unit_count_getter = this->getStatGetter(UNIT_COUNT, algorithm);
				// Returns the number of units processed per millisecond
				return (double) (unit_count_getter(result) * 1e6 / result->times[algorithm]);
			};
			break;

		case SCALE_FACTOR :
			getter = [this, algorithm](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;

				// Taking the getters for the "VELOCITY" statistic
				auto benchmark_vel_getter = this->getStatGetter(VELOCITY, benchmark);
				auto algorithm_vel_getter = this->getStatGetter(VELOCITY, algorithm);

				// Getting the statistics for the benchmark algorithm and the current algorithm
				double benchmark_vel = benchmark_vel_getter(result);
				double algorithm_vel = algorithm_vel_getter(result);

				// Computing the convenience
				// If the UTP of the algorithm is 0, the convenience is maximum
				if (algorithm_vel <= 1E-4) {
					return DEFAULT_MAX_SCALE_FACTOR;
				}
				// Otherwise, the smaller the UTP of the algorithm wrt to the benchmark, the higher the convenience
				else {
					return (double) (benchmark_vel / algorithm_vel);
				}
			};
			break;

		/*
		case UNIT_PROCESSING_TIME :
			getter = [this, algorithm](Result* result) {
				// Uses the getter of the "UNIT_COUNT" statistic, supposing it's already computed
				auto unit_count_getter = this->getStatGetter(UNIT_COUNT, algorithm);
				// Returns the time spent by the algorithm to process a unit
				return (double) (result->times[algorithm] / unit_count_getter(result));
			};
			break;

		case CONVENIENCE :
			getter = [this, algorithm](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;

				// Taking the getters for the "UNIT_PROCESSING_TIME" statistic
				auto benchmark_upt_getter = this->getStatGetter(UNIT_PROCESSING_TIME, benchmark);
				auto algorithm_upt_getter = this->getStatGetter(UNIT_PROCESSING_TIME, algorithm);

				// Getting the statistics for the benchmark algorithm and the current algorithm
				double benchmark_utp = benchmark_upt_getter(result);
				double algorithm_utp = algorithm_upt_getter(result);

				// Computing the convenience
				// If the UTP of the algorithm is 0, the convenience is maximum
				if (algorithm_utp <= 1E-8) {
					return DEFAULT_MAX_CONVENIENCE;
				}
				// Otherwise, the smaller the UTP of the algorithm wrt to the benchmark, the higher the convenience
				else {
					return (double) (benchmark_utp / algorithm_utp);
				}
			};
			break;
			*/

		default :
			DEBUG_LOG_ERROR("Valore %d non riconosciuto all'interno dell'enumerazione AlgorithmStat", stat);
			getter = [](Result* result) {
				return -1;
			};
			break;
		}
		return getter;
	}

	/**
	 * Private method.
	 * Returns a getter that, applied to a result and to an algorithm, allows to extract the
	 * requested statistic as a parameter.
	 * This getter depends on the algorithm and requires that the algorithm has actually calculated such a statistic and saved it in the map that then assigns to the struct Result.
	 */
	std::function<double(Result*)> ResultCollector::getStatGetter(RuntimeStat stat, DeterminizationAlgorithm* algorithm) {
			std::function<double(Result*)> getter = [stat, algorithm](Result* result) {
				map<RuntimeStat, double> map = result->runtime_stats[algorithm];
				return map[stat];
			};
			return getter;
	}

	/**
	 * Adds a result to the list.
	 * Since the list is not ordered, the addition is performed by default at the end.
	 */
	void ResultCollector::addResult(Result* result) {
		DEBUG_ASSERT_NOT_NULL(result);
		if (result != NULL) {
			this->m_results.push_back(result);
		}
	}

	/**
	 * Removes all the results from the list, calling their destructor
	 * to clean the memory.
	 */
	void ResultCollector::reset() {
		while (!this->m_results.empty()) {
			delete (this->m_results.back());
			this->m_results.pop_back();
		}
	}

	/**
	 * Returns the number of testcases currently contained
	 * in the list of results.
	 * In case of call to the "reset" method, this number is reset
	 * and the memory of the previous testcases is lost.
	 */
	unsigned int ResultCollector::getTestCaseNumber() {
		return this->m_results.size();
	}

	/**
	 * Returns a quadruple of values (MIN, AVG, MAX, DEV) calculated according to a brief
	 * statistical analysis on all the testcases currently contained in the list.
	 * The values are extracted from each result according to the parameter in input.
	 */
	std::tuple<double, double, double, double> ResultCollector::computeStat(std::function<double(Result*)> getter) {
		double min = 1E20, sum = 0, max = -2;
		vector<double> values = vector<double>();
		for (Result* result : this->m_results) {
			double current_value = getter(result);
			values.push_back(current_value);	// Save the value for the computation of the standard deviation
			if (current_value < min) {
				min = current_value;
			}
			sum += current_value;
			if (current_value > max) {
				max = current_value;
			}
		}
		// Computing the average
		double avg = sum / this->m_results.size();
		// Computing the standard deviation
		double dev = 0;
		for (double value : values) {
			dev += pow(value - avg, 2);
		}
		dev = sqrt(dev / this->m_results.size());
		DEBUG_LOG("Computed stat: min = %f, avg = %f, max = %f, dev = %f", min, avg, max, dev);
		return std::make_tuple(min, avg, max, dev);
	}

	/**
	 * Returns a triple of values (MIN, AVG, MAX, DEV) calculated according to a brief
	 * statistical analysis on all the testcases currently contained in the list.
	 * The values are extracted from each result according to the parameter in
	 * input.
	 */
	std::tuple<double, double, double, double> ResultCollector::getStat(ResultStat stat) {
		auto getter = this->getStatGetter(stat);
		return this->computeStat(getter);
	}

	/**
	 * Returns a triple of values (MIN, AVG, MAX, DEV) calculated according to a brief
	 * statistical analysis on all the testcases currently contained in the list.
	 * The values are extracted from each result according to the parameter in
	 * input and the algorithm specified.
	 */
	std::tuple<double, double, double, double> ResultCollector::getStat(AlgorithmStat stat, DeterminizationAlgorithm* algorithm) {
		auto getter = this->getStatGetter(stat, algorithm);
		return this->computeStat(getter);
	}

	/**
	 * Returns a triple of values (MIN, AVG, MAX, DEV) calculated according to a brief
	 * statistical analysis on all the testcases currently contained in the list.
	 * The values are extracted from each result according to the parameter in
	 * input and the algorithm specified.
	 */
	std::tuple<double, double, double, double> ResultCollector::getStat(RuntimeStat stat, DeterminizationAlgorithm* algorithm) {
		auto getter = this->getStatGetter(stat, algorithm);
		return this->computeStat(getter);
	}

	/**
	 * Returns the success percentage of the algorithm passed as a reference,
	 * compared to the sample of all available testcases.
	 * The correctness test is done in relation to the solution provided by the benchmark algorithm, which is
	 * assumed to be correct.
	 * If the benchmark algorithm is passed as input, the maximum correctness will be obtained (100%).
	 */
	double ResultCollector::getSuccessPercentage(DeterminizationAlgorithm* algorithm) {
		int correct_result_counter = 0;
		for (Result* result : this->m_results) {
			if (*(result->solutions[result->benchmark_algorithm]) == *(result->solutions[algorithm])) {
				correct_result_counter++;
			}
			// If debug is active, the wrong automaton is printed
			IF_DEBUG_ACTIVE(
				else {
					DEBUG_LOG_ERROR("Found an error case for the algorithm " COLOR_PURPLE("%s"), algorithm->name().c_str());

					DEBUG_LOG_ERROR("\nAutomaton " COLOR_PURPLE("Original NFA"));
					AutomataDrawer drawer_original = AutomataDrawer((static_cast<DeterminizationProblem*> (result->original_problem))->getNFA());
					std::cout << drawer_original.asString() << std::endl;

					DEBUG_LOG_ERROR("\nBenchmark solution obtained with the algorithm " COLOR_PURPLE("%s"), result->benchmark_algorithm->name().c_str());
					AutomataDrawer drawer_benchmark = AutomataDrawer(result->solutions[result->benchmark_algorithm]);
					std::cout << drawer_benchmark.asString() << std::endl;

					DEBUG_LOG_ERROR("\nSolution obtained with the algorithm " COLOR_PURPLE("%s"), algorithm->name().c_str());
					AutomataDrawer drawer_result = AutomataDrawer(result->solutions[algorithm]);
					std::cout << drawer_result.asString() << std::endl;

				}
			)
		}
		return ((double)(correct_result_counter)) / this->m_results.size();
	}

	/**
	 * Prints the result of a single testcase.
	 * The output depends on the program settings.
	 */
	void ResultCollector::presentResult(Result* result) {

		DEBUG_MARK_PHASE("Presentation of the first problem") {

			switch (result->original_problem->getType()) {

			case Problem::DETERMINIZATION_PROBLEM : {
				DeterminizationProblem* determinization_problem = (DeterminizationProblem*) result->original_problem;
				AutomataDrawer nfa_drawer = AutomataDrawer(determinization_problem->getNFA());

				if (this->m_config_reference->valueOf<bool>(PrintOriginalAutomaton)) {
					std::cout << "ORIGINAL NFA:\n";
					std::cout << nfa_drawer.asString();
				}

				if (this->m_config_reference->valueOf<bool>(DrawOriginalAutomaton)) {
					string filename = std::string(DIR_RESULTS) + FILE_NAME_ORIGINAL_AUTOMATON + FILE_EXTENSION_GRAPHVIZ;
					nfa_drawer.asDotFile(filename);
					string command = "dot -Tpdf \"" + filename + "\" -o " + DIR_RESULTS + FILE_NAME_ORIGINAL_AUTOMATON + FILE_EXTENSION_PDF;
					system(command.c_str());
				}
			}
				break;

			default :
				DEBUG_LOG_ERROR("Cannot parse the value %d as an element of the enumeration ProblemType", result->original_problem->getType());
				break;
			}
		}

		// Iterate over all the algorithms used to calculate the result
		for (auto &pair : result->solutions) {
			DeterminizationAlgorithm* algorithm = pair.first;
			Automaton* solution = pair.second;

			DEBUG_ASSERT_NOT_NULL(algorithm);
			DEBUG_ASSERT_NOT_NULL(solution);

			DEBUG_MARK_PHASE("Presenting the solution automaton of the algorithm %s", algorithm->name().c_str()) {

				AutomataDrawer drawer = AutomataDrawer(solution);

				if (this->m_config_reference->valueOf<bool>(PrintSolutionAutomaton)) {
					printf(COLOR_PURPLE("\nSolution of %s:\n"), algorithm->name().c_str());
					std::cout << std::endl << drawer.asString() << std::endl;
				}

				if (this->m_config_reference->valueOf<bool>(DrawSolutionAutomaton)) {
					// Print the automaton in a dot file
					string filename = std::string(DIR_RESULTS) + algorithm->abbr() + "_" + FILE_NAME_SOLUTION + FILE_EXTENSION_GRAPHVIZ;
					drawer.asDotFile(filename);
					string command = "dot -Tpdf \"" + filename + "\" -o " + DIR_RESULTS + algorithm->abbr() + "_" + FILE_NAME_SOLUTION + FILE_EXTENSION_PDF;
					system(command.c_str());
				}
			}
		}
	}

	/**
	 * Prints the result of all the testcases.
	 * The output depends on the program settings.
	 */
	void ResultCollector::printLogHeader(string stat_file_name) {
		ifstream ifile(stat_file_name);
		ofstream file_out(stat_file_name, ios::app);

		bool do_log   = this->m_config_reference->valueOf<bool>(LogStatistics);
		bool do_log_min, do_log_avg, do_log_max, do_log_dev;
		if (do_log) {
			do_log_min = this->m_config_reference->valueOf<bool>(LogStatisticsMin);
			do_log_avg = this->m_config_reference->valueOf<bool>(LogStatisticsAvg);
			do_log_max = this->m_config_reference->valueOf<bool>(LogStatisticsMax);
			do_log_dev = this->m_config_reference->valueOf<bool>(LogStatisticsDev);
		}

		if (! (bool)ifile) {
			for (int setting = 0; setting < SETTINGID_END; setting++) {
				SettingID id = static_cast<SettingID>(setting);

				if (this->m_config_reference->isTestParam(id)) {
					file_out << Configurations::nameOf(id) + ", ";
				}
			}

			for (int int_stat = 0; int_stat < RESULTSTAT_END; int_stat++) {
				ResultStat stat = static_cast<ResultStat>(int_stat);
				if (do_log_min) file_out << result_stat_headlines[stat] << " min, ";
				if (do_log_avg) file_out << result_stat_headlines[stat] << " avg, ";
				if (do_log_max) file_out << result_stat_headlines[stat] << " max, ";
				if (do_log_dev) file_out << result_stat_headlines[stat] << " dev, ";
			}
			for (DeterminizationAlgorithm* algo : this->m_algorithms) {
				for (int int_stat = 0; int_stat < ALGORITHMSTAT_END; int_stat++) {
					AlgorithmStat stat = static_cast<AlgorithmStat>(int_stat);
					if (do_log_min) file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " min, ";
					if (do_log_avg) file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " avg, ";
					if (do_log_max) file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " max, ";
					if (do_log_dev) file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " dev, ";
				}
				for (RuntimeStat stat : algo->getRuntimeStatsList()) {
					if (do_log_min) file_out << algo->abbr() << " " << stat << " min, ";
					if (do_log_avg) file_out << algo->abbr() << " "  << stat << " avg, ";
					if (do_log_max) file_out << algo->abbr() << " "  << stat << " max, ";
					if (do_log_dev) file_out << algo->abbr() << " "  << stat << " dev, ";
				}
			}

			file_out << std::endl;
		} 

		ifile.close();
		file_out.close();
	}

	
	/**
	 * Utility methods.
	 * This method gets the entry of a statistics as input.
	 * The entry is composed by a name and a measure unit, inserted in brackets and separated by a space.
	 * The method returns a tuple containing the name and the measure unit, separately.
	*/
	pair<string, string> getStatHeader(string statistic_entry) {
		string name = statistic_entry.substr(0, statistic_entry.find(" "));
		string unit = statistic_entry.substr(statistic_entry.find("["), statistic_entry.find("]"));
		return make_pair(name, unit);
	}

	/**
	 * PMacro defining the format of the output.
	 */
	#define FORMAT "\t" COLOR_PINK("%-20s %6s") " | %11.4f | %11.4f | %11.4f | %11.4f |\n"

	/**
	 * Presents the results of the testcases.
	 */
	void ResultCollector::presentResults() {
		for (Result* result : this->m_results) {
			this->presentResult(result);
		}

		DEBUG_MARK_PHASE("Presenting statistics") {
			bool do_print = this->m_config_reference->valueOf<bool>(PrintStatistics);
			bool do_log   = this->m_config_reference->valueOf<bool>(LogStatistics);
			bool do_log_min, do_log_avg, do_log_max, do_log_dev;
			
			if (do_log) {
				do_log_min = this->m_config_reference->valueOf<bool>(LogStatisticsMin);
				do_log_avg = this->m_config_reference->valueOf<bool>(LogStatisticsAvg);
				do_log_max = this->m_config_reference->valueOf<bool>(LogStatisticsMax);
				do_log_dev = this->m_config_reference->valueOf<bool>(LogStatisticsDev);
				DEBUG_LOG("Set logging statistics to: min: %d, avg: %d, max: %d, dev: %d", do_log_min, do_log_avg, do_log_max, do_log_dev);
			}

			// If no other action is required, return
			if (!do_print && !do_log) {
				DEBUG_LOG("No action required (do_print: %d, do_log: %d). Ending procedure.", do_print, do_log);
				return;
			}

			if (do_print) {
				printf("RESULTS:\n");
				printf("Based on " COLOR_BLUE("%u") " testcases of automata with these characteristics:\n", this->getTestCaseNumber());
				printf("Automaton Type         = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonStructure));
				printf("AlphabetCardinality    = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AlphabetCardinality));
				printf("Size                   = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonSize));
				printf("TransitionPercentage   = " COLOR_BLUE("%f") "\n", this->m_config_reference->valueOf<double>(AutomatonTransitionsPercentage));
				printf("EpsilonPercentage      = " COLOR_BLUE("%.3f") "\n", this->m_config_reference->valueOf<double>(EpsilonPercentage));
				printf("MaximumLevel           = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonMaxLevel));
				printf("SafeZoneLevel          = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonSafeZoneLevel));

				printf("\n_____________________________________|_____" COLOR_YELLOW("MIN") "_____|_____" COLOR_YELLOW("AVG") "_____|_____" COLOR_YELLOW("MAX") "_____|_____" COLOR_YELLOW("DEV") "_____|\n");
				printf("\n" COLOR_PURPLE("Solution") "\n");
			}

			ofstream file_out;
			if (do_log) {
				string stat_file_name = DIR_RESULTS;
				stat_file_name += string(FILE_NAME_STATS_LOG) + string(FILE_EXTENSION_CSV);
				this->printLogHeader(stat_file_name);
				file_out = ofstream(stat_file_name, ios::app);

				file_out << this->m_config_reference->getValueString();
			}

			//// Computation of the statistics

			// Iteration over all the statistics
			for (int int_stat = 0; int_stat < RESULTSTAT_END; int_stat++) {
				ResultStat stat = static_cast<ResultStat>(int_stat);
				tuple<double, double, double, double> stat_values = this->getStat(stat);
				if (do_print) {
					pair<string, string> stat_str = getStatHeader(result_stat_headlines[stat]);
					printf(FORMAT, stat_str.first.c_str(), stat_str.second.c_str(),
						std::get<0>(stat_values),
						std::get<1>(stat_values),
						std::get<2>(stat_values),
						std::get<3>(stat_values));
				}
				if (do_log) {
					if (do_log_min)	file_out << std::to_string(std::get<0>(stat_values)) << ", ";
					if (do_log_avg)	file_out << std::to_string(std::get<1>(stat_values)) << ", ";
					if (do_log_max)	file_out << std::to_string(std::get<2>(stat_values)) << ", ";
					if (do_log_dev)	file_out << std::to_string(std::get<3>(stat_values)) << ", ";
				}
			}

			// Iteration over all the algorithms
			for (DeterminizationAlgorithm* algo : this->m_algorithms) {

				if (do_print) {
					printf("\n" COLOR_PURPLE("%s") "\n", algo->name().c_str());
					//printf("Success percentage = %f %%\n", (100 * this->getSuccessPercentage(algo)));
				}

				// Iteration over all the statistics depending on the algorithm and the automaton
				for (int int_stat = 0; int_stat < ALGORITHMSTAT_END; int_stat++) {
					AlgorithmStat stat = static_cast<AlgorithmStat>(int_stat);
					tuple<double, double, double, double> stat_values = this->getStat(stat, algo);
					if (do_print) {
						pair<string, string> stat_str = getStatHeader(algorithm_stat_headlines[stat]);
						printf(FORMAT, stat_str.first.c_str(), stat_str.second.c_str(),
							std::get<0>(stat_values),
							std::get<1>(stat_values),
							std::get<2>(stat_values),
							std::get<3>(stat_values));
					}
					if (do_log) {
						if (do_log_min)	file_out << std::to_string(std::get<0>(stat_values)) << ", ";
						if (do_log_avg)	file_out << std::to_string(std::get<1>(stat_values)) << ", ";
						if (do_log_max)	file_out << std::to_string(std::get<2>(stat_values)) << ", ";
						if (do_log_dev)	file_out << std::to_string(std::get<3>(stat_values)) << ", ";
					}
				}

				// Iteration over all the statistics depending on the algorithm and the automaton and a single execution of the algorithm
				for (RuntimeStat stat : algo->getRuntimeStatsList()) {
					tuple<double, double, double, double> stat_values = this->getStat(stat, algo);
					if (do_print) {
						pair<string, string> stat_str = getStatHeader(stat);
						printf(FORMAT, stat_str.first.c_str(), stat_str.second.c_str(),
							std::get<0>(stat_values),
							std::get<1>(stat_values),
							std::get<2>(stat_values),
							std::get<3>(stat_values));
					}
					if (do_log) {
						if (do_log_min)	file_out << std::to_string(std::get<0>(stat_values)) << ", ";
						if (do_log_avg)	file_out << std::to_string(std::get<1>(stat_values)) << ", ";
						if (do_log_max)	file_out << std::to_string(std::get<2>(stat_values)) << ", ";
						if (do_log_dev)	file_out << std::to_string(std::get<3>(stat_values)) << ", ";
					}
				}
			}

			if (do_log) {
				// End of line
				file_out << std::endl;

				// File closing
				file_out.close();
			}
		}
	}

} /* namespace quicksc */
