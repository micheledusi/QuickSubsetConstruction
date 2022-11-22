/*
 * Configurations.cpp
 *
 * Implementazione della classe "Configurations", che memorizza e offre un'interfaccia
 * standard per le impostazioni con cui eseguire il programma.
 */

#include "Configurations.hpp"

#include <fstream>
#include <regex>

#include "AutomataGenerator.hpp"
#include "ProblemGenerator.hpp"

//#define DEBUG_MODE
#define DEBUG_QUERY false
#define DEBUG_BUILD false
#include "Debug.hpp"

namespace quicksc {

// CLASSE "SettingValue"

	/**
	 * Costruttore con valore intero.
	 */
	AtomicSettingValue::AtomicSettingValue(int value) {
		if (DEBUG_BUILD) { DEBUG_LOG("Costruzione di un oggetto SettingValue con valore INT = %d", value); }
		this->m_type = INT;
		this->m_value.integer = value;
	}

	/**
	 * Costruttore con valore reale.
	 */
	AtomicSettingValue::AtomicSettingValue(double value) {
		if (DEBUG_BUILD) { DEBUG_LOG("Costruzione di un oggetto SettingValue con valore DOUBLE = %f", value); }
		this->m_type = DOUBLE;
		this->m_value.real = value;
	}

	/**
	 * Costruttore con valore booleano.
	 */
	AtomicSettingValue::AtomicSettingValue(bool value) {
		if (DEBUG_BUILD) { DEBUG_LOG("Costruzione di un oggetto SettingValue con valore BOOL = %d", value); }
		this->m_type = BOOL;
		this->m_value.flag = value;
	}

	/**
	 * Restituisce il tipo del valore.
	 */
	SettingType AtomicSettingValue::getType() {
		return this->m_type;
	}

	/**
	 * Restituisce il valore.
	 * Sarà compito del metodo di Configurations castarlo al tipo corretto.
	 */
	Value AtomicSettingValue::getValue() {
		return this->m_value;
	}

	/**
	 * Restituisce il valore come stringa.
	 * Poiché il casting avviene internamente non è necessario usare template o overloading.
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
			DEBUG_LOG_ERROR("Impossibile interpretare il valore %d", this->m_value.integer);
			return "null";
		}
	}

	/**
	 * Restituisce una rappresentazione dell'oggetto come stringa.
	 * In caso di valori multipli, restituisce TUTTI i valori.
	 * Nella stringa è presente anche un'informazione sul tipo di valore.
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
			DEBUG_LOG_ERROR("Impossibile interpretare il valore %d", this->m_value.integer);
			return "null";
		}
	}

	/**
	 * Poiché il valore è "singolo" ( o atomico), non esiste un valore successivo e il metodo restituisce sempre FALSE.
	 */
	bool AtomicSettingValue::nextCase() {
		return false;
	}

// CLASSE "SettingMultiValue"

	/**
	 * Costruttore con un array di interi.
	 */
	CompositeSettingValue::CompositeSettingValue(vector<int> values) : SettingValue() {
		// Inserisco i valori
		this->m_multivalue = vector<SettingValue*>();
		for (int n : values) {
			this->m_multivalue.push_back(new AtomicSettingValue(n));
		}
		this->m_current_value_index = 0;
	}

	/**
	 * Costruttore con un array di double.
	 */
	CompositeSettingValue::CompositeSettingValue(vector<double> values) : SettingValue() {
		// Inserisco i valori
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
	 * Restituisce il valore corrente come stringa.
	 * In caso di valori multipli, solo il valore corrente è restituito come stringa.
	 */
	string CompositeSettingValue::getValueString() {
		return this->m_multivalue[this->m_current_value_index]->getValueString();
	}

	/**
	 * Restituisce una rappresentazione dell'oggetto come stringa.
	 * In caso di valori multipli, restituisce TUTTI i valori.
	 * Nella stringa è presente anche un'informazione sul tipo di valore.
	 */
	string CompositeSettingValue::toString() {
		// Concateno i valori
		string result = "{";
		for (SettingValue* sv : this->m_multivalue) {
			result += sv->getValueString() + ", ";
		}
		result.pop_back(); result.pop_back();
		result += "}";
		return result;
	}

	/**
	 * Restituisce il valore all'indice corrente.
	 * Sarà responsabilità della classe Configuration castare il tutto.
	 */
	Value CompositeSettingValue::getValue() {
		return this->m_multivalue[this->m_current_value_index]->getValue();
	}

	/**
	 * Incrementa il contatore e imposta l'oggetto al valore successivo del vettore.
	 * Se tale valore esiste, e quindi se tale valore cambia, viene restituito TRUE.
	 * Altrimenti viene restituito FALSE, e il contatore viene resettato a 0 per un nuovo ciclo.
	 */
	bool CompositeSettingValue::nextCase() {
		// Verifico se il valore corrente è atomico o contiene valori multipli a sua volta
		if ((this->m_multivalue[this->m_current_value_index])->nextCase()) {
			// In tal caso, itero su di esso e restituisco TRUE
			return true;
		}
		// Se invece il valore corrente ha già "esaurito" i suoi valori, o ha un valore atomico, il suo metodo restituirà false.
		else {
			// Viene quindi verificato se esistono altri valori nel vettore di questo oggetto
			// Incremento il contatore
			this->m_current_value_index++;
			// Verifico se corrisponde ad un nuovo valore
			if (this->m_current_value_index < this->m_multivalue.size()) {
				return true;
			} else {
				// Altrimenti sono arrivato alla fine, azzero il contatore e restituisco FALSE
				this->m_current_value_index = 0;
				return false;
			}
		}
	}

// CLASSE "Configurations"

	/**
	 * Costruttore.
	 */
	Configurations::Configurations() {
		this->m_session_index = 0;
		this->m_settings_instances = vector<map<SettingID, SettingValue*>>();
	}

	/**
	 * Distruttore.
	 */
	Configurations::~Configurations() {}

	/**
	 * Metodo static.
	 * Restituisce il setting associato all'ID.
	 *
	 * NOTA: Il metodo fa affidamento sul fatto che le strutture sono state inizializzate nell'array statico
	 * in ordine, e quindi è possibile avervi accesso tramite l'indice dell'array.
	 * Per sicurezza è stato aggiunto un controllo all'interno di questo metodo, che -in caso l'ID non corrisponda-
	 * segnala un errore di debug.
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
		// Numero di Testcase
		load(Testcases, 100);

		// Proprietà del problema
		load(ProblemType, Problem::DETERMINIZATION_PROBLEM);
		load(AutomatonStructure, AUTOMATON_RANDOM);
//		load(AutomatonStructure, AUTOMATON_STRATIFIED);
//		load(AutomatonStructure, AUTOMATON_STRATIFIED_WITH_SAFE_ZONE);
//		load(AutomatonStructure, AUTOMATON_ACYCLIC);

		// Variabili d'ambiente
		load(AlphabetCardinality, 5);
		load(AutomatonFinalProbability, 0.1);
		load(AutomatonTransitionsPercentage, 0.2);

		// Variabili indipendenti default
		load(AutomatonSize, 100);
		load(EpsilonPercentage, 0.2);
		load(AutomatonMaxDistance, 20);
		load(AutomatonSafeZoneDistance, 10);

		// Moduli e funzionalità opzionali
		load(ActiveAutomatonPruning, true); 				// In caso sia attivato, evita la formazione e la gestione dello stato con estensione vuota, tramite procedura Automaton Pruning
		load(ActiveRemovingLabel, true); 					// In caso sia attivato, utilizza una label apposita per segnalare le epsilon-transizione, che deve essere rimossa durante la determinizzazione
		load(ActiveDistanceCheckInTranslation, false); 		// In caso sia attivato, durante la traduzione genera delle singolarità solamente se gli stati soddisfano una particolare condizione sulla distanza [FIXME è una condizione che genera bug]

		load(PrintStatistics, true);
		load(LogStatistics, true);
		load(PrintOriginalAutomaton, false);
		load(PrintSolutionAutomaton, false);
		load(DrawOriginalAutomaton, false);
		load(DrawSolutionAutomaton, false);
	}

	/**
	 * Funzione di utilità che fa il trimming di una stringa, ossia rimuove tutti i caratteri di spaziatura (o i delimitatori specificati in input) dall'inizio e dalla fine della stringa.
	 NOTA: la stringa è modificata in place, non copiata.
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
	 * Carica le configurazioni da file, costruendo differenti sessioni.
	 */
	void Configurations::load(string filename) {
		// Reset indice
		this->m_session_index = 0;

		// Apro il file di configurazione in lettura
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
			// Trimming line
			trim(line);

			// Salto le linee vuote
			if (line == "") {
				continue;
			}
			// Inizio una nuova sessione
			else if (line == "start session") {
				DEBUG_LOG("E' stata identificata una nuova sessione di test");
				// Creo il nuovo array
				this->m_settings_instances.push_back(map<SettingID, SettingValue*>());
				// Carico le variabili fisse
				this->loadDefault();
			}
			// Finisco una sessione
			else if (line == "end session") {
				DEBUG_LOG_SUCCESS("La sessione è stata configurata correttamente");
				// Incremento l'indice
				this->m_session_index += 1;
			}
			// Altrimenti estraggo i dati
			else {

				// Cerco all'interno l'uguale
				string delimiter = "=";
				int eq_pos = line.find(delimiter);
				if (eq_pos == std::string::npos) {
					DEBUG_LOG_ERROR("Impossibile interpretare la riga \"%s\" come istruzione di configurazione", line.c_str());
					continue;
				}

				// Estraggo il token prima dell'uguale, e il valore dopo l'uguale
				string line_setting = line.substr(0, eq_pos);
				trim(line_setting);
				string line_value = line.substr(eq_pos + delimiter.length());
				trim(line_value);

				// Scorro su tutti i settings per capire a quale corrisponde
				for (int sett_id = 0; sett_id < SETTINGID_END; sett_id++) {
					Setting setting = Configurations::settings_list[sett_id];

					// Verifico se la linea (trimmed) inizia con l'abbreviazione o il nome
					if (line_setting == setting.m_name || line_setting == setting.m_abbr) {
						DEBUG_LOG("Identificata impostazione del valore del parametro con ID = %2d: <%s> = %s", setting.m_id, setting.m_name.c_str(), line_value.c_str());

						// Spezzo la stringa dei valori nei valori corrispondenti
						// Il delimitatore è la virgola
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

		// Reset dell'Indice
		this->m_session_index = 0;
	}

	/** Inizializzazione della lista di configurazioni */
	const Configurations::Setting Configurations::settings_list[] = {
			{ Testcases,					"Testcases", 								"#test", false },
			{ ProblemType,					"Problem type", 							"problem", false },
			{ AlphabetCardinality,			"Alphabet cardinality", 					"#alpha", true },
			{ EpsilonPercentage , 			"Epsilon percentage", 						"%epsilon", true },
			{ AutomatonStructure , 			"Automaton's structure type", 				"structure", false },
			{ AutomatonSize , 				"Automaton's size (#states)",	 			"#size", true },
			{ AutomatonFinalProbability , 	"Automaton's final states probability", 	"%finals", false },
			{ AutomatonTransitionsPercentage , "Automaton's transitions percentage", 	"%transitions", true },
			{ AutomatonMaxDistance , 		"Automaton's max distance", 				"maxdist", true },
			{ AutomatonSafeZoneDistance , 	"Automaton's safe-zone distance", 			"safezonedist", true },
			{ ActiveAutomatonPruning , 		"Active \"automaton pruning\"", 			"?autompruning", false },
			{ ActiveRemovingLabel , 		"Active \"removing label\"", 				"?removlabel", false },
			{ ActiveDistanceCheckInTranslation , "Active \"distance check in translation\"", "?distcheck",  false },
			{ PrintStatistics , 			"Print statistics", 						"?pstats", false },
			{ LogStatistics , 				"Log statistics in file", 					"?lstats", false },
			{ PrintOriginalAutomaton , 		"Print original automaton", 				"?porig", false },
			{ PrintSolutionAutomaton , 		"Print solution solution", 					"?psolu", false },
			{ DrawOriginalAutomaton , 		"Draw original automaton", 					"?dorig", false },
			{ DrawSolutionAutomaton , 		"Draw solution automaton", 					"?dsolu", false },
	};

	/**
	 * Metodo statico.
	 * Restituisce il nome del parametro di configurazione.
	 */
	string Configurations::nameOf(const SettingID& id) {
		return Configurations::getSetting(id).m_name;
	}

	/**
	 * Metodo statico.
	 * Restituisce la sigla identificativa del parametro di configurazione.
	 */
	string Configurations::abbreviationOf(const SettingID& id) {
		return Configurations::getSetting(id).m_abbr;
	}

	/**
	 * Metodo statico.
	 * Restituisce un flag booleano indicante se il valore del parametro deve
	 * essere stampato in coda ai test.
	 */
	bool Configurations::isTestParam(const SettingID& id) {
		return Configurations::getSetting(id).m_test_param;
	}

	/**
	 * Stampa i valori CORRENTI delle configurazioni attuali.
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
	 * Stampa i valori (completi!) di tutte configurazioni.
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
	 * Restituisce una stringa contenente NOME e VALORE associati al parametro
	 * passato in ingresso.
	 */
	string Configurations::toString(const SettingID& id) {
		return (Configurations::abbreviationOf(id) + ":" + this->m_settings_instances[this->m_session_index].at(id)->toString());
	}

	/**
	 * Imposta i parametri salvati all'interno delle configurazioni con la combinazione successiva
	 */
	bool Configurations::nextTestCase() {
		for (auto &pair : this->m_settings_instances[this->m_session_index]) {
			// Se viene trovato un caso successivo, si restituisce TRUE
			if (pair.second->nextCase()) { // Next case ha l'effetto collaterale di aumentare l'indice interno al parametro
				return true;
			}
		}
		// Se la combinazione di tutti i parametri è terminata, passo alla sessione successiva.
		this->m_session_index += 1;
		// A questo punto, se il nuovo indice rappresenta una configurazione caricata in memoria, tutto ok. Altrimenti finisco
		return (this->m_session_index < this->m_settings_instances.size());
	}


	template <class T> T Configurations::valueOf(const SettingID& id) {
		if (DEBUG_QUERY) {
			DEBUG_ASSERT_TRUE(this->m_settings_instances[this->m_session_index].count(id));
			DEBUG_LOG("Richiesta del valore di: %s", settings_list[int(id)].m_name.c_str());
		}
		SettingValue* associated_value_container = (this->m_settings_instances[this->m_session_index])[id];
		Value value = associated_value_container->getValue();
		switch (associated_value_container->getType()) {
		case INT :
			if (DEBUG_QUERY) { DEBUG_LOG("Valore restituito: %s", std::to_string(value.integer).c_str()); }
			return static_cast<int>(value.integer);
		case DOUBLE :
			if (DEBUG_QUERY) { DEBUG_LOG("Valore restituito: %s", std::to_string(value.real).c_str()); }
			return static_cast<double>(value.real);
		case BOOL :
			if (DEBUG_QUERY) { DEBUG_LOG("Valore restituito: %s", std::to_string(value.flag).c_str()); }
			return static_cast<bool>(value.flag);
		default :
			DEBUG_LOG_ERROR("Impossibile interpretare il valore %d", value.integer);
			return static_cast<int>(value.integer);
		}
	}


	template bool quicksc::Configurations::valueOf<bool>(quicksc::SettingID const&);
	template int quicksc::Configurations::valueOf<int>(quicksc::SettingID const&);
	template double quicksc::Configurations::valueOf<double>(quicksc::SettingID const&);
	template unsigned int quicksc::Configurations::valueOf<unsigned int>(quicksc::SettingID const&);

} /* namespace quicksc */
