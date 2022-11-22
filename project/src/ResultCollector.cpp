/*
 * ResultCollector.cpp
 *
 * Modulo che si occupa di archiviare i risultati di un problema.
 */

#include "ResultCollector.hpp"

#include <fstream>

#include "AutomataDrawer.hpp"
//#define DEBUG_MODE
#include "Debug.hpp"
#include "Properties.hpp"

using namespace std;

#define COMPUTE_CORRECTNESS false

namespace quicksc {

	// Stringhe per la visualizzazione delle statistiche
	vector<string> result_stat_headlines = vector<string> {
		"SOL_SIZE    [#] ",		// Dimensione della soluzione trovata dall'algoritmo
		"SOL_GROWTH  [%] ",		// Rapporto fra la dimensione dell'automa della soluzione e l'automa originale
        "SOL_TR_COUNT[#] ",		// Numero di transizioni dell'automa finale
	};

	// Stringhe per la visualizzazione delle statistiche
	vector<string> algorithm_stat_headlines = vector<string> {
		"CORRECTNESS [%] ",		// Percentuale di correttezza della soluzione offerta dall'algoritmo
		"EXEC_TIME   [ms]",		// Tempo dedicato all'esecuzione nell'algoritmo
		"EMP_GAIN    [.] "		// Guadagno sperimentale di tempo dell'algoritmo rispetto ad un algoritmo di benchmark, normalizzato fra -1 e 1
	};

	/**
	 * Costruttore.
	 */
	ResultCollector::ResultCollector(Configurations* configurations, const vector<DeterminizationAlgorithm*>& algorithms) :
	m_algorithms(algorithms) {
		this->m_results = list<Result*>();
		this->m_config_reference = configurations;
	}

	/**
	 * Distruttore della classe ResultCollector.
	 */
	ResultCollector::~ResultCollector() {}

	/**
	 * Metodo privato.
	 * Restituisce un getter che, applicato ad un risultato, permette di
	 * estrarre la statistica richiesta come parametro.
	 * Esempio:
	 * La chiamata di getStatGetter(SOL_SIZE) restituisce una funzione che
	 * accetta come parametro un risultato e restituisce la dimensione dell'automa ottenuto come soluzione.
	 */
	std::function<double(Result*)> ResultCollector::getStatGetter(ResultStat stat) {
		int aux_size = 0;
		std::function<double(Result*)> getter;
		switch(stat) {

		// Dimensione dell'automa ottenuto nella soluzione. (Nota: si considera come riferimeno SC).
		case SOL_SIZE :
			getter = [](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;
				return (double) (result->solutions[benchmark]->size());
			};
			break;

		// Rapporto fra la dimensione dell'automa ottenuto nella soluzione e la dimensione dell'automa originale.
		case SOL_GROWTH :
			aux_size = this->m_config_reference->valueOf<unsigned int>(AutomatonSize);
			getter = [aux_size](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;
				return ((double) (result->solutions[benchmark]->size()) / aux_size) * 100;
			};
			break;

		// Numero di transizioni dell'automa finale
		case SOL_TR_COUNT :
			getter = [](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;
				return (double) (result->solutions[benchmark]->getTransitionsCount());
			};
			break;

		default :
			DEBUG_LOG_ERROR("Valore %d non riconosciuto all'interno dell'enumerazione ResultStat", stat);
			getter = [](Result* result) {
				return -1;
			};
			break;
		}
		return getter;
	}

	/**
	 * Metodo privato.
	 * Restituisce un getter che, applicato ad un risultato e ad un algoritmo, permette di
	 * estrarre la statistica richiesta come parametro.
	 * Esempio:
	 * La chiamata di getStatGetter(SC_TIME) restituisce una funzione che
	 * accetta come parametro un risultato e restituisce il tempo impiegato
	 * per eseguire SC ed ottenere quel risultato.
	 */
	std::function<double(Result*)> ResultCollector::getStatGetter(AlgorithmStat stat, DeterminizationAlgorithm* algorithm) {
		int aux_size = 0;
		std::function<double(Result*)> getter;
		switch(stat) {

		// Tempo di esecuzione dell'algoritmo
		case CORRECTNESS :
			getter = [algorithm](Result* result) {
				// Controllo a compile-time se verificarer la correttezza. NOTA: pesa moltissimo, perché deve confrontare interamente gli automi
				if (COMPUTE_CORRECTNESS) {
					// Confronto la soluzione con quella di benchmark. Se sono uguali, è corretto al 100%. Altrimenti non è corretto (=> 0%).
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

		// Tempo di esecuzione dell'algoritmo
		case EXECUTION_TIME :
			getter = [algorithm](Result* result) {
				return (double) (result->times[algorithm]);
			};
			break;

		// Guadagno sperimentale di tempo di esecuzione rispetto alle tempistiche del benchmark, normalizzato fra -1 e 1
		case EMPIRICAL_GAIN :
			getter = [algorithm](Result* result) {
				DeterminizationAlgorithm* benchmark = result->benchmark_algorithm;

				signed long int benchmark_time = (signed long int) (result->times[benchmark]);
				signed long int algorithm_time = (signed long int) (result->times[algorithm]);

				// Se entrambi i tempi sono uguali, il guadagno empirico è nullo.
				if (benchmark_time == algorithm_time) {
					return 0.0;
					// Nota: questo controllo serve specificatamente ad escludere il caso in cui entrambi i risultati siano nulli, che genererebbe una divisione per 0.
					// In tutti gli altri casi, il tempo maggiore sarà diverso da 0 e sarà possibile dividere per esso.
				}

				double time_diff = benchmark_time - algorithm_time;

				// Appurato che i tempi sono diversi, stabilisco il tempo maggiore
				if (benchmark_time > algorithm_time) {
					return (double) (time_diff / benchmark_time);
				} else {
					return (double) (time_diff / algorithm_time);
				}
			};
			break;

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
	 * Metodo privato.
	 * Restituisce un modo per estrarre la statistica dal risultato.
	 * Questo modo dipende dall'algoritmo e richiede che l'algoritmo abbia effettivamente calcolato tale statistica e l'abbia salvata nella mappa che poi assegna alla struct Result.
	 */
	std::function<double(Result*)> ResultCollector::getStatGetter(RuntimeStat stat, DeterminizationAlgorithm* algorithm) {
			std::function<double(Result*)> getter = [stat, algorithm](Result* result) {
				map<RuntimeStat, double> map = result->runtime_stats[algorithm];
				return map[stat];
			};
			return getter;
	}

	/**
	 * Aggiunge un risultato alla lista.
	 * Non essendo ordinata, l'aggiunta è effettuata di default in coda.
	 */
	void ResultCollector::addResult(Result* result) {
		DEBUG_ASSERT_NOT_NULL(result);
		if (result != NULL) {
			this->m_results.push_back(result);
		}
	}

	/**
	 * Rimuove tutti i risultati della lista, chiamandone il distruttore
	 * per ripulire la memoria.
	 */
	void ResultCollector::reset() {
		// TODO Controllare
		while (!this->m_results.empty()) {
			delete (this->m_results.back());
			this->m_results.pop_back();
		}
	}

	/**
	 * Restituisce il numero di testcases attualmente contenuto
	 * nella lista di risultati.
	 * In caso di chiamata al metodo "reset", questo numero viene azzerato
	 * e viene persa la memoria dei testcase precedenti.
	 */
	unsigned int ResultCollector::getTestCaseNumber() {
		return this->m_results.size();
	}

	/**
	 * Restituisce una terna di valori (MIN, AVG, MAX) calcolati secondo una breve
	 * analisi statistica su tutti i testcases attualmente contenuti nella lista.
	 * I valori vengono estratti da ciascun risultato in funzione del parametro in
	 * ingresso.
	 */
	std::tuple<double, double, double> ResultCollector::computeStat(std::function<double(Result*)> getter) {
		// Inizializzo le variabili
		double min = 1E20, sum = 0, max = -2;
		// Scorro su tutti i risultati
		for (Result* result : this->m_results) {
			// Chiamo la funzione estrattore, estraendo la statistica dal risultato
			double current_value = getter(result);
			// Aggiorno i valori
			if (current_value < min) {
				min = current_value;
			}
			sum += current_value;
			if (current_value > max) {
				max = current_value;
			}
		}
		// Costruisco e restituisco la tupla
		return std::make_tuple(min, (sum / this->m_results.size()), max);
	}

	/**
	 * Restituisce una terna di valori (MIN, AVG, MAX) calcolati secondo una breve
	 * analisi statistica su tutti i testcases attualmente contenuti nella lista.
	 * I valori vengono estratti da ciascun risultato in funzione del parametro in
	 * ingresso.
	 */
	std::tuple<double, double, double> ResultCollector::getStat(ResultStat stat) {
		// Preparo la funzione estrattore
		auto getter = this->getStatGetter(stat);
		DEBUG_LOG("Ho appena estratto l'estrattore");
		// Calcolo
		return this->computeStat(getter);
	}

	/**
	 * Restituisce una terna di valori (MIN, AVG, MAX) calcolati secondo una breve
	 * analisi statistica su tutti i testcases attualmente contenuti nella lista.
	 * La variabile di riferimento dipende dall'algoritmo di interesse.
	 * I valori vengono estratti da ciascun risultato in funzione del parametro in
	 * ingresso e dall'algoritmo specificato.
	 */
	std::tuple<double, double, double> ResultCollector::getStat(AlgorithmStat stat, DeterminizationAlgorithm* algorithm) {
		// Preparo la funzione estrattore
		auto getter = this->getStatGetter(stat, algorithm);
		DEBUG_LOG("Ho appena estratto l'estrattore");
		// Calcolo
		return this->computeStat(getter);
	}

	/**
	 * Restituisce una terna di valori (MIN, AVG, MAX) calcolati secondo una breve
	 * analisi statistica su tutti i testcases attualmente contenuti nella lista.
	 * La variabile di riferimento dipende dall'algoritmo di interesse.
	 * I valori vengono estratti da ciascun risultato in funzione del parametro in
	 * ingresso e dall'algoritmo specificato.
	 */
	std::tuple<double, double, double> ResultCollector::getStat(RuntimeStat stat, DeterminizationAlgorithm* algorithm) {
		// Preparo la funzione estrattore
		auto getter = this->getStatGetter(stat, algorithm);
		DEBUG_LOG("Ho appena estratto l'estrattore");
		// Calcolo
		return this->computeStat(getter);
	}

	/**
	 * Restituisce la percentuale di successo dell'algoritmo passato come riferimento,
	 * confrontato sul campione di tutti i testcase disponibili.
	 * Il test di correttezza è fatto in rapporto alla soluzione fornita dall'algoritmo di benchmark, che si
	 * assume essere corretta.
	 * Se viene passato in ingresso l'algoritmo di benchmark si otterrà ovviamente la correttezza massima (100%).
	 */
	double ResultCollector::getSuccessPercentage(DeterminizationAlgorithm* algorithm) {
		int correct_result_counter = 0;
		for (Result* result : this->m_results) {
			if (*(result->solutions[result->benchmark_algorithm]) == *(result->solutions[algorithm])) {
				correct_result_counter++;
			}
			// Se il debug è attivo, stampo l'automa errato
			IF_DEBUG_ACTIVE(
				else {
					DEBUG_LOG_ERROR("Ho trovato un caso d'errore per l'algoritmo " COLOR_PURPLE("%s"), algorithm->name().c_str());

					DEBUG_LOG_ERROR("\nAutoma " COLOR_PURPLE("NFA originale"));
					AutomataDrawer drawer_original = AutomataDrawer((static_cast<DeterminizationProblem*> (result->original_problem))->getNFA());
					std::cout << drawer_original.asString() << std::endl;

					DEBUG_LOG_ERROR("\nSoluzione benchmark ottenuta con " COLOR_PURPLE("%s"), result->benchmark_algorithm->name().c_str());
					AutomataDrawer drawer_benchmark = AutomataDrawer(result->solutions[result->benchmark_algorithm]);
					std::cout << drawer_benchmark.asString() << std::endl;

					DEBUG_LOG_ERROR("\nSoluzione dell'algoritmo " COLOR_PURPLE("%s"), algorithm->name().c_str());
					AutomataDrawer drawer_result = AutomataDrawer(result->solutions[algorithm]);
					std::cout << drawer_result.asString() << std::endl;

				}
			)
		}
		return ((double)(correct_result_counter)) / this->m_results.size();
	}

	/**
	 * Presentazione di un singolo problema e delle sue soluzioni.
	 * Il contenuto in output dipende dalle impostazioni del programma.
	 */
	void ResultCollector::presentResult(Result* result) {

		DEBUG_MARK_PHASE("Presentazione del problema di partenza") {

			switch (result->original_problem->getType()) {

			case Problem::DETERMINIZATION_PROBLEM : {
				DeterminizationProblem* determinization_problem = (DeterminizationProblem*) result->original_problem;
				AutomataDrawer nfa_drawer = AutomataDrawer(determinization_problem->getNFA());

				if (this->m_config_reference->valueOf<bool>(PrintOriginalAutomaton)) {
					std::cout << "ORIGINAL NFA:\n";
					std::cout << nfa_drawer.asString();
				}

				if (this->m_config_reference->valueOf<bool>(DrawOriginalAutomaton)) {
					// Stampa su file dell'automa originale
					string filename = std::string(DIR_RESULTS) + FILE_NAME_ORIGINAL_AUTOMATON + FILE_EXTENSION_GRAPHVIZ;
					nfa_drawer.asDotFile(filename);
					string command = "dot -Tpdf \"" + filename + "\" -o " + DIR_RESULTS + FILE_NAME_ORIGINAL_AUTOMATON + FILE_EXTENSION_PDF;
					system(command.c_str());
				}
			}
				break;

			default :
				DEBUG_LOG_ERROR("Impossibile interpretare il valore %d come appartenente all'enum ProblemType", result->original_problem->getType());
				break;
			}
		}

		// Scorro su tutti gli algoritmi usati per calcolare il risultato
		for (auto &pair : result->solutions) {
			DeterminizationAlgorithm* algorithm = pair.first;
			Automaton* solution = pair.second;

			DEBUG_ASSERT_NOT_NULL(algorithm);
			DEBUG_ASSERT_NOT_NULL(solution);

			DEBUG_MARK_PHASE("Presentazione della soluzione") {

				AutomataDrawer drawer = AutomataDrawer(solution);

				if (this->m_config_reference->valueOf<bool>(PrintSolutionAutomaton)) {
					// Stampa in formato testuale
//					std::cout << "SOLUZIONE di " << algorithm->name() << ":\n";
					printf(COLOR_PURPLE("\nSoluzione di %s:\n"), algorithm->name().c_str());
					std::cout << std::endl << drawer.asString() << std::endl;
				}

				if (this->m_config_reference->valueOf<bool>(DrawSolutionAutomaton)) {
					// Stampa su file
					string filename = std::string(DIR_RESULTS) + algorithm->abbr() + "_" + FILE_NAME_SOLUTION + FILE_EXTENSION_GRAPHVIZ;
					drawer.asDotFile(filename);
					string command = "dot -Tpdf \"" + filename + "\" -o " + DIR_RESULTS + algorithm->abbr() + "_" + FILE_NAME_SOLUTION + FILE_EXTENSION_PDF;
					system(command.c_str());
				}
			}
		}
	}

	void ResultCollector::printLogHeader(string stat_file_name) {
		ifstream ifile(stat_file_name);
		ofstream file_out(stat_file_name, ios::app);

		// Stampa della headline
		if (! (bool)ifile) {
			for (int setting = 0; setting < SETTINGID_END; setting++) {
				SettingID id = static_cast<SettingID>(setting);

				if (this->m_config_reference->isTestParam(id)) {
					file_out << Configurations::nameOf(id) + ", ";
				}
			}

			// Scorro su tutte le statistiche dipendenti dall'automa
			for (int int_stat = 0; int_stat < RESULTSTAT_END; int_stat++) {
				ResultStat stat = static_cast<ResultStat>(int_stat);
				file_out << result_stat_headlines[stat] << " min, ";
				file_out << result_stat_headlines[stat] << " avg, ";
				file_out << result_stat_headlines[stat] << " max, ";
			}
			// Scorro sugli algoritmi
			for (DeterminizationAlgorithm* algo : this->m_algorithms) {
				// Scorro su tutte le statistiche dipendenti dall'automa & DALL'ALGORITMO
				for (int int_stat = 0; int_stat < ALGORITHMSTAT_END; int_stat++) {
					AlgorithmStat stat = static_cast<AlgorithmStat>(int_stat);
					file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " min, ";
					file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " avg, ";
					file_out << algo->abbr() << " "  << algorithm_stat_headlines[stat] << " max, ";
				}
				// Scorro su tutte le statistiche dipendenti dall'automa & da una singola esecuzione dell'algoritmo
				for (RuntimeStat stat : algo->getRuntimeStatsList()) {
					file_out << algo->abbr() << " " << stat << " min, ";
					file_out << algo->abbr() << " "  << stat << " avg, ";
					file_out << algo->abbr() << " "  << stat << " max, ";
				}
			}

			// Fine della linea header
			file_out << std::endl;
		} // Chiusura dell'header

		ifile.close();
		file_out.close();
	}

	#define FORMAT "\t" COLOR_PINK("%-20s") " | %11.4f | %11.4f | %11.4f |\n"
	/**
	 * Presentazione di tutti i risultati e delle statistiche.
	 */
	void ResultCollector::presentResults() {
		for (Result* result : this->m_results) {
			this->presentResult(result);
		}

		bool do_print = this->m_config_reference->valueOf<bool>(PrintStatistics);
		bool do_log   = this->m_config_reference->valueOf<bool>(LogStatistics);

		// Se non devo fare altro, esco
		if (!do_print && !do_log) {
			return;
		}

		if (do_print) {
			printf("RISULTATI:\n");
			printf("Basati su " COLOR_BLUE("%u") " testcase di automi con le seguenti caratteristiche:\n", this->getTestCaseNumber());
			//printf("AlphabetCardinality    = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AlphabetCardinality));
			printf("Size                   = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonSize));
			//printf("TransitionPercentage   = " COLOR_BLUE("%f") "\n", this->m_config_reference->valueOf<double>(AutomatonTransitionsPercentage));
			printf("EpsilonPercentage      = " COLOR_BLUE("%.3f") "\n", this->m_config_reference->valueOf<double>(EpsilonPercentage));
			printf("MaximumDistance        = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonMaxDistance));
			printf("SafeZoneDistance       = " COLOR_BLUE("%d") "\n", this->m_config_reference->valueOf<int>(AutomatonSafeZoneDistance));

			printf("\n_____________________________|_____" COLOR_YELLOW("MIN") "_____|_____" COLOR_YELLOW("AVG") "_____|_____" COLOR_YELLOW("MAX") "_____|\n");
			printf("\n" COLOR_PURPLE("Solution") "\n");
		}

		ofstream file_out;
		if (do_log) {
			// Scrittura su file dei risultati del blocco di testcase
			string stat_file_name = DIR_RESULTS;
			stat_file_name += string(FILE_NAME_STATS_LOG) + string(FILE_EXTENSION_CSV);
			// Stampo l'header, se il file è vuoto
			this->printLogHeader(stat_file_name);
			// Output stream
			file_out = ofstream(stat_file_name, ios::app);

			file_out << this->m_config_reference->getValueString();
		}

		//// CALCOLO

		// Scorro su tutte le statistiche dipendenti dall'automa
		for (int int_stat = 0; int_stat < RESULTSTAT_END; int_stat++) {
			ResultStat stat = static_cast<ResultStat>(int_stat);
			tuple<double, double, double> stat_values = this->getStat(stat);
			if (do_print) {
				printf(FORMAT, result_stat_headlines[stat].c_str(),
					std::get<0>(stat_values),
					std::get<1>(stat_values),
					std::get<2>(stat_values));
			}
			if (do_log) {
				file_out << std::to_string(std::get<0>(stat_values)) << ", ";
				file_out << std::to_string(std::get<1>(stat_values)) << ", ";
				file_out << std::to_string(std::get<2>(stat_values)) << ", ";
			}
		}

		// Scorro su tutti gli algoritmi
		for (DeterminizationAlgorithm* algo : this->m_algorithms) {

			if (do_print) {
				printf("\n" COLOR_PURPLE("%s") "\n", algo->name().c_str());
				//printf("Success percentage = %f %%\n", (100 * this->getSuccessPercentage(algo)));
			}

			// Scorro su tutte le statistiche dipendenti dall'automa & DALL'ALGORITMO
			for (int int_stat = 0; int_stat < ALGORITHMSTAT_END; int_stat++) {
				AlgorithmStat stat = static_cast<AlgorithmStat>(int_stat);
				tuple<double, double, double> stat_values = this->getStat(stat, algo);
				if (do_print) {
					printf(FORMAT, algorithm_stat_headlines[stat].c_str(),
						std::get<0>(stat_values),
						std::get<1>(stat_values),
						std::get<2>(stat_values));
				}
				if (do_log) {
					file_out << std::to_string(std::get<0>(stat_values)) << ", ";
					file_out << std::to_string(std::get<1>(stat_values)) << ", ";
					file_out << std::to_string(std::get<2>(stat_values)) << ", ";
				}
			}

			// Scorro su tutte le statistiche dipendenti dall'automa & da una singola esecuzione dell'algoritmo
			for (RuntimeStat stat : algo->getRuntimeStatsList()) {
				tuple<double, double, double> stat_values = this->getStat(stat, algo);
				if (do_print) {
					printf(FORMAT, stat.c_str(),
						std::get<0>(stat_values),
						std::get<1>(stat_values),
						std::get<2>(stat_values));
				}
				if (do_log) {
					file_out << std::to_string(std::get<0>(stat_values)) << ", ";
					file_out << std::to_string(std::get<1>(stat_values)) << ", ";
					file_out << std::to_string(std::get<2>(stat_values)) << ", ";
				}
			}
		}

		if (do_log) {
			// Fine riga
			file_out << std::endl;

			// Chiusura del file
			file_out.close();
		}
	}

} /* namespace quicksc */
