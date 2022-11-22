/*
 * Automaton.cpp
 *
 * Project: TranslatedAutomaton
 *
 * File sorgente contenente la classe "Automaton", che rappresenta in astratto
 * un Automa a stati finiti.
 * Questa classe viene ereditata dalle due sottoclassi concrete DFA e NFA,
 * rispettivamente Automa a Stati Finit Non Deterministico e Automa a Stati Finiti
 * Deterministico.
 *
 */

#include "Automaton.hpp"

#include <algorithm>

//#define DEBUG_MODE
#include "Debug.hpp"

namespace quicksc {

	/**
	 * Costruttore della classe Automaton.
	 * Istanzia lo stato iniziale a NULL
	 * e la mappa degli stati come vuota.
	 */
    Automaton::Automaton()
	: m_states() {
    	m_initial_state = NULL;
    }


    /**
     * Distruttore della classe Automaton.
     * Distrugge TUTTI gli stati contenuti nell'automa.
     */
    Automaton::~Automaton() {
    	DEBUG_MARK_PHASE("Distruzione di un automa con %lu stati", this->m_states.size()) {

    	for (State* s : m_states) {
    		s->detachAllTransitions();
    	}
    	for (State* s : m_states) {
//    		this->m_states.erase(s);
    	}

    	}
    }

    /**
     * Restituisce la dimensione dell'automa, ossia il numero di stati.
     */
    int Automaton::size() {
        return m_states.size();
    }

    /**
     * Verifica se un automa contiene lo stato s al suo interno.
     * I confronti avvengono tramite puntatore, NON tramite nome.
     * Per effettuare un confronto tramite nomi è opportuno utilizzare
     * l'omonimo metodo che accetta in ingresso una stringa.
     */
    bool Automaton::hasState(State* s) {
        return m_states.find(s) != m_states.end();
    }

    /**
     * Verifica se un automa contiene uno stato con un nome specifico al suo interno.
     * I confronti vengono effettuati tramite nome, non tramite puntatore.
     * In caso di stati ominimi, questo metodo restituirebbe comunque un risultato affermativo.
     */
    bool Automaton::hasState(string name) {
    	for (State* s : m_states) {
    		if (s->getName() == name) {
    			return true;
    		}
    	}
    	return false;
    }

    /**
     * Ritorna - se presente - lo stato corrispondente alla label passata come parametro.
     * In caso questo stato non faccia parte dell'automa, viene restituito un puntatore NULL.
     *
     * Nota: In alcuni casi (durante l'esecuzione di algoritmi di costruzione) esiste più di uno stato
     * con lo stesso nome. Questa condizione si verifica solo all'interno di un'esecuzione e viene "corretta" al termine,
     * ma tuttavia non permette il corretto funzionamento di questo metodo (che invece restituisce il primo stato trovato).
     * Per evitare l'insorgere di problematiche legate a stati omonimi, si consiglia di utilizzare il metodo
     * "getStatesByName" che restituisce un insieme contenente TUTTI gli stati con il medesimo nome.
     */
    State* Automaton::getState(string name) {
    	for (State* s : m_states) {
    		if (s->getName() == name) {
    			return s;
    		}
    	}
    	return NULL;
    }

    /**
     * Restituisce l'insieme di tutti gli stati aventi il nome passato come parametro.
     * Normalmente questo metodo restituisce un unico stato, poiché gli stati sono unici per nome.
     * Tuttavia, durante l'esecuzione di algoritmi di costruzione, l'automa potrebbe trovarsi in condizioni particolari per cui
     * esiste più di uno stato con lo stesso nome. In quel caso, è bene utilizzare questo metodo e non "getState".
     */
    const vector<State*> Automaton::getStatesByName(string name) {
    	vector<State*> namesake_states; // Insieme di stati omonimi
    	for (State* s : m_states) {
    		if (s->getName() == name) {
    			namesake_states.push_back(s);
    		}
    	}
    	return namesake_states;
    }

    /**
     * Aggiunge uno stato alla mappa degli stati di questo automa.
     * Nel caso esi già uno stato associato a tale nome, quello pre-esistente viene sovrascritto.
     */
    void Automaton::addState(State* s) {
        m_states.insert(s);
    }

    /**
     * Richiede in ingresso uno stato.
     * Rimuove dall'automa lo stato dell'automa che abbia come nome il nome
     * dello stato in ingresso.
     * Se la rimozione avviene restituisce "TRUE", altrimenti se lo stato
     * non viene trovato restituisce "FALSE".
	 * Questo metodo NON distrugge lo stato.
     */
    bool Automaton::removeState(State* s) {
    	DEBUG_MARK_PHASE("Function \"detachAllTransitions\" sullo stato %s", s->getName().c_str()) {
    		s->detachAllTransitions();
    	}
    	DEBUG_LOG("Verifica dello stato dopo la funzione \"detachAllTransitions\" e prima di essere rimosso:\n%s", s->toString().c_str());
    	DEBUG_ASSERT_TRUE(this->hasState(s));
    	m_states.erase(s);
    	DEBUG_ASSERT_FALSE(this->hasState(s));
    	return true;
    	// FIXME
    }

    /**
     * Imposta uno stato come stato iniziale.
     * All'istanziazione, il riferimento allo stato iniziale ha valore NULL.
     *
     * In caso questo metodo venga chiamato una seconda volta, il nodo iniziale viene
     * "sovrascritto", e si perde il riferimento al precedente.
     *
     * Nota: non è possibile impostare come stato iniziale uno stato non appartenente all'automa.
     * In caso lo stato non faccia parte dell'automa, l'operazione non verrà effettuata.
     *
     * Inoltre, si suppone che questa operazione venga effettuata al termine dell'inserimento di tutti
     * gli stati, poiché causa anche l'assegnamento delle distanze per ogni nodo raggiungibile dallo stato
     * iniziale. Pertanto, al termine di questa chiamata, ogni stato conterrà la distanza dal nodo iniziale,
     * a partire dal nodo iniziale che avrà distanza 0.
     * L'assegnamento delle distanze NON ha effetto sugli stati NON raggiungibili dallo stato impostato come
     * stato iniziale.
     */
    void Automaton::setInitialState(State* s) {
    	if (hasState(s)) {
			m_initial_state = s;
	    	s->initDistancesRecursively(0);
    	} else {
    		DEBUG_LOG_ERROR("Impossibile impostare %s come stato iniziale poiché non appartenente all'automa", s->getName().c_str());
    	}
    }

    /**
     * Imposta come stato iniziale lo stato dell'automa associato al nome passato come parametro.
     *
     * In caso questo metodo venga chiamato più di una volta, il nodo iniziale viene
     * "sovrascritto" e viene perso il riferimento al precedente.
     *
     * Nota: non è possibile impostare come stato iniziale uno stato non appartenente all'automa.
     * In caso lo stato non faccia parte dell'automa, l'operazione non verrà effettuata.
     *
     * Inoltre, si suppone che questa operazione venga effettuata al termine dell'inserimento di tutti
     * gli stati, poiché causa anche l'assegnamento delle distanze per ogni nodo raggiungibile dallo stato
     * iniziale. Pertanto, al termine di questa chiamata, ogni stato conterrà la distanza dal nodo iniziale,
     * a partire dal nodo iniziale che avrà distanza 0.
     * L'assegnamento delle distanze NON ha effetto sugli stati NON raggiungibili dallo stato impostato come
     * stato iniziale.
     */
    void Automaton::setInitialState(string name) {
    	if (hasState(name)) {
			m_initial_state = getState(name);
	    	m_initial_state->initDistancesRecursively(0);
    	}
    }

    /**
     * Verifica se uno stato associato ad un certo nome è impostato come stato iniziale.
     */
    bool Automaton::isInitial(string name) {
        return (m_initial_state->getName() == name);
    }

    /**
     * Verifica se uno stato è impostato come stato iniziale.
     */
    bool Automaton::isInitial(State* s) {
        return (m_initial_state == s);
        /* Nota:
         * Prima il confronto veniva effettuato tramite i nomi, ma poiché è stato definito
         * l'operatore di uguaglianza come confronto di nomi, quest'implementazione non dovrebbe
         * dare problemi.
         */
    }

    /**
     * Restituisce il nodo iniziale
     */
    State* Automaton::getInitialState() {
        return m_initial_state;
    }

    /**
     * Metodo privato.
     *
     * Rimuove da un insieme tutti i nodi che sono raggiungibili (leggi: CONNESSI) dal nodo s passato come parametro
     * mediante una transizione s->s', più tutti quelli che sono raggiungibili anche da essi in cascata.
     * Inoltre, rimuove anche lo stato s.
     * Questo metodo è usato per rimuovere tutti i nodi connessi (in maniera DIREZIONALE) allo stato iniziale,
     * ossia tutti quelli raggiungibili nell'automa.
     */
    void Automaton::removeReachableStates(State* s, set<State*> &states) {
    	// Verifico se contengo lo stato s
        if (states.find(s) != states.end()) {
            states.erase(s);
            // Rimuovo lo stato dalla mappa. Se lo rimuovessi DOPO la chiamata ricorsiva, un ciclo
            // di transizioni sugli stati genererebbe uno stack di chiamate ricorsive illimitato.

            // Per tutte le transizioni uscenti da s
            for (auto &pair: s->getExitingTransitions()) {
            	// Per ciascuno stato raggiunto dalle transizioni
                for (State* child: pair.second) {
                	removeReachableStates(child, states); // Chiamata ricorsiva sui figli
                }
            }
        }
    }

    /**
     * Rimuove gli stati dell'automa che non sono più raggiungibili dallo stato iniziale,
     * ossia tutti gli stati dell'automa che non possono essere "visitati" tramite una sequenza di transizioni.
     *
     * L'idea è quella di prendere l'insieme di tutti gli stati dell'automa e rimuovere quelli raggiungibili.
     * Gli stati che rimarranno saranno necessariamente gli stati irraggiungibili.
     *
     * Restituisce gli stati che sono stati rimossi e che risultavano irraggiungibili.
     */
    set<State*> Automaton::removeUnreachableStates() {
    	// Creo l'insieme di tutti gli stati dell'automa
        set<State*> unreachable = set<State*>(m_states.begin(), m_states.end());

        // Lavoro per differenza: rimuovo dall'insieme tutti gli stati raggiungibili
        removeReachableStates(m_initial_state, unreachable);

        // Mi restano tutti gli stati irrangiungibili, sui quali itero
        for (State* s: unreachable) {
        	// Rimuovo dalla mappa dell'automa ogni stato irraggiungibile
            m_states.erase(s);
        }

        return unreachable;
    }

    /**
     * Restituisce la lista di tutti gli stati dell'automa sotto forma di list.
     * Gli stati sono restituiti come puntatori.
     * La classe "list" permette un'aggiunta o rimozione in mezzo alla lista
     * senza reallocazione della coda.
     */
    const list<State*> Automaton::getStatesList() {
    	return list<State*>(m_states.begin(), m_states.end());
    }

    /**
     * Restituisce il vettore dinamico di tutti gli stati dell'automa sotto forma di vector.
     * Gli stati sono restituiti come puntatori.
     * La classe "vector" permette un accesso casuale con tempo costante.
     */
    const vector<State*> Automaton::getStatesVector() {
    	return vector<State*>(m_states.begin(), m_states.end());
    }

    /**
     * Resituisce il numero di transizioni totali dell'automa.
     */
    unsigned int Automaton::getTransitionsCount() {
        unsigned int count = 0;
        for (State* s : m_states) {
            count += s->getExitingTransitionsCount();
        }
        return count;
    }

    /**
     * Restituisce l'alfabeto dell'automa.
     * NOTA: Questo metodo non restituisce necessariamente tutto l'alfabeto su cui è stato
     * definito in principio l'automa, poiché un automa NON mantiene un riferimento a tale
     * alfabeto.
     * Al contrario, computa l'alfabeto analizzando tutte le labels presenti in tutte le transizioni
     * dell'automa. Per questo motivo, può essere dispendioso in termini di performance.
     */
    const Alphabet Automaton::getAlphabet() {
        Alphabet alphabet = Alphabet();
        for (State* s : m_states) {
            for (auto &trans: s->getExitingTransitionsRef()) {
            	auto iterator = std::find(alphabet.begin(), alphabet.end(), trans.first);
            	if (iterator == alphabet.end()) {
            		alphabet.push_back(trans.first);
            	}
            }
        }

        return alphabet;
    }

    /**
     * Inserisce una transizione fra i due stati marcata con la label passata come parametro.
     * Il metodo funziona solamente se entrambi gli stati sono appartenenti all'automa, e in
     * tal caso restituisce TRUE.
     * In caso contrario restituisce FALSE.
     */
    bool Automaton::connectStates(State *from, State *to, string label) {
    	if (this->hasState(from) && this->hasState(to)) {
    		from->connectChild(label, to);
    		return true;
    	} else {
    		return false;
    	}
    }

    /**
	 * Inserisce una transizione fra i due stati marcata con la label passata come parametro.
	 * Il metodo funziona solamente se entrambi gli stati sono appartenenti all'automa, e in
     * tal caso restituisce TRUE.
     * In caso contrario restituisce FALSE.
	 */
    bool Automaton::connectStates(string from, string to, string label) {
    	return this->connectStates(getState(from), getState(to), label);
    }

    /**
     * Operatore di uguaglianza per automi.
     */
    bool Automaton::operator==(Automaton& other) {
    	DEBUG_MARK_PHASE("Confronto di due automi") {

    	// Se gli stati iniziali non sono uguali, certamente i due automi non sono uguali
        if (*m_initial_state != *other.m_initial_state) {
        	DEBUG_LOG("Lo stato %s è diverso da %s", m_initial_state->getName().c_str(), other.m_initial_state->getName().c_str());
        	return false;
        }

        // Se gli automi non hanno la stessa dimensione (= numero di stati), allora non sono certamente uguali
        if (m_states.size() != other.m_states.size()) {
        	DEBUG_LOG("Il primo automa ha %lu stati, il secondo ne ha %lu", m_states.size(), other.m_states.size());
        	return false;
        }

        // Effettuo il confronto stato per stato dei due automi.
        // Per ciascuno stato del primo automa, verifico che esista anche nell'altro.
        // Non c'è bisogno di fare il processo inverso, poiché il numero di stati è uguale.
        for (auto state : m_states) {

        	// Cerco uno stato con lo stesso nome nell'altro automa
        	State* sakename_state;
        	if ((sakename_state = other.getState(state->getName())) != NULL) {
        		DEBUG_LOG("In entrambi gli automi esiste lo stato \"%s\"", state->getName().c_str());

        		// Verifico che abbia le stesse transizioni a stati con lo stesso nome (!)
        		if (!state->hasSameTransitionsNamesOf(sakename_state)) {
        			DEBUG_LOG("Tuttavia, gli stati non hanno le stesse transizioni");
        			return false;
        		}

        	} else {
        		DEBUG_LOG("Nel secondo automa non è stato trovato uno stato \"%s\" contenuto invece nel primo automa", state->getName().c_str());
        		return false;
        	}
        }

    	}
    	DEBUG_LOG("I due automi analizzati sono risultati essere congruenti");
        return true;
    }

} /* namespace quicksc */
