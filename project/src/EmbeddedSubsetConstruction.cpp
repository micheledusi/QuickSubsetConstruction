/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * embedded_subset_construction.hpp
 *
 * 
 * This file implements the EmbeddedSubsetConstruction class.
 * The EmbeddedSubsetConstruction class is a subclass of the DeterminizationAlgorithm class, used for a previous study on the determinization of finite automata.
 * This is <b>not used</b> in the current version of the Quick Subset Construction algorithm.
 * 
 * NOTE: This represents an older module of the project. Do not refer to this.
 * It is kept here for historical reasons.
 */

#include "EmbeddedSubsetConstruction.hpp"

#include <algorithm>

#include "AutomataDrawer.hpp"
//#define DEBUG_MODE
#include "Debug.hpp"
#include "Properties.hpp"

#define REMOVING_LABEL "~"

namespace quicksc {

	/**
	 * Costruttore basato sulle configurazioni.
	 * Imposta ogni campo ad un valore nullo. Per eseguire l'algoritmo
	 * è infatti necessario chiamare il metodo "loadInputs".
	 * Imposta il nome della classe e l'abbreviazione dell'algoritmo, chiamando il costruttore padre.
	 */
	EmbeddedSubsetConstruction::EmbeddedSubsetConstruction(Configurations* configurations)
	: DeterminizationAlgorithm(ESC_ABBR, ESC_NAME) {
		// Memorizzazione delle configurationi desiderate
		this->m_active_automaton_pruning = configurations->valueOf<bool>(ActiveAutomatonPruning);
		this->m_active_distance_check_in_translation = configurations->valueOf<bool>(ActiveDistanceCheckInTranslation);
		this->m_active_removing_label = configurations->valueOf<bool>(ActiveRemovingLabel);

		this->m_nfa = NULL;
		this->m_dfa = NULL;
		this->m_singularities = NULL;
	}

	/**
	 * Distruttore.
	 * Elimina gli elementi utilizzati internamente all'algoritmo.
	 * Non elimina gli input (poiché generati all'esterno) e nemmeno il risultato
	 * finale (poiché ancora usato all'esterno).
	 */
	EmbeddedSubsetConstruction::~EmbeddedSubsetConstruction() {
		if (this->m_singularities != NULL) {
			delete this->m_singularities;
		}
	}

	/**
	 * Metodo che pulisce tutte le variabili interne, usate da precedenti esecuzioni dell'algoritmo.
	 * Viene richiamato all'inizio di runAutomatonTranslation (che inizia la risoluzione di un
	 * problema di traduzione) e di runAutomatonCheckup (che inizia la risoluzione di un problema
	 * di determinizzazione).
	 */
	void EmbeddedSubsetConstruction::cleanInternalStatus() {
		// Rimozione degli eventuali oggetti dell'esecuzione precedente
		if (this->m_singularities != NULL) {
			delete this->m_singularities;
		}
		if (this->m_nfa) {
			delete this->m_nfa;
		}
		// Nota: non cancello il risultato DFA poiché potrebbe essere ancora utilizzato da metodi esterni

		this->m_singularities = NULL;
		this->m_nfa = NULL;
		this->m_dfa = NULL;
	}

	/**
	 * Esegue l'algoritmo ESC sull'automa passato come parametro.
	 * Restituisce l'automa determinizzato.
	 */
	Automaton* EmbeddedSubsetConstruction::run(Automaton* nfa) {
		this->runAutomatonCheckup(nfa);
		this->runSingularityProcessing();
		return this->m_dfa;
	}

	/**
	 * Metodo che implementa la fase iniziale dell'algoritmo di costruzione.
	 * Esamina l'automa NON deterministico ed identifica i punti di non determinismo.
	 * Genera un automa isomorfo DFA che verrà utilizzato durante la fase di determinizzazione (Singularity Processing).
	 *
	 * INPUT:
	 * @param automaton L'automa da determinizzare.
	 */
	void EmbeddedSubsetConstruction::runAutomatonCheckup(Automaton* automaton) {
		this->cleanInternalStatus();
		// Acquisizione degli input
		DEBUG_ASSERT_NOT_NULL(automaton);
		this->m_nfa = automaton;

		// Istanziazione degli oggetti ausiliari
		this->m_singularities = new SingularityList();
		this->m_dfa = new Automaton();
		// NOTA: "original_dfa" e "translation" non vengono utilizzati per i problemi di determinizzazione.

		// Variabili locali ausiliarie
		map<State*, ConstructedState*> states_map = map<State*, ConstructedState*>();
			/* Poiché è necessario generare un automa isomorfo a quello originale, questa mappa
			 * mantiene la corrispondenza fra gli stati dell'NFA con quelli del DFA.
			 */

		// Iterazione su tutti gli stati dell'automa in input per creare gli stati corrispondenti
		for (State* state : this->m_nfa->getStatesVector()) {

			// Creo uno stato copia nel DFA
			Extension extension;
			extension.insert(state);
			ConstructedState* translated_dfa_state = new ConstructedState(extension);
			// Lo aggiungo al DFA
			this->m_dfa->addState(translated_dfa_state);

			// Associo allo stato originale il nuovo stato del DFA, in modo da poterlo ritrovare facilmente
			states_map[state] = translated_dfa_state;

		}

		// ^ ^ ^
		// Termino il primo ciclo su tutti gli stati dell'automa, in modo da procedere
		// solamente quando le associazioni fra gli stati sono complete

		// Iterazione su tutti gli stati dell'automa in input per copiare le transizioni
		for (State* state : this->m_nfa->getStatesVector()) {

			// Viene recuperato lo stato creato in precedenza, associato allo stato dell'automa originale
			ConstructedState* translated_dfa_state = states_map[state];

			// Iterazione su tutte le transizioni uscenti dallo stato dell'automa
			for (auto &pair : state->getExitingTransitions()) {

				// Label corrente
				string current_label = pair.first;

				// Distinguo due casi, basandomi sulla label dell'automa NFA di riferimento
				if (current_label == EPSILON) {
					// caso EPSILON-TRANSIZIONE

					// Per tutti gli stati figli raggiunti da transizioni marcate con la label originaria
					for (State* child : pair.second) {

						// Escludo il caso con epsilon-transizione ad anello
						if (child == state) {
							continue;
						}

						// Inserisco la transizione con label che segnala la RIMOZIONE nell'automa DETERMINISTICO D'
						translated_dfa_state->connectChild(REMOVING_LABEL, states_map[child]);
						// Inserisco il nuovo singularity nella lista, che servirà per rimuovere la transizione.
						this->addSingularityToList(translated_dfa_state, REMOVING_LABEL);

						// Se lo stato corrente è lo stato iniziale
						if (this->m_nfa->isInitial(state)) {
							// Aggiungo il SINGULARITY con epsilon transizione, che sarà il primo ad essere processato
							this->addSingularityToList(translated_dfa_state, EPSILON);
						}
						// Altrimenti, se lo stato corrente NON è lo stato iniziale, considero
						// tutte le transizioni entranti che non sono marcate da epsilon
						else {
							unsigned int current_distance = state->getDistance();
							for (auto &parent_pair : state->getIncomingTransitionsRef()) {
								string parent_label = parent_pair.first;
								// Se le transizioni sono marcate da epsilon, non le considero
								if (parent_label != EPSILON) {

									// Altrimenti, per ciascuno stato genitore con distanza minore o uguale
									// (ma solo se le impostazioni prevedono l'uso del controllo sulla distanza)
									for (State* parent : parent_pair.second) {
										if (!(this->m_active_distance_check_in_translation)
												|| parent->getDistance() <= current_distance) {

											// Aggiungo il singularity (stato_genitore, label)
											this->addSingularityToList(states_map[parent], parent_label);
										}
									}
								}
							}
						}
					}

				}
				else {
					// caso TRANSIZIONE NORMALE

					// Per tutti gli stati figli raggiunti da transizioni marcate con la label originaria
					for (State* child : pair.second) {
						// Inserisco la transizione tradotta nell'automa DETERMINISTICO D'
						translated_dfa_state->connectChild(current_label, states_map[child]);
					}

					// Verifico i punti di non determinismo: se gli stati raggiunti dalle transizioni marcate
					// con quest'etichetta sono più di uno, allora aggiungo una singolarità alla lista.
					if (pair.second.size() > 1) {
						this->addSingularityToList(translated_dfa_state, current_label);
					}
				}

			}

		}

		// Marco gli stati iniziali
		this->m_dfa->setInitialState(states_map[this->m_nfa->getInitialState()]);
	}

	/**
	 * Fornisce un'implementazione della seconda fase dell'algoritmo "Embedded Subset Construction"
	 * per la traduzione di automi (più specificamente, DFA).
	 */
	void EmbeddedSubsetConstruction::runSingularityProcessing() {
		// Finché la coda dei singularity non si svuota
		while (!this->m_singularities->empty()) {

			DEBUG_MARK_PHASE( "Nuova iterazione per un nuovo singularity" ) {

//			DEBUG_LOG( "Stampa dell'automa finale FINO A QUI:" );
//			IF_DEBUG_ACTIVE( DFADrawer drawer = DFADrawer(translated_dfa) );
//			IF_DEBUG_ACTIVE( std::cout << drawer.asString() << std::endl );

			DEBUG_LOG( "Lista delle singolarità attuale:");
			IF_DEBUG_ACTIVE( this->m_singularities->printSingularities() );

			// Estrazione del primo elemento della coda
			Singularity* current_singularity = this->m_singularities->pop();
			DEBUG_LOG( "Estrazione del Singularity corrente: %s", current_singularity->toString().c_str());

			// Preparazione dei riferimenti allo stato e alla label
			ConstructedState* current_dfa_state = current_singularity->getState();
			string current_label = current_singularity->getLabel();

			// Verifico se si tratta del singularity iniziale, l'unico con la label "EPSILON"
			// (In tal caso, non convien proseguire con il ciclo)
			if (current_label == EPSILON && this->m_dfa->isInitial(current_dfa_state)) {											/* RULE 0 */
				DEBUG_LOG( "RULE 0" );
				// Computazione della epsilon-chiusura
				Extension epsilon_closure = ConstructedState::computeEpsilonClosure(current_dfa_state->getExtension());
				// Procedura "Extension Update" sullo stato inziale e sulla sua epsilon-chiusura
				this->runExtensionUpdate(current_dfa_state, epsilon_closure);

				continue;
			}

			// Transizioni dello stato corrente
			map<string, set<State*>> current_exiting_transitions = current_dfa_state->getExitingTransitions();

			// Impostazione della front distance e della l-closure
			unsigned int front_distance = current_dfa_state->getDistance();

			DEBUG_LOG("Front distance = %u", front_distance);

			Extension l_closure = current_dfa_state->computeLClosureOfExtension(current_label); // Nell'algoritmo è rappresentata con un N in grassetto.
			string l_closure_name = ConstructedState::createNameFromExtension(l_closure);
			DEBUG_LOG("|N| = %s", l_closure_name.c_str());

			// Se le impostazioni lo prevedono, verifico se l'estensione è vuota
			if (this->m_active_automaton_pruning && l_closure.empty()) {
				DEBUG_LOG( "RULE 1" );																								/* RULE 1 */
				DEBUG_MARK_PHASE("Automaton pruning sul singularity %s", current_singularity->toString().c_str()) {
					this->runAutomatonPruning(current_singularity);
				}
			}
			// Se dallo stato corrente NON escono transizioni marcate dalla label corrente
			else if (current_exiting_transitions[current_label].empty()) {

				// Se esiste uno stato nel DFA con la stessa estensione
				if (this->m_dfa->hasState(l_closure_name)) { 																	/* RULE 2 */
					DEBUG_LOG( "RULE 2" );

					// Aggiunta della transizione dallo stato corrente a quello appena trovato
					State* child = this->m_dfa->getState(l_closure_name);
					current_dfa_state->connectChild(current_label, child);
					DEBUG_LOG("Creazione della transizione %s --(%s)--> %s",
							current_dfa_state->getName().c_str(), current_label.c_str(), child->getName().c_str());

					this->runDistanceRelocation(child, front_distance + 1);

				}
				// Se nel DFA non c'è nessuno stato con l'estensione prevista
				else { 																												/* RULE 3 */
					DEBUG_LOG( "RULE 3" );

					// Creazione di un nuovo stato State apposito e collegamento da quello corrente
					ConstructedState* new_state = new ConstructedState(l_closure);
					this->m_dfa->addState(new_state);
					current_dfa_state->connectChild(current_label, new_state);
					new_state->setDistance(front_distance + 1);

					// Per ogni transizione uscente dall'estensione, viene creato e aggiunto alla lista un nuovo Singularity
					// Nota: si sta prendendo a riferimento l'NFA associato
					for (string label : new_state->getLabelsExitingFromExtension()) {
						if (label != EPSILON) {
							this->addSingularityToList(new_state, label);
						}
					}

				}

			}
			// Se invece dallo stato corrente escono transizioni marcate dalla label corrente
			else {

				// Per tutte le transizioni marcate dalla label corrente che NON arrivano
				// in uno stato con estensione pari alla l-closure
				for (State* child_ : current_exiting_transitions[current_label]) {
					ConstructedState* child = (ConstructedState*) child_;
					DEBUG_LOG("Considero la transizione:  %s --(%s)--> %s", current_dfa_state->getName().c_str(), current_label.c_str(), child->getName().c_str());

					// Escludo gli stati con estensione diversa da |N|
					if (child->getName() == l_closure_name) {
						continue;
					}

					// Flag per la gestione delle condizioni
					bool child_is_initial = (this->m_dfa->isInitial(child));

					// Parametri per la gestione delle condizioni
					current_dfa_state->setDistance(front_distance + 1);
					int child_minimum_parents_distance = child->getMinimumParentsDistance();
					current_dfa_state->setDistance(front_distance);
							/* Nota: l'operazione di settaggio della distanza viene fatta per escludere lo State corrente dalla lista
							 * di genitori con la distanza minima durante il calcolo. Questo vincolo è necessario nella verifica delle
							 * condizioni per la rule 5 (poco sotto), dove la minima distanza NON deve considerare lo stato corrente. */

					// Se lo stato raggiunto NON è lo stato iniziale, e contemporaneamente
					// non esistono altre transizioni entranti diverse da quella che arriva dal nodo corrente
					// (che sappiamo esistere per certo, per come è stato trovato lo stato child)
					if (!child_is_initial && child->getIncomingTransitionsCount() == 1) {												/* RULE 4 */
						DEBUG_LOG( "RULE 4" );

						DEBUG_MARK_PHASE( "Extension Update" ) {
						// Aggiornamento dell'estensione
						this->runExtensionUpdate(child, l_closure);
						}
					}

					// Se lo stato raggiunto (child) è lo stato iniziale, oppure se possiede una transizione entrante
					// da uno stato con distanza inferiore o pari alla front distance, che non sia lo stato corrente
					else if (child_is_initial || child_minimum_parents_distance <= front_distance) {

//						// Calcolo del nome che avrebbe uno stato con estensione pari alla l-closure // CREDO SIA RIPETUTO
//						string l_closure_name = ConstructedState::createNameFromExtension(l_closure);

						// Se esiste uno stato nel DFA con la stessa estensione
						if (this->m_dfa->hasState(l_closure_name)) { 																/* RULE 5 */
							DEBUG_LOG( "RULE 5" );

							// Ridirezione della transizione dallo stato corrente a quello appena trovato
							State* old_child = this->m_dfa->getState(l_closure_name);
							current_dfa_state->connectChild(current_label, old_child);
							current_dfa_state->disconnectChild(current_label, child);
							DEBUG_MARK_PHASE("Distance Relocation su %s, distanza %ul", old_child->getName().c_str(), (front_distance + 1)) {
								this->runDistanceRelocation(old_child, front_distance + 1);
							}

						}
						// Se nel DFA non c'è nessuno stato con l'estensione prevista
						else { 																											/* RULE 6 */
							DEBUG_LOG( "RULE 6" );

							// Creazione di un nuovo stato State apposito e collegamento da quello corrente
							ConstructedState* new_state = new ConstructedState(l_closure);
							this->m_dfa->addState(new_state);
							current_dfa_state->connectChild(current_label, new_state);
							current_dfa_state->disconnectChild(current_label, child);
							new_state->setDistance(front_distance + 1);

							DEBUG_MARK_PHASE( "Aggiunta di tutte le labels" )
							// Per ogni transizione uscente dall'estensione, viene creato e aggiunto alla lista un nuovo Singularity
							// Nota: si sta prendendo a riferimento l'NFA associato
							for (string label : new_state->getLabelsExitingFromExtension()) {
								if (label != EPSILON) {
									this->addSingularityToList(new_state, label);
								}
							}

						}

					}

					// Altrimenti, se non si verificano le condizioni precedenti
					else {																												/* RULE 7 */
						DEBUG_LOG( "RULE 7" );

						set<std::pair<ConstructedState*, string>> transitions_to_remove = set<std::pair<ConstructedState*, string>>();

						// Per tutte le transizioni ENTRANTI nel figlio
						for (auto &pair : child->getIncomingTransitionsRef()) {
//						for (auto pair_iterator = child->getIncomingTransitions().begin();
//								pair_iterator != child->getIncomingTransitions().end();
//								pair_iterator++) {
//							auto &pair = *pair_iterator;

//							for (State* parent_ : pair.second) {
							for (auto trans_iterator = pair.second.begin(); trans_iterator != pair.second.end(); trans_iterator++) {
								ConstructedState* parent = (ConstructedState*) *trans_iterator;

								DEBUG_ASSERT_NOT_NULL( parent );

								DEBUG_LOG("Sto considerando la transizione :  %s --(%s)--> %s", parent->getName().c_str(), pair.first.c_str(), child->getName().c_str());

								// Escludo la transizione corrente
								if (parent == current_dfa_state && pair.first == current_label) {
									DEBUG_LOG("Questa è la transizione corrente, non va considerata.");
									continue;
								}

								// Preparazione delle informazioni sullo stato genitore
								Extension parent_x_closure = parent->computeLClosureOfExtension(pair.first);
								string x_closure_name = ConstructedState::createNameFromExtension(parent_x_closure);

								DEBUG_LOG("Confronto le due estensioni: %s VS %s", l_closure_name.c_str(), x_closure_name.c_str());

								// Se lo stato genitore ha un'estensione differente dallo stato corrente
								if (x_closure_name != l_closure_name) {

									DEBUG_LOG("Le due estensioni sono differenti!");
									DEBUG_LOG("Al termine, rimuoverò la transizione :  %s --(%s)--> %s", parent->getName().c_str(), pair.first.c_str(), child->getName().c_str());

									// Aggiungo lo stato "parent" alla lista dei nodi da eliminare
									// NOTA: Non è possibile eliminarlo QUI perché creerebbe problemi al ciclo
									auto t = std::pair<ConstructedState*, string>(parent, pair.first);
									transitions_to_remove.insert(t);

								} else {
									DEBUG_LOG("Le due estensioni sono uguali, non rimuovo nulla");
								}

							}
						}

						for (auto &pair : transitions_to_remove) {
							// Rimozione della transizione e aggiunta di un nuovo Singularity
							/*
							 * pair.first = nodo genitore
							 * pair.second = label
							 */
							pair.first->disconnectChild(pair.second, child);

							DEBUG_LOG("Se non presente, aggiungo il SINGULARITY : (%s, %s)", pair.first->getName().c_str(), pair.second.c_str());
							this->addSingularityToList(pair.first, pair.second);
						}

						DEBUG_MARK_PHASE( "Extension Update" )
						this->runExtensionUpdate(child, l_closure);
					}
				}
			}

			DEBUG_LOG("Arrivato al termine dell'iterazione per lo stato %s", current_dfa_state->getName().c_str() ); }
		}

		// Fase finale: eliminazione dello stato con estensione vuota

		// Verifico se è disattivata l'opzione di Automaton Pruning (che evita la creazione di stati vuoti)
		// e contemporaneamente verifico che siano presenti epsilon-transizioni
		if (!(this->m_active_automaton_pruning)) {
			State* empty_state = this->m_dfa->getState(EMPTY_EXTENSION_NAME);
			if (empty_state != NULL) {
				// Se effettivamente esiste uno stato vuoto, viene eliminato
				this->m_dfa->removeState(empty_state);
				DEBUG_LOG("Eliminazione dello stato vuoto completata");
				auto removed_states = this->m_dfa->removeUnreachableStates();
				DEBUG_LOG("Ho eliminato %lu stati irraggiungibili", removed_states.size());
			}
		}
	}

	/**
	 * Metodo privato.
	 * Fornisce un'implementazione della procedura "Distance Relocation".
	 * Modifica la distanza di una sequenza di nodi secondo i valori passati come argomento. La modifica
	 * viene poi propagata sui figli finché la nuova distanza risulta migliore. La propagazione avviene
	 * in maniera "width-first".
	 */
	void EmbeddedSubsetConstruction::runDistanceRelocation(list<pair<State*, int>> relocation_sequence) {
		while (!relocation_sequence.empty()) {
			auto current = relocation_sequence.front();
			relocation_sequence.pop_front();
			State* current_state = current.first;

			DEBUG_LOG("Esecuzione di \"Distance Relocation\" sullo stato %s", current_state->getName().c_str());

			// Se la distanza "nuova" è inferiore
			if (current_state->getDistance() > current.second) {
				DEBUG_LOG("La distanza è stata effettivamente ridotta da %u a %u", current_state->getDistance(), current.second);
				current_state->setDistance(current.second);

				// Propago la modifica ai figli
				for (auto &trans : current_state->getExitingTransitionsRef()) {
					for (State* child : trans.second) {

						// Aggiungo il figlio in coda
						relocation_sequence.push_back(pair<State*, int>(child, current.second + 1));
					}
				}
			}
		}
	}

	/**
	 * Metodo privato.
	 * Wrapper per la funzione "runDistanceRelocation" che richiede in ingresso una lista di coppie
	 * (State*, int). Poiché più di una volta, all'interno dell'algoritmo "Singularity Processing", viene
	 * richiamata la procedura "Distance Relocation" con un singolo argomento, questo metodo fornisce
	 * un'utile interfaccia per semplificare la costruzione dei parametri della chiamata.
	 */
	void EmbeddedSubsetConstruction::runDistanceRelocation(State* state, int new_distance) {
		pair<State*, int> new_pair(state, new_distance);
		list<pair<State*, int>> list;
		list.push_back(new_pair);
		this->runDistanceRelocation(list);
	}

	/**
	 * Metodo privato.
	 * Fornisce un'implementazione per la procedura "Extension Update", che modifica l'estensione di uno
	 * stato DFA aggiungendo eventuali stati NFA non presenti.
	 */
	void EmbeddedSubsetConstruction::runExtensionUpdate(ConstructedState* d_state, Extension& new_extension) {
		// Computazione degli stati aggiuntivi dell'update
		Extension difference_states_1 = ConstructedState::subtractExtensions(new_extension, d_state->getExtension());
		Extension difference_states_2 = ConstructedState::subtractExtensions(d_state->getExtension(), new_extension);

		IF_DEBUG_ACTIVE( int size = this->m_dfa->size(); )
		DEBUG_LOG( "Dimensione attuale dell'automa: %d", size );

		// Aggiornamento delle transizioni aggiuntive
		// Per tutte le transizioni uscenti dagli stati dell'estensione che non sono contenuti già nella vecchia estensione
		// Nota: In teoria si dovrebbero unire i due insiemi, ma scorrendo su entrambi separatamente è più efficiente.
		for (State* nfa_state : difference_states_1) {
			for (auto &trans : nfa_state->getExitingTransitionsRef()) {
				string label = trans.first;
				if (label != EPSILON) {
					DEBUG_LOG("Data sull'automa N la transizione: %s --(%s)-->", nfa_state->getName().c_str(), label.c_str());
					this->addSingularityToList(d_state, label);
				}
			}
		}
		for (State* nfa_state : difference_states_2) {
			for (auto &trans : nfa_state->getExitingTransitionsRef()) {
				string label = trans.first;
				if (label != EPSILON) {
					DEBUG_LOG("Data sull'automa N la transizione: %s --(%s)-->", nfa_state->getName().c_str(), label.c_str());
					this->addSingularityToList(d_state, label);
				}
			}
		}

		DEBUG_LOG("Estensione prima dell'aggiornamento: %s", ConstructedState::createNameFromExtension(d_state->getExtension()).c_str());
		// Aggiornamento dell'estensione dello stato DFA
		d_state->replaceExtensionWith(new_extension);
		DEBUG_LOG("Estensione dopo l'aggiornamento: %s", ConstructedState::createNameFromExtension(d_state->getExtension()).c_str());

		// Verifica dell'esistenza di un secondo stato nel DFA che abbia estensione uguale a "new_extension"
		string new_extension_name = ConstructedState::createNameFromExtension(new_extension);

		DEBUG_LOG("Verifico se esiste un altro stato in D con estensione pari a : %s", new_extension_name.c_str());

		// Estrazione di tutti gli stati con il nome previsto
		vector<State*> namesake_states = this->m_dfa->getStatesByName(new_extension_name);

		// Controllo se esiste più di uno stato con la medesima estensione
		if (namesake_states.size() > 1) {
			DEBUG_LOG("E' stato trovato più di uno stato con la stessa estensione \"%s\"", new_extension_name.c_str());

			ConstructedState* min_dist_state;
			ConstructedState* max_dist_state;

			// Identificazione dello stato con distanza minore / maggiore
			if (namesake_states[0]->getDistance() < namesake_states[1]->getDistance()) {
				min_dist_state = (ConstructedState*) namesake_states[0];
				max_dist_state = (ConstructedState*) namesake_states[1];
			} else {
				min_dist_state = (ConstructedState*) namesake_states[1];
				max_dist_state = (ConstructedState*) namesake_states[0];
			}

			DEBUG_ASSERT_TRUE( min_dist_state->getDistance() <= max_dist_state->getDistance() );

			// Sotto-procedura di sostituzione dello stato con distanza massima con quello con distanza minima:

			// Re-direzione di tutte le transizioni dello stato con distanza massima su quello con distanza minima
			// (Le transizioni duplicate non vengono copiate)
			DEBUG_MARK_PHASE("Copia delle transizioni") {
//			std::cout << "MIN:\n" << min_dist_state->toString() << std::endl;
//			std::cout << "MAX:\n" << max_dist_state->toString() << std::endl;
			min_dist_state->copyAllTransitionsOf(max_dist_state);
//			std::cout << "MIN:\n" << min_dist_state->toString() << std::endl;
//			std::cout << "MAX:\n" << max_dist_state->toString() << std::endl;
			}

			// Rimozione dello stato dall'automa DFA
			bool removed = this->m_dfa->removeState(max_dist_state);		// Rimuove il riferimento dello stato
			DEBUG_ASSERT_TRUE( removed );

			// All'interno della lista di singolarità, elimino ogni occorrenza allo stato con distanza massima,
			// salvando tuttavia le label dei singularity che erano presenti.
			set<string> max_dist_singularities_labels = this->m_singularities->removeSingularitiesOfState(max_dist_state);

			// Per tutte le label salvate, se il relativo singularity legato allo stato con distanza minima NON è presente, lo aggiungo
			for (string singularity_label : max_dist_singularities_labels) {
				if (singularity_label != EPSILON) {
					this->addSingularityToList(min_dist_state, singularity_label);
				}
			}

			// Procedura "Distance Relocation" su tutti i figli dello stato con dist.min, poiché i figli acquisiti dallo stato
			// con dist.max. devono essere modificati
			list<pair<State*, int>> to_be_relocated_list;
			for (auto &trans : min_dist_state->getExitingTransitionsRef()) {
				for (State* child : trans.second) {
					DEBUG_LOG("Aggiungo alla lista di cui fare la distance_relocation: (%s, %u)", child->getName().c_str(), min_dist_state->getDistance() + 1);
					to_be_relocated_list.push_back(pair<State*, int>(child, min_dist_state->getDistance() + 1));
				}
			}
			this->runDistanceRelocation(to_be_relocated_list);
			this->m_singularities->sort();

		}
	}

	/**
	 * Aggiunge una singularity alla lista, occupandosi della creazione e del fatto che possano esserci duplicati.
	 * Eventualmente, segnala anche gli errori.
	 */
	void EmbeddedSubsetConstruction::addSingularityToList(ConstructedState* singularity_state, string singularity_label) {
		Singularity* new_singularity = new Singularity(singularity_state, singularity_label);
		// Provo ad inserire il singularity nella lista
		if (this->m_singularities->insert(new_singularity)) {
			// Caso in cui non sono presenti singularity uguali
			DEBUG_LOG("Aggiungo alla lista il Singularity %s" , new_singularity->toString().c_str());
		} else {
			// Caso in cui esistono singularity duplicati
			DEBUG_LOG("Il Singularity %s è già presente nella lista, pertanto non è stato aggiunto" , new_singularity->toString().c_str());
			delete new_singularity;
		}
	}

	/**
	 * Metodo che si occupa della gestione del caso "estensione vuota" durante la procedura "Singularity Processing".
	 * In pratica, rimuove tutti e soli gli stati non più raggiungibili, poiché connessi solamente tramite lo stato
	 * e la transizione marcate con una label specifica.
	 *
	 * @param singularity Il singularity corrente che ha generato un'estensione |N| vuota; contiene lo stato da cui partire e la label interessata,
	 */
	void EmbeddedSubsetConstruction::runAutomatonPruning(Singularity* singularity) {
		// Lista di (potenziali) candidati, ossia coloro che verranno eliminati
		list<ConstructedState*> candidates = list<ConstructedState*>();

		// Lista di (potenziali) Entry Points, ossia stati raggiunti dall'esterno
		list<ConstructedState*> entry_points = list<ConstructedState*>();

		// Lista degli stati effettivamente raggiunti dall'esterno
		list<ConstructedState*> reached_states = list<ConstructedState*>();

		ConstructedState* starting_state = singularity->getState();
		string starting_label = singularity->getLabel();
		auto starting_state_exiting_transitions = starting_state->getExitingTransitionsRef();

		DEBUG_MARK_PHASE("Ciclo (1) - Primi figli dell'estensione vuota") {
		// Per tutte le transizioni uscenti dallo stato iniziale che generano l'estensione vuota
		for (State* _empty_child : starting_state_exiting_transitions[starting_label]) {
			ConstructedState* empty_child = (ConstructedState*) _empty_child;
			DEBUG_LOG("Aggiungo alla lista dei candidati lo stato %s", empty_child->getName().c_str());
			candidates.push_back(empty_child);
			// La marcatura indica che lo stato fa parte dei candidati
			// In questo modo è possibile sapere subito se uno stato è nella lista, senza doverlo cercare.
			empty_child->setMarked(true);
			// Infine, viene rimossa la transizione che genera l'estensione vuota
			DEBUG_LOG("Viene rimossa la transizione %s --(%s)--> %s", starting_state->getName().c_str(), starting_label.c_str(), empty_child->getName().c_str());
			starting_state->disconnectChild(starting_label, empty_child);
		}
		}

		DEBUG_MARK_PHASE("Ciclo (2) - Lista dei candidati") {
		// Scorro sulla lista di candidati
		// Si noti che questo ciclo provoca l'aggiunta di stati (sempre in coda) alla lista su cui si itera
		for (auto it = candidates.begin(); it != candidates.end(); /* L'aggiornamento dell'iteratore è effettuato in coda */) {
			// Dereferenzio l'iteratore
			ConstructedState* current = *it;
			DEBUG_LOG("\tConsidero il possibile candidato all'eliminazione %s", current->getName().c_str());
			current->setMarked(true);

			/* Verifico se esista almeno una transizione che:
			 * 1) Sia entrante nello stato corrente
			 * 2) Provenga da uno stato NON candidato
			 * 3) Provenga da uno stato con distanza strettamente minore della distanza dello stato corrente OPPURE sia lo stato iniziale.
			 *
			 * Se si verificano tutte queste tre condizioni, lo stato corrente è raggiunto "dall'esterno".
			 * Pertanto NON è un candidato, e può essere rimosso dalla lista
			 *
			 * Altrimenti, lo stato è effettivamente un candidato; tutti i suoi figli NON candidati vengono
			 * aggiunti alla lista dei candidati.
			 */
			bool is_reachable = false;
			bool is_possible_entry_point = false;
			// Verifico se è lo stato iniziale
			if (this->m_dfa->isInitial(current)) {
				is_reachable = true;
			}
			else {
				// Scorro su tutte le transizioni entranti nello stato corrente [condizione 1]
				for (auto &pair : current->getIncomingTransitionsRef()) {
					for (State* _parent : pair.second) {
						ConstructedState* parent = (ConstructedState*) _parent;
						DEBUG_LOG("\t\tConsidero la transizione %s --(%s)--> %s", parent->getName().c_str(), pair.first.c_str(), current->getName().c_str());

						// Considero solo le transizioni da stati NON candidati [condizione 2]
						if (!parent->isMarked()) {
							DEBUG_LOG("\t\t\tIl nodo genitore %s non è marcato", parent->getName().c_str());
							// Se la transizione arriva da uno stato con distanza minore [condizione 3]
							if (parent->getDistance() < current->getDistance()) {
								is_reachable = true;
								break;
							}
							else {
								is_possible_entry_point = true;
							}
						}
					}
					// Se è raggiunto, è inutile continuare ad iterare
					if (is_reachable) {
						break;
					}
				}
			}

			// Se lo stato corrente è raggiungibile, non è candidato per l'eliminazione
			if (is_reachable) {
				DEBUG_LOG("Lo stato %s è risultato raggiungibile dall'esterno dell'insieme dei candidati, pertanto non è più marcato", current->getName().c_str());
				current->setMarked(false);
				candidates.erase(it++);
			}
			else {
				// Se è risultato un possibile entry point, viene aggiunto alla lista
				if (is_possible_entry_point) {
					DEBUG_LOG("Lo stato %s è risultato un possibile entry point", current->getName().c_str());
					entry_points.push_back(current);
				}

				DEBUG_LOG("Tutti i figli di %s non marcati sono possibili candidati:", current->getName().c_str());
				// Tutti i figli vengono aggiunti come possibili candidati
				for (auto &pair : current->getExitingTransitionsRef()) {
					for (State* _child : pair.second) {
						ConstructedState* child = (ConstructedState*) _child;
						if (!child->isMarked()) {
							DEBUG_LOG("Aggiungo alla lista dei candidati lo stato %s", child->getName().c_str());
							candidates.push_back(child);
						}
						/* Piccola nota: in questo punto della procedura, ogni figlio dello stato corrente che non è marcato
						 * ha sicuramente una distanza maggiore del padre; infatti, se avesse una distanza inferiore, per come
						 * vengono inseriti (e processati) gli stati potenziali candidati, esso sarebbe già marcato.
						 * Per questo motivo non è necessario inserire un controllo.
						 */
					}
				}

				++it;
			}

			// Termine dell'iterazione sullo stato corrente
		}
		}

		DEBUG_MARK_PHASE("Ciclo (3) - Controllo degli entry points") {
		// Controllo dei possibili entry points
		/* Iterando sulla lista dei potenziali entry points, viene verificato per ciascuno di essi
		 * che esista almeno una transizione entrante che proviene da uno stato NON candidato.
		 * Poiché gli stati candidati sono stati identificati completamente (e sono marcati), questa condizione
		 * dovrebbe essere sufficiente a decretare se uno stato è raggiungibile dall'esterno dell'insieme dei candidati.
		 */
		for (auto it = entry_points.begin(); it != entry_points.end(); ) {
			ConstructedState* entry_pt = *it;

			bool is_entry_point = false;
			for (auto &pair : entry_pt->getIncomingTransitionsRef()) {
				for (State* _parent : pair.second) {
					ConstructedState* parent = (ConstructedState*) _parent;
					if (!parent->isMarked()) {
						// E' davvero un entry point!
						DEBUG_LOG("Lo stato %s è davvero un entry point!", entry_pt->getName().c_str());
						is_entry_point = true;
						break;
					}
				}
				if (is_entry_point) {
					break;
				}
			}

			// Al termine dell'analisi delle transizioni entranti, posso decretare se lo stato è davvero raggiunto da stati esterni all'insieme di candidati
			if (!is_entry_point) {
				// Se NON è un'entry point, lo rimuovo dalla lista.
				// Questo significa che è effettivamente uno stato candidato
				entry_points.erase(it++);
				DEBUG_ASSERT_TRUE(entry_pt->isMarked());
			}
			else {
				// Altrimenti, se E' un entry point, lo rimuovo dalla lista dei candidati
//				candidates.remove(entry_pt);
					/*
					 * Nota: la precedente operazione potrebbe risultare lunga, pertanto si potrebbe decidere
					 * di spostare il "carico" di ricordarsi quali stati sono da eliminare sulla marcatura.
					 * Al termine, si scorre sui candidati e si verifica quali sono marcati e quali no, per stabilire
					 * quali eliminare.
					 */
				entry_pt->setMarked(false);
				DEBUG_ASSERT_FALSE(entry_pt->isMarked());//
				// Inoltre, lo aggiungo alla lista degli stati effettivamente raggiunti
				reached_states.push_back(entry_pt);
				++it;
			}
		}
		}

		DEBUG_MARK_PHASE("Ciclo (4) - Chiusura degli stati raggiungibili") {
		/* A questo punto, ho in "reached_states" solo stati che vengono raggiunti dall'esterno dell'insieme dei candidati
		 * (gli entry points, appunto).
		 * Pertanto si cerca di computarne la chiusura, ossia tutti gli stati raggiungibili dagli entry points ma che stiano
		 * anche nell'insieme dei candidati (i.e. che abbiano la marcatura attiva).
		 */
		for (auto it = reached_states.begin(); it != reached_states.end(); it++) {
			ConstructedState* reached_state = *it;

			// In teoria, ogni stato in questa lista non dovrebbe più essere marcato
			DEBUG_ASSERT_FALSE(reached_state->isMarked());

			// Per tutti i figli MARCATI
			for (auto &pair : reached_state->getExitingTransitionsRef()) {
				for (State* _child : pair.second) {
					ConstructedState* child = (ConstructedState*) _child;

					// Se fa ancora parte dei candidati
					if (child->isMarked()) {
						// Rimuovo la marcatura
						child->setMarked(false);
						// Lo aggiungo alla lista degli stati raggiunti
						reached_states.push_back(child);
					}
				}
			}

			/* Infine, per ciascun entry point "annullo" la distanza, in modo tale che possa essere computata
			 * successivamente in maniera comoda e corretta, senza basarsi su informazioni pregresse che (con
			 * l'eliminazione degli stati) risultano errate.
			 */
			reached_state->setDistance(DEFAULT_VOID_DISTANCE);
		}
		}

		DEBUG_MARK_PHASE("Ciclo (5) - Rimozioni dei candidati da eliminare") {
		/* A questo punto in "candidates" sono presenti tutti i candidati per l'eliminazione, ma solo quelli
		 * marcati risultano effettivamente da rimuovere.
		 */
		for (ConstructedState* candidate : candidates) {
			if (candidate->isMarked()) {
				DEBUG_LOG("Rimuovo lo stato %s", candidate->getName().c_str());
				// Rimuovo lo stato dall'automa (rimuovendo anche le sue transizioni
				this->m_dfa->removeState(candidate);
				// Rimuovo lo stato dalla lista dei singularity
				this->m_singularities->removeSingularitiesOfState(candidate);
			}
		}
		}

		DEBUG_MARK_PHASE("Ciclo (6) - Ricostruzione delle distanze") {
		/* Per ciascuno degli stati rimasti, computo la distanza.
		 * Nota: l'ordine degli stati (che viene mantenuto in tutta la procedura) si basa sulla distanza; pertanto
		 * dovrebbe essere sufficiente per effettuare il calcolo correttamente.
		 */
		for (ConstructedState* entry_pt : entry_points) {
			unsigned int new_distance = entry_pt->getMinimumParentsDistance();
			DEBUG_ASSERT_FALSE(new_distance == DEFAULT_VOID_DISTANCE);
			entry_pt->initDistancesRecursively(new_distance + 1);
		}
		// Ri-ordino la lista di singolarità
		this->m_singularities->sort();
		}

	}

} /* namespace quicksc */
