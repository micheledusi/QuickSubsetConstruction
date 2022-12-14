/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * ResultCollector.hpp
 *
 * 
 * This module manages the collection of results of the tests.
 * The results are stored in a Result object, which is then stored in a ResultCollector object.
 * 
 * The ResultCollector allows to aggregate and compute common statistics over the results.
 */

#ifndef INCLUDE_RESULTCOLLECTOR_HPP_
#define INCLUDE_RESULTCOLLECTOR_HPP_

#include <functional>
#include <list>
#include <map>
#include <tuple>
#include <utility>

#include "Statistics.hpp"
#include "ProblemGenerator.hpp"
#include "DeterminizationAlgorithm.hpp"

namespace quicksc {

	/**
	 * Struttura che rappresenta un singolo risultato ottenuto con la
	 * risoluzione di un singolo problema.
	 */
	struct Result {
		Problem* original_problem;
		map<DeterminizationAlgorithm*, Automaton*> solutions;
		map<DeterminizationAlgorithm*, double> times;
		map<DeterminizationAlgorithm*, map<RuntimeStat, double>> runtime_stats;
		DeterminizationAlgorithm* benchmark_algorithm;
	};

	/**
	 * Classe che raccoglie i risultati e permette l'analisi
	 * di semplici statistiche.
	 */
	class ResultCollector {

	private:
		list<Result*> m_results;
		Configurations* m_config_reference;
		const vector<DeterminizationAlgorithm*>& m_algorithms;

		std::tuple<double, double, double, double> computeStat(std::function<double(Result*)> getter);
		std::function<double(Result*)> getStatGetter(ResultStat stat);
		std::function<double(Result*)> getStatGetter(AlgorithmStat stat, DeterminizationAlgorithm* algorithm);
		std::function<double(Result*)> getStatGetter(RuntimeStat stat, DeterminizationAlgorithm* algorithm);

		void printLogHeader(string stat_file_name);

	public:
		ResultCollector(Configurations* configurations, const vector<DeterminizationAlgorithm*>& algorithms);
		virtual ~ResultCollector();

		// Gestione della lista di risultati
		void addResult(Result* result);
		void reset();

		// Statistiche
		unsigned int getTestCaseNumber();
		double getSuccessPercentage(DeterminizationAlgorithm* algorithm);

		std::tuple<double, double, double, double> getStat(ResultStat stat);
		std::tuple<double, double, double, double> getStat(AlgorithmStat stat, DeterminizationAlgorithm* algorithm);
		std::tuple<double, double, double, double> getStat(RuntimeStat stat, DeterminizationAlgorithm* algorithm);

		void presentResult(Result* result);
		void presentResults();

	};

} /* namespace quicksc */

#endif /* INCLUDE_RESULTCOLLECTOR_HPP_ */
