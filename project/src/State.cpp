/*
 * State.cpp
 *
 * Project: TranslatedAutomata
 *
 * File sorgente per l'implementazione della classe astratta "State", definita
 * nel file header "state.hpp".
 * Nel dominio concettuale, uno "State" è uno stato di un "automa a stati finiti",
 * sia esso deterministico o non deterministico. E' caratterizzato da un nome
 * univoco ed è legato ad altri stati mediante transizioni, ciascuna delle quali
 * è marcata da una label.
 *
 */

#include "State.hpp"

#include <list>
#include <map>
#include <set>
#include <string>

#include "Alphabet.hpp"
//#define DEBUG_MODE
#include "Debug.hpp"

using std::list;
using std::map;
using std::set;
using std::string;

namespace quicksc {

	/**
	 * Classe astratta "State".
	 * Viene implementata da State e State.
	 */

	/**
	 * Costruttore della classe State.
	 * Inizializza come vuoti gli insiemi di transizioni entranti e uscenti.
	 */
	State::State (string name, bool final) {
		this->m_exiting_transitions = map<string, set<State*>>();
		this->m_incoming_transitions = map<string, set<State*>>();
		m_final = final;
		m_name = name;
		DEBUG_LOG( "Nuovo oggetto State creato correttamente" );
	}

	/**
	 * Distruttore della classe State.
	 */
	State::~State () {
		DEBUG_LOG( "Distruzione dell'oggetto State \"%s\"", m_name.c_str() );
	}

	/**
	 * Metodo privato.
	 * Restituisce il puntatore all'oggetto corrente, automaticamente
	 * castato con il tipo corretto.
	 */
	State* State::getThis() const {
		return (State*) this;
	}

	/**
	 * Restituisce il nome dello stato.
	 */
	string State::getName() const {
		return m_name;
	}

	/**
	 * Restituisce TRUE se lo stato è marcato come stato finale.
	 */
	bool State::isFinal() {
		return m_final;
	}

	/**
	 * Imposta lo stato come FINAL oppure NON_FINAL,
	 * a seconda del valore passato come parametro.
	 */
	void State::setFinal(bool final) {
		m_final = final;
	}

	/**
	 * Collega lo stato soggetto allo stato passato come parametro, con una transizione
	 * etichettata dalla label "label" passata come parametro.
	 * Nota: se la transizione esiste già, non viene aggiunta nuovamente.
	 */
	void State::connectChild(string label, State* child)	{
		bool flag_new_insertion = false;
		// Verifico se la label ha già un set associato nello stato CORRENTE.
		// In caso il set non ci sia, viene creato
		if (this->m_exiting_transitions.count(label) == 0) {
			this->m_exiting_transitions.insert({label, set<State*>()});
			flag_new_insertion = true;
		}

		// Verifico se la label ha già un set associato nello stato FIGLIO
		// In caso il set non ci sia, viene creato
		if (child->m_incoming_transitions.count(label) == 0) {
			child->m_incoming_transitions.insert({label, set<State*>()});
			flag_new_insertion = true;
		}

//		// FIXME Migliorare l'implementazione
//		bool flag_found = false;
//		for (auto iterator = this->m_exiting_transitions[label].begin();
//				!flag_new_insertion && iterator != this->m_exiting_transitions[label].end();
//				iterator++) {
//			if ((*iterator)->getName() == child->getName()) {
//				flag_found = true;
//			}
//		}

		// Se la label è un nuovo inserimento || Se per tale label, lo stato non è ancora presente
//		if (flag_new_insertion || !flag_found) {
		if (flag_new_insertion || !this->hasExitingTransition(label, child)) {
			// Aggiungo una transizione uscente da questo stato
			this->m_exiting_transitions[label].insert(child);
			// Aggiungo una transizione entrante allo stato di arrivo
			child->m_incoming_transitions[label].insert(getThis());
		}
	}

	/**
	 * Disconnette due stati, rimuovendo la l-transizione che parte da questo
	 * stato e arriva in quello passato come parametro.
	 *
	 * Precondizione: si suppone che tale transizione sia esistente
	 */
	void State::disconnectChild(string label, State* child) {
		if (!this->hasExitingTransition(label)) {
			DEBUG_LOG("Non sono presenti transizioni uscenti con label %s", label.c_str());
			return;
		}
		// Ricerca del figlio da disconnettere
		auto iterator = this->m_exiting_transitions[label].find(child);

		if (iterator != this->m_exiting_transitions[label].end()) {
			DEBUG_ASSERT_TRUE(this->hasExitingTransition(label, child));
			this->m_exiting_transitions[label].erase(iterator);
			DEBUG_ASSERT_FALSE(this->hasExitingTransition(label, child));
			child->m_incoming_transitions[label].erase(getThis());
		} else {
			DEBUG_LOG_FAIL("Figlio %s non trovato per la label %s", child->getName().c_str(), label.c_str());
			return;
		}
	}

	/**
	 * Rimuove tutte le transizioni entranti e uscenti
	 * da questo stato.
	 * Le transizioni vengono aggiornate anche sui nodi che
	 * risultavano precedentemente connessi.
	 */
	void State::detachAllTransitions() {
		// Rimuove le transizioni uscenti
		for (auto pair_it = m_exiting_transitions.begin(); pair_it != m_exiting_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto child_it = pair_it->second.begin(); child_it != pair_it->second.end(); ) {
				getThis()->disconnectChild(label, *(child_it++));
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}

		// Rimuove le transizioni entranti
		for (auto pair_it = m_incoming_transitions.begin(); pair_it != m_incoming_transitions.end(); pair_it++) {
			string label = pair_it->first;
			for (auto parent_iterator = pair_it->second.begin(); parent_iterator != pair_it->second.end(); ) {
				// Controllo che non sia una transizione ad anello
				if (*parent_iterator != this->getThis()) {
					(*(parent_iterator++))->disconnectChild(label, getThis());
				}
			}
			DEBUG_ASSERT_TRUE(pair_it->second.empty());
		}
	}

	/**
	 * Restituisce lo stato raggiunto da una transizione con una specifica etichetta.
	 * In pratica, fa le stesse operazioni del metodo "getChildren" ma prende solamente il
	 * primo nodo.
	 * Se non viene trovato alcun figlio relativo alla label passata come argomento,
	 * viene restituito un valore nullo.
	 */
	State* State::getChild(string label) {
		if (this->hasExitingTransition(label)) {
			IF_DEBUG_ACTIVE(
				if (this->getChildrenRef(label).size() > 1) {
					DEBUG_LOG_ERROR("Il nodo DFA \"%s\" contiene più di un figlio", this->getName().c_str());
				}
			)
			return *(this->getChildrenRef(label).begin());
		}
		// Se non ha figli con quella label
		else {
			return NULL;
		}
	}

	/**
	 * Metodo protetto.
	 * Restituisce l'insieme di stati raggiunti da tutte le transizioni
	 * uscenti da questo nodo marcate con la label "l" passata come parametro.
	 * In pratica, restituisce la l-closure di questo nodo.
	 *
	 * Nota: questo metodo verrà ereditato dalla classe State, poiché per un NFA
	 * è possibile avere più figli marcati con la stessa label. Nel caso di uno stato
	 * State, invece, sarà opportuno operare alcuni controlli per verificare che
	 * esista un unico figlio per ciascuna label.
	 */
	set<State*> State::getChildren(string label) {
		if (this->hasExitingTransition(label)) {
			// Restituisco i nodi alla transizione uscente
			return this->m_exiting_transitions[label];
		} else {
			// Restituisco un insieme vuoto
			return set<State*>();
		}
	}

	/**
	 * Restituisce gli stati che hanno un transizione marcata da una specifica label
	 * che punta a questo stato. In pratica, tutti gli stati "padri" secondo una certa
	 * label.
	 */
	set<State*> State::getParents(string label) {
		if (this->hasIncomingTransition(label)) {
			return this->m_incoming_transitions[label];
		} else {
			return set<State*>();
		}
	}

	/**
	 * Restituisce il riferimento al vettore di figli secondo una certa label.
	 * Attenzione: è necessario avere dei figli con tale label. Se ciò non si verifica, il comportamento di questo metodo è indefinito.
	 */
	const set<State*>& State::getChildrenRef(string label) {
		DEBUG_LOG("Ho chiamato il metodo per i figli della label %s", label.c_str());
		for (State* child : this->m_exiting_transitions[label]) {
			DEBUG_LOG("----> Figlio: %s", child->getName().c_str());
		}

		DEBUG_LOG("Ho dei figli con label %s", label.c_str());
		return this->m_exiting_transitions[label];
	}

	/**
	 * Restituisce il riferimento al vettore di genitori secondo una certa label.
	 * Attenzione: è necessario avere dei genitori con tale label. Se ciò non si verifica, il comportamento di questo metodo è indefinito.
	 */
	const set<State*>& State::getParentsRef(string label) {
		return this->m_incoming_transitions[label];
	}

	/**
	 * Verifica se lo stato soggetto ha una transizione USCENTE
	 * marcata con la label passata come parametro.
	 */
	bool State::hasExitingTransition(string label)	{
		auto search = this->m_exiting_transitions.find(label);
		return (search != this->m_exiting_transitions.end()) && !(this->m_exiting_transitions[label].empty());
	}

	/**
	 * Verifica se lo stato soggetto ha una transizione USCENTE che vada
	 * allo stato "child" e che sia marcata con l'etichetta "label".
	 */
	bool State::hasExitingTransition(string label, State* child) {
		if (this->m_exiting_transitions.count(label)) {
			return (this->m_exiting_transitions[label].find(child) != this->m_exiting_transitions[label].end());
		} else {
			return false;
		}
	}

	/**
	 * Verifica se lo stato soggetto ha una transizione ENTRANTE
	 * marcata con la label passata come parametro.
	 */
	bool State::hasIncomingTransition(string label)	{
		auto search = this->m_incoming_transitions.find(label);
		return (search != this->m_incoming_transitions.end()) && !(this->m_incoming_transitions[label].empty());
	}

	/**
	 * Verifica se lo stato soggetto ha una transizione ENTRANTE
	 * che parta dallo stato "parent" e che sia marcata con l'etichetta "label".
	 */
	bool State::hasIncomingTransition(string label, State* parent) {
		if (this->m_incoming_transitions.count(label)) {
			return (this->m_incoming_transitions[label].find(parent) != this->m_incoming_transitions[label].end());
		} else {
			return false;
		}
	}

	/**
	 * Restituisce la mappa di transizioni uscenti da questo stato.
	 */
	map<string, set<State*>> State::getExitingTransitions() {
		return m_exiting_transitions;
	}

	/**
	 * Restituisce la mappa di transizioni entranti in questo stato.
	 */
	map<string, set<State*>> State::getIncomingTransitions() {
		return m_incoming_transitions;
	}

	/**
	 * Restituisce una reference alla mappa di transizioni, ossia l'indirizzo della memoria
	 * in cui la mappa è salvata.
	 * Restituire un indirizzo permette di usare questo metodo come lvalue in un assegnamento, ad esempio.
	 */
	const map<string, set<State*>>& State::getExitingTransitionsRef() {
		return m_exiting_transitions;
	}

	/**
	 * Restituisce una reference alla mappa di transizioni entranti, ossia l'indirizzo della memoria
	 * in cui la mappa è salvata.
	 * Restituire un indirizzo permette di usare questo metodo come lvalue in un assegnamento, ad esempio.
	 */
	const map<string, set<State*>>& State::getIncomingTransitionsRef() {
		return m_incoming_transitions;
	}

	/**
	 * Conta le transizioni uscenti dallo stato.
	 * Per ciascuna label, conteggia la quantità di transizioni riferite a tale label uscenti
	 * dallo stato corrente.
	 */
	int State::getExitingTransitionsCount() {
		int count = 0;
		for (auto &pair: m_exiting_transitions) {// Ciclo su tutti gli elementi della mappa
			count += pair.second.size();
		}
		// "second" prende il secondo elemento della coppia,
		// in questo caso l'insieme di stati raggiunti dalle transizioni con label predefinita
		return count;
	}

	/**
	 * Conta le transizioni entranti nello stato.
	 */
	int State::getIncomingTransitionsCount() {
		int count = 0;
		for (auto &pair: m_incoming_transitions) {
			count += pair.second.size();
		}
		return count;
	}
    /**
     * Guarda tutte le transizioni uscenti dallo stato "state".
     * Se trova una transizione che questo stato non possiede,
     * la aggiunge come transizione propria.
     * Al termine dell'esecuzione, è garantito che questo stato
     * contenga almeno tutte le transizioni uscenti dallo stato
     * passato come parametro.
     */
	void State::copyExitingTransitionsOf(State* state) {
    	// Per tutte le transizioni uscenti dallo stato "state"
        for (auto &pair: state->getExitingTransitionsRef()) {
            string label = pair.first;
            for (State* child: pair.second) {
                if (!this->hasExitingTransition(label, child)) {
                    this->connectChild(label, child);
                }
            }
        }
    }

    /**
     * Guarda tutte le transizioni entranti nello stato "state".
     * Se trova una transizione che questo stato non possiede,
     * la aggiunge come transizione propria.
     * Al termine dell'esecuzione, è garantito che questo stato
     * contenga almeno tutte le transizioni entranti nello stato
     * passato come parametro.
     */
	void State::copyIncomingTransitionsOf(State* state) {
    	// Per tutte le transizioni entranti nello stato "state"
        for (auto &pair: state->getIncomingTransitionsRef()) {
            string label = pair.first;
            for (State* parent: pair.second) {
                if (!parent->hasExitingTransition(label, this->getThis())) {
                    parent->connectChild(label, this->getThis());
                }
            }
        }
    }

    /**
     * Copia tutte le transizioni entranti in e uscenti da uno stato d
     * all'interno di questo stato.
     * Le transizioni già esistenti non vengono duplicate.
     */
	void State::copyAllTransitionsOf(State* state) {
        copyIncomingTransitionsOf(state);
        copyExitingTransitionsOf(state);
    }

	/**
	 * Verifica se lo stato ha le stesse transizioni (entranti E uscenti) dello stato
	 * passato come parametro.
	 * Le transizioni vengono confrontate tramite puntatore; gli stati devono perciò essere
	 * effettivamente gli stessi.
	 */
	bool State::hasSameTransitionsOf(State* other_state) {
		// Verifico che il numero di transizioni uscenti sia uguale
		if (this->m_exiting_transitions.size() != other_state->m_exiting_transitions.size()) {
			return false;
		}

		// Per tutte le transizioni uscenti
		for (auto &pair: m_exiting_transitions) {
			string label = pair.first;
			set<State*> other_children = other_state->m_exiting_transitions[label];

			// Verifico che il numero di figli sia uguale
			if (pair.second.size() != other_children.size()) {
				return false;
			}

			// Verifico che tutti i figli siano coincidenti
			for (auto &child: pair.second) {
				// Se il figlio non è contenuto nell'altra lista
				if (other_children.count(child) <= 0) {
					return false;
				}
			}
		}

		// Verifico che il numero di transizioni entranti sia uguale
		if (this->m_incoming_transitions.size() != other_state->m_incoming_transitions.size()) {
			return false;
		}

		// Per tutte le transizioni entranti
		for (auto &pair: m_incoming_transitions) {
			string label = pair.first;
			set<State*> other_parents = other_state->m_incoming_transitions[label];

			// Verifico che il numero di padri sia uguale
			if (pair.second.size() != other_parents.size()) {
				return false;
			}

			// Verifico che tutti i padri siano coincidenti
			for (auto &parent: pair.second) {
				// Se il padre non è contenuto nell'altra lista
				if (other_parents.count(parent) <= 0) {
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * Metodo che verifica se le transizioni presenti nello stato corrispondono (solo
	 * a livello di nome) con le transizioni dello stato passato come parametro.
	 * Questo metodo è utilizzato per confrontare due automi isomorfi, senza che siano
	 * presenti stati in comune fra essi, ma solamente stati omonimi.
	 */
	bool State::hasSameTransitionsNamesOf(State* other_state) {
		// Verifico che il numero di transizioni uscenti sia uguale
		if (this->getExitingTransitionsCount() != other_state->getExitingTransitionsCount()) {
			DEBUG_LOG("I due stati non hanno lo stesso numero di transizioni uscenti");
			return false;
		}

		// Per tutte le transizioni uscenti
		for (auto &pair : m_exiting_transitions) {
			string label = pair.first;
			set<State*> other_children = other_state->m_exiting_transitions[label];

			// Verifico che il numero di figli sia uguale
			if (pair.second.size() != other_children.size()) {
				return false;
			}

			// Verifico che tutti i figli siano coincidenti
			for (auto &child : pair.second) {
				// Se il figlio non è contenuto nell'altra lista
				bool found_flag = false;
				for (auto other_child : other_children) {
					// Appena trovo una corrispondenza, esco dal ciclo
					if (*child == *other_child) {
						found_flag = true;
						break;
					}
				}
				// Se non ho trovato corrispondenze, esco
				if (!found_flag) {
					return false;
				}
			}
		}
		/* NOTA: In teoria, se i due automi sono stati ben formati secondo i metodi offerti dalle classi del model,
		 * le transizioni entranti corrisponderanno sempre a quelle uscenti. Pertanto non è necessario verificare
		 * entrambe. */
		/*
		// Verifico che il numero di transizioni entranti sia uguale
		if (this->m_incoming_transitions.size() != other_state->m_incoming_transitions.size()) {
			return false;
		}

		// Per tutte le transizioni entranti
		for (auto &pair: m_incoming_transitions) {
			string label = pair.first;
			set<State*> other_parents = other_state->m_incoming_transitions[label];

			// Verifico che il numero di padri sia uguale
			if (pair.second.size() != other_parents.size()) {
				return false;
			}

			// Verifico che tutti i genitori siano coincidenti
			for (auto &parent : pair.second) {
				// Se il genitore non è contenuto nell'altra lista
				bool found_flag = false;
				for (auto other_parent : other_parents) {
					// Appena trovo una corrispondenza, esco dal ciclo
					if (*parent == *other_parent) {
						found_flag = true;
						break;
					}
				}
				// Se non ho trovato corrispondenze, esco
				if (!found_flag) {
					return false;
				}
			}
		}
		*/
		return true;
	}

    /**
     * Restituisce la distanza di questo stato.
     */
    unsigned int State::getDistance() {
        return m_distance;
    }

    /**
     * Imposta la distanza di questo stato.
     */
    void State::setDistance(unsigned int distance) {
        m_distance = distance;
    }

    /**
     * Imposta i valori delle distanze partendo dallo stato corrente
     * come radice, considerando ongi transizione come un incremento unitario sulla
     * distanza.
     * In realtà l'assegnamento delle distanze non è implementato con un algoritmo ricorsivo, ma si utilizza
     * una coda che permette un'attraversamento dell'automa di tipo breadth-first (attraversamento in ampiezza).
     * Nota: questo metodo viene richiamato appena terminata la costruzione di un automa, per definire le distanze
     * di tutti gli stati; è quindi importante che tutti gli stati facciano parte dello stesso automa, e che tutti abbiano
     * una distanza non ancora inizializzata (ossia uguale al valore di default).
     */
    void State::initDistancesRecursively(int root_distance) {
    	// Imposto la distanza corrente
        this->setDistance(root_distance);
        // Preparo la lista di nodi su cui effettuare l'aggiornamento della distanza
        list<State*> updated_list;
        updated_list.push_back(this->getThis());

        while (updated_list.size() > 0) {		// Finché la queue non è vuota

            State* current_state = updated_list.front();
            updated_list.pop_front();

            // Per tutte le transizioni uscenti dallo stato corrente
            for (auto &trans: current_state->getExitingTransitionsRef()) {
            	// Per tutti i figli raggiunti
                for (State* child : trans.second) {
                	// Se la distanza non vale quanto la distanza iniziale di default
                    if (child->getDistance() == DEFAULT_VOID_DISTANCE) {
                    	// Imposto la distanza del figlio = 1 + dist(padre)
                        child->setDistance(current_state->getDistance() + 1);
                        // Aggiungo il figlio alla lista, in modo da aggiornarlo in futuro
                        updated_list.push_back(child);
                    }
                }
            }
        }
    }

    /**
     * Restituisce la minima distanza fra tutte le distanze dei genitori.
     */
    int State::getMinimumParentsDistance() {
    	int minimum = DEFAULT_VOID_DISTANCE;
    	// Per tutte le transizioni entranti
    	for (auto &pair : this->getIncomingTransitionsRef()) {
    		// Per tutti gli stati genitori
    		for (State* parent : pair.second) {
    			// Se la distanza è inferiore
    			if (parent->m_distance < minimum) {
    				minimum = parent->m_distance;
    			}
    		}
    	}
    	return minimum;
    }

	/**
	 * Restituisce una stringa contenente tutte le informazioni relative allo stato.
	 */
	string State::toString() const {
		// Stringa che verrà restituita
		string result = "";

		// Inserisco il nome dello stato
		result += "\033[33;1m" + getThis()->getName() + "\033[0m";

		// Distanza dello stato
		result += " (dist = " + std::to_string(m_distance) + ")";

		// Se lo stato è final, aggiungo un'etichetta alla stringa visualizzata
		if (getThis()->isFinal()) {
			result += " [FINAL]";
		}

		result += "\n\t" + (std::to_string(getThis()->getExitingTransitionsCount())) + " exiting transitions:\n";
		if (!this->m_exiting_transitions.empty()) {
			// Per tutte le label delle transizioni uscenti
			for (auto &pair: m_exiting_transitions) {
				string label = pair.first;
				// Per tutti gli stati associati ad una label
				for (State* state: pair.second) {
					// Inserisco le informazioni riguardanti la transizione uscente
					result += "\t━━┥" + SHOW(label) + "┝━━▶ " + state->getName() + "\n";
//					result += "\t━━┥" + SHOW(label) + "┝━━▶ " + state->getName() + "\033[35m[id = " + (std::to_string((long int)state)) + "]\033[0m" + "\n";
				}
			}
		}

/*
		result += "\t" + (std::to_string(getThis()->getIncomingTransitionsCount())) + " incoming transitions:\n";
		if (!this->m_incoming_transitions.empty()) {
			// Per tutte le label delle transizioni entranti
			for (auto &pair: m_incoming_transitions) {
				string label = pair.first;
				// Per tutti gli stati associati ad una label
				for (State* state: pair.second) {
					// Inserisco le informazioni riguardanti la transizione entrante
					result += "\t" + state->getName() + " ━━(" + SHOW(label) + ")━━▶" + '\n';
				}
			}
		}
//*/

		return result;
	};

	/**
	 * Definisce un operatore "<" di confronto, basato sul confronto dei nomi.
	 */
	bool State::operator<(const State &other) const	{
		return getThis()->getName() < other.getName();
	}

	/**
	 * Definisce un operatore di uguaglianza "==", basato sull'uguaglianza dei nomi.
	 */
	bool State::operator==(const State &other) const {
		return getThis()->getName() == other.getName();
	}

	/**
	 * Definisce un operatore di disuguaglianza "!=", basato sul confronto dei nomi.
	 */
	bool State::operator!=(const State &other) const {
			return getThis()->getName() != other.getName();
	}

	/**
	 * Confronta due stati sulla base del loro nome.
	 */
//	template <class S>
//	int State::compareNames(const S &other) const {
//		return getThis()->getName().compare(other.getName());
//	}\

///////////////////////////////////////////////////////////////////

	/**
	 * Metodo statico.
	 * Crea il nome dello stato concatenando i nomi degli stati dell'estensione.
	 * Questo metodo è utilizzato in automatico per stabilire il nome di un oggetto
	 * ConstructedState ogni volta che viene assegnata o modificata la sua
	 * estensione.
	 */
	string ConstructedState::createNameFromExtension(const Extension &ext) {
		if (ext.empty()) {
			return EMPTY_EXTENSION_NAME;
		}

		// Inizializzo la stringa
		string name = "{";

		// Per ciascuno stato dell'estensione, aggiungo il nome alla lista
		for (State* s: ext) {
			name += s->getName() + ',';
		}
		// Rimuovo la virgola in coda
		name.pop_back();
		name += "}";

		return name;
	}

	/**
	 * Metodo statico.
	 * Sottrae dalla prima estensione gli stati della seconda estensione.
	 * Considerando le estensioni come insiemi, opera una differenza insiemistica e restituisce il risultato.
	 */
	Extension ConstructedState::subtractExtensions(const Extension &ext1, const Extension &ext2) {
		Extension result = set<State*, State::Comparator>();

		for (State* s : ext1) {
			if (ext2.count(s) == 0) {
				result.insert(s);
			}
		}

		return result;
	}

	/**
	 * Metodo statico.
	 * Computa la epsilon chiusura di un'estensione, ovvero di un'insieme di stati (generalmente un insieme di stati di un NFA).
	 */
	Extension ConstructedState::computeEpsilonClosure(const Extension &ext) {
		Extension result = set<State*, State::Comparator>(ext);
		list<State*> queue = list<State*>();
		for (State* s : ext) {
			queue.push_back(s);
		}

		while (!queue.empty()) {
			// Estraggo lo stato da processare
			State* current = queue.front();
			queue.pop_front();
			// Calcolo gli stati raggiungibili tramite epsilon transitions
			set<State*> closure = current->getChildren(EPSILON);
			// Per ciascuno di essi
			for (State* epsilon_child : closure) {
				// Aggiungo lo stato alla epsilon-chiusura dell'estensione
				// Se NON era già contenuto
				if (result.insert(epsilon_child).second) {
					// Allora aggiungo lo stato anche alla coda di stati da processare
					queue.push_back(epsilon_child);
				}
			}
		}

		return result;
	}

	/**
	 * Metodo statico.
	 * Computa la epsilon chiusura di un singolo stato.
	 */
	Extension ConstructedState::computeEpsilonClosure(State* state) {
		// Creazione di un'estensione (insieme di stati) con solo l'insieme iniziale
		Extension result = set<State*, State::Comparator>();
		result.insert(state);
		// Creazione di una coda con solo l'insieme iniziale
		list<State*> queue = list<State*>();
		queue.push_back(state);

		while (!queue.empty()) {
			// Estraggo lo stato da processare
			State* current = queue.front();
			queue.pop_front();
			// Calcolo gli stati raggiungibili tramite epsilon transizioni
			set<State*> closure = current->getChildren(EPSILON);
			// Per ciascuno di essi
			for (State* epsilon_child : closure) {
				// Aggiungo lo stato alla epsilon-chiusura dell'estensione
				// Se NON era già contenuto
				if (result.insert(epsilon_child).second) {
					// Allora aggiungo lo stato anche alla coda di stati da processare
					queue.push_back(epsilon_child);
				}
			}
		}

		return result;
	}

	/**
	 * Metodo statico.
	 * Restituisce "true" se e solo se esiste almeno uno stato NFA all'interno
	 * dell'estensione marcato come stato finale.
	 */
	bool ConstructedState::hasFinalStates(const Extension &ext) {
		for (State* state_nfa : ext) {
			if (state_nfa->isFinal()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Costruttore della classe ConstructedState.
	 * Assegna allo stato l'estensione passata come parametro e imposta il nome dello stato.
	 * Prima della costruzione viene chiamato il costruttore della classe padre "State" mediante
	 * l'utilizzo di due metodi statici che operano sull'estensione per ottenere il nome dello stato
	 * e il valore booleano rappresentante se lo stato è final o no.
	 */
	ConstructedState::ConstructedState(Extension &extension)
		: State(ConstructedState::createNameFromExtension(extension), ConstructedState::hasFinalStates(extension)) {

		this->m_extension = extension;
	}

	/**
	 * Distruttore della classe ConstructedState.
	 */
	ConstructedState::~ConstructedState() {
		this->m_extension.clear();
	}

	/**
	 * Imposta lo stato con il valore di marcatura passato come parametro.
	 */
	void ConstructedState::setMarked(bool mark) {
		this->m_mark = mark;
	}

	/**
	 * Restituisce true se lo stato è marcato.
	 */
	bool ConstructedState::isMarked() {
		return this->m_mark;
	}

	/**
	 * Verifica se lo stato ha una specifica estensione passata come parametro.
	 * Il confronto viene effettuato mediante la stringa contenente tutti i nomi dell'estensione.
	 */
	bool ConstructedState::hasExtension(const Extension &ext) {
		return (getName() == createNameFromExtension(ext));
	}

	/**
	 * Restituisce l'estensione dello stato, ossia l'insieme di State
	 * da cui questo stato è stato creato.
	 */
	const Extension& ConstructedState::getExtension() {
		return m_extension;
	}

	/**
	 * Restituisce tutte le etichette delle transizioni uscenti dagli stati
	 * dell'estensione.
	 */
	set<string>& ConstructedState::getLabelsExitingFromExtension() {
		set<string> *labels = new set<string>;
		DEBUG_ASSERT_TRUE(labels->size() == 0);

		// Per ciascuno stato dell'estensione
		for (State* member : m_extension) {
			DEBUG_LOG("Per lo stato dell'estensione \"%s\"", member->getName().c_str());
			// Inserisco le label delle transizioni uscenti
			for (auto &pair: member->getExitingTransitionsRef()) {
				// Se la label marca almeno una transizione
				DEBUG_LOG("Numero di transizioni marcate dalla label %s: %lu", pair.first.c_str(), pair.second.size());
				if (pair.second.size() > 0) {
					DEBUG_LOG("Aggiungo la label \"%s\"", pair.first.c_str());
					labels->insert(pair.first);
				}
			}
		}
		DEBUG_LOG("Lunghezza finale dell'insieme di labels: %lu", labels->size());
		return *labels;
	}

	/**
	 * Restituisce la l-closure data la stringa etichetta:
	 * per tutti gli stati della propria estensione calcola la
	 * l-closure, poi degli stati raggiunti calcola la epsilon-chiusura
	 * Si suppone, pertanto, che l'estensione presente nello stato sia
	 * sempre epsilon-chiusa.
	 */
	Extension ConstructedState::computeLClosureOfExtension(string label) {
//		// Epsilon-chiusura della estensione corrente
//		Extension eps_closure = ConstructedState::computeEpsilonClosure(this->m_extension);

		// Computazione degli stati raggiunti tramite label L
		Extension l_closure;
		// Scorro su tutti gli stati dell'estensione (NFA)
		for (State* member : this->m_extension) {
			for (State* child : member->getChildren(label)) {
				l_closure.insert(child);
			}
		}

		// Epsilon chiusura degli stati raggiunti
		return ConstructedState::computeEpsilonClosure(l_closure);
	}

	/**
	 * Restituisce la ell-chiusura dello stato, data l'etichetta.
	 * In altre parole, calcola:
	 * - l'insieme di tutti gli ell-figli di questo stato
	 * - la epsilon chiusura degli ell-figli
	 *
	 * NOTA: Questo metodo assume che lo stato NON abbia epsilon-transizioni uscenti. Questo perché è usato in algoritmi (QSC) che dovrebbero garantire questa condizione.
	 */
	Extension ConstructedState::computeLClosure(string label) {
		// Computazione degli stati raggiunti tramite label L
		Extension l_closure;
		// Scorro su tutti gli stati figli raggiunti dalla label
		for (State* child : this->getChildren(label)) {
			l_closure.insert(child);
		}

		// Epsilon chiusura degli stati raggiunti
		return ConstructedState::computeEpsilonClosure(l_closure);
	}

	/**
	 * Sostituisce interamente l'estensione di questo stato con un'altra.
	 *
	 * Nota: questo metodo causa anche il cambio del nome dello stato, basato
	 * sugli stati dell'NFA che sono contenuti nella nuova estensione.
	 */
	void ConstructedState::replaceExtensionWith(Extension &new_ext) {
		this->m_extension = new_ext;
		this->m_name = createNameFromExtension(m_extension);
		this->m_final = hasFinalStates(m_extension);
	}

	/**
	 * Verifica se lo stato è "vuoto", ossia se la sua estensione è vuota.
	 */
	bool ConstructedState::isExtensionEmpty() {
		return m_extension.empty();
	}

	/**
	 * Verifica se lo stato è safe rispetto ad una data singolarità.
	 * La singolarità viene spezzata nei suoi due componenti.
	 *
	 * ATTENZIONE: Si assume che lo stato APPARTENGA alla chiusura della singolarità.
	 */
	// bool ConstructedState::isSafe(Singularity* singularity) {
	// 	return this->isSafe(singularity->getState(), singularity->getLabel());
	// }

	/**
	 * Verifica se lo stato è safe rispetto ad una data singolarità, rappresentata da:
	 * - Uno stato
	 * - Un'etichetta
	 *
	 * Uno stato è <safe> se e solo se si verifica una delle seguenti condizioni:
	 * - E' lo stato iniziale dell'automa (ossia la sua distanza è zero).
	 * - Esiste una transizione entrante nello stato che valgano entrambe:
	 * 		- la transizione deve essere diversa dalla transizione rappresentata dalla singolarità
	 *		- lo stato di provenienza deve avere distanza minore o uguale allo stato della singolarità
	 *
	 * ATTENZIONE: Si assume che lo stato APPARTENGA alla chiusura della singolarità.
	 */
	bool ConstructedState::isSafe(State* singularity_state, string singularity_label) {
		// Verifico se la distanza è 0, ossia se è lo stato iniziale
		if (this->getDistance() == 0) {
			return true;
		}

		// Verifico l'esistenza di una transizione entrante con le caratteristiche desiderate
    	for (auto &pair : this->getIncomingTransitionsRef()) {
			// Estraggo la label della transizione corrente
			string label = pair.first;

			// Caso in cui l'etichetta sia la stessa della singolarità
			if (label == singularity_label) {

				// Per tutti gli stati genitori
				for (State* parent : pair.second) {

					// Se la label è uguale
					if (parent != singularity_state
						&& parent->getDistance() <= singularity_state->getDistance()) {
						return true;
					}
				}
			}
			// Caso in cui l'etichetta sia diversa da quella della singolarità
			else {

	    		// Per tutti gli stati genitori
	    		for (State* parent : pair.second) {

	    			// So già che l'etichetta è diversa, è sufficiente controllare la distanza
	    			if (parent->getDistance() <= singularity_state->getDistance()) {
						return true;
	    			}
	    		}
			}
    	}
		// Se tutti i controlli sono falliti, lo stato non presenta le condizioni per qualificarsi come stato safe. Pertanto viene restituito <false>.
    	return false;
	}

	/**
	 * Uno stato è <unsafe> se e solo se non è <safe>.
	 */
	// bool ConstructedState::isUnsafe(Singularity* singularity) {
	// 	return !this->isSafe(singularity);
	// }

	/**
	 * Uno stato è <unsafe> se e solo se non è <safe>.
	 */
	bool ConstructedState::isUnsafe(State* singularity_state, string singularity_label) {
		return !this->isSafe(singularity_state, singularity_label);
	}

}
