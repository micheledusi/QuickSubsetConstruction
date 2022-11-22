/******************************************************************************
 * embedded_subset_construction.hpp
 *
 * Project: TranslatedAutomata
 *
 * File sorgente della classe "QuickSubsetConstruction" che implementa l'algoritmo
 * "Quick Subset Construction" per la traduzione di un automa DFA.
 * Per maggiori informazioni sul funzionamento dell'algoritmo, si veda la documentazione
 * del metodo "run".
 *
 ******************************************************************************/

#include "QuickSubsetConstruction.hpp"

#include <algorithm>

#include "AutomataDrawer.hpp"
#include "Properties.hpp"

//#define DEBUG_MODE
#include "Debug.hpp"

namespace quicksc {

	/**
	 * Costruttore vuoto.
	 * Imposta ogni campo ad un valore nullo. Per eseguire l'algoritmo
	 * è infatti necessario chiamare il metodo "loadInputs".
	 * Imposta il nome della classe e l'abbreviazione dell'algoritmo, chiamando il costruttore padre.
	 */
	QuickSubsetConstruction::QuickSubsetConstruction(Configurations* configurations)
	: DeterminizationAlgorithm(QSC_ABBR, QSC_NAME) {
		this->m_singularities = NULL;
	}

	/**
	 * Distruttore.
	 * Elimina gli elementi utilizzati internamente all'algoritmo.
	 * Non elimina gli input (poiché generati all'esterno) e nemmeno il risultato
	 * finale (poiché ancora usato all'esterno).
	 */
	QuickSubsetConstruction::~QuickSubsetConstruction() {
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
	void QuickSubsetConstruction::cleanInternalStatus() {
		// Rimozione degli eventuali oggetti dell'esecuzione precedente
		if (this->m_singularities != NULL) {
			delete this->m_singularities;
		}
		this->m_singularities = NULL;
	}

	void QuickSubsetConstruction::resetRuntimeStatsValues() {
		// Calling parent method
		DeterminizationAlgorithm::resetRuntimeStatsValues();
		// Initializing the values
		map<RuntimeStat, double> stats = this->getRuntimeStatsValuesRef();
		for (RuntimeStat stat : this->getRuntimeStatsList()) {
			stats[stat] = (double) 0;
		}
	}

	/**
	 * Definizione delle statistiche utilizzate.
	 */
	vector<RuntimeStat> QuickSubsetConstruction::getRuntimeStatsList() {
        vector<RuntimeStat> list = DeterminizationAlgorithm::getRuntimeStatsList();
		list.push_back(NUMBER_SINGULARITIES_CHECKUP);
		list.push_back(NUMBER_SINGULARITIES_SCENARIO_0);
		list.push_back(NUMBER_SINGULARITIES_SCENARIO_1);
		list.push_back(NUMBER_SINGULARITIES_SCENARIO_2);
		list.push_back(NUMBER_SINGULARITIES_TOTAL);
		list.push_back(IMPACT);
		return list;
    }

	/**
	 * Esegue l'algoritmo QSC sull'automa passato come parametro.
	 * Restituisce l'automa determinizzato.
	 */
	Automaton* QuickSubsetConstruction::run(Automaton* nfa) {
		this->cleanInternalStatus();

		// Acquisizione degli input
		DEBUG_ASSERT_NOT_NULL(nfa);

		// Disegno l'NFA originale
		IF_DEBUG_ACTIVE( AutomataDrawer drawer = AutomataDrawer(nfa) );
		IF_DEBUG_ACTIVE( std::cout << drawer.asString() << std::endl );
		DEBUG_WAIT_USER_ENTER();

		// Istanziazione degli oggetti ausiliari
		this->m_singularities = new SingularityList();
		Automaton* dfa = new Automaton();

		// Variabili locali ausiliarie
		map<State*, ConstructedState*> states_map = map<State*, ConstructedState*>(); // Poiché è necessario generare un automa isomorfo a quello originale, questa mappa mantiene la corrispondenza fra gli stati dell'NFA con quelli del DFA.


		/**************************
		  PRIMA FASE - CLONAZIONE

	 	  	Esamino l'automa NON deterministico ed identifico i punti di non determinismo.
			Genero un automa isomorfo DFA che verrà utilizzato durante la fase di ristrutturaizone.

		 **************************/



		// Iterazione su tutti gli stati dell'automa in input per creare gli stati corrispondenti
		for (State* nfa_state : nfa->getStatesVector()) {

			// Creo uno stato copia nel DFA
			Extension extension;
			extension.insert(nfa_state);
			ConstructedState* dfa_state = new ConstructedState(extension);
			// Lo aggiungo al DFA
			dfa->addState(dfa_state);
			dfa_state->setDistance(nfa_state->getDistance());

			// Associo allo stato originale il nuovo stato del DFA, in modo da poterlo ritrovare facilmente
			states_map[nfa_state] = dfa_state;

		}

		// ^ ^ ^
		// Termino il primo ciclo su tutti gli stati dell'automa, in modo da procedere
		// solamente quando le associazioni fra gli stati sono complete

		// Iterazione su tutti gli stati dell'automa in input per copiare le transizioni
		for (State* nfa_state : nfa->getStatesVector()) {
			DEBUG_LOG("Considero lo stato %s", nfa_state->getName().c_str());

			// Viene recuperato lo stato creato in precedenza, associato allo stato dell'automa originale
			ConstructedState* dfa_state = states_map[nfa_state];

			// Iterazione su tutte le transizioni uscenti dallo stato dell'automa NFA
			for (auto &pair : nfa_state->getExitingTransitions()) {

				// Label corrente
				string current_label = pair.first;

				// Valore che diventa vero appena la singolarità viene aggiunta. Evita controlli inutili
				bool added_singularity_flag = false;

				// *** SINGOLARITA' CASO 1 *** //
				// Se lo stato corrente dell'NFA è lo stato iniziale
				if (nfa->isInitial(nfa_state) && current_label == EPSILON) {
					DEBUG_LOG("L'automa necessita della eps-singolarità iniziale");
					// Aggiungo la singolarità con EPSILON transizione, che sarà la prima ad essere processata
					this->addSingularityToList(dfa_state, current_label);
					added_singularity_flag = true;
				}

				// Per tutti gli stati figli raggiunti da transizioni marcate con la label originaria
				for (State* nfa_child : pair.second) {

					// Escludo il caso con epsilon-transizione ad anello
					if (nfa_child == nfa_state && current_label == EPSILON) {
						continue;
					}

					// Inserisco (a prescindere dalla label) la transizione dell'NFA nell'automa DFA copia
					State* dfa_child = states_map[nfa_child];
					dfa_state->connectChild(current_label, dfa_child);

					// *** SINGOLARITA' CASO 2 *** //
					// Se la transizione ha una epsilon-transizione uscente dal nodo figlio, e non è una eps-transizione
					if (!added_singularity_flag && pair.first != EPSILON && nfa_child->hasExitingTransition(EPSILON)) {

						// Devo tuttavia verificare che non si tratti di una eps-transizione ad anello
						bool is_not_epsilon_ring = false;
						for (State* nfa_grandchild : nfa_child->getChildren(EPSILON)) {
							if (nfa_grandchild != nfa_child) {
								is_not_epsilon_ring = true;
								break;
							}
						}

						// Ora che sono sicuro, aggiungo la singolarità
						if (is_not_epsilon_ring) {
							this->addSingularityToList(dfa_state, current_label);
						}
					}

				} // Iterazione sui figli

				// *** SINGOLARITA' CASO 2 *** //
				// Verifico se gli stati raggiunti dalle transizioni marcate con quest'etichetta sono più di uno; in tal caso aggiungo una singolarità alla lista.
				if (!added_singularity_flag && current_label != EPSILON && pair.second.size() > 1) {
					this->addSingularityToList(dfa_state, current_label);
				}

			} // Iterazione sulle label

		} // Iterazioni sugli stati

		// Marco lo stato iniziale del DFA copia, in base allo stato iniziale dell'NFA, e calcolo le distanze
		dfa->setInitialState(states_map[nfa->getInitialState()]);

		(this->getRuntimeStatsValuesRef())[NUMBER_SINGULARITIES_CHECKUP] = this->m_singularities->size();


		/***********************************
		  SECONDA FASE - RISTRUTTURAZIONE
		 ***********************************/


		/***** SCENARIO S_0 (ZERO) *****/

		// Controllo sulla prima singolarità della lista.
		// Verifico se la prima singolarità è quella relativa allo stato iniziale con etichetta EPSILON. Per farlo è sufficiente controllare l'etichetta (l'unica possibile eps-singolarità è infatti relativa allo stato iniziale, per come viene svolto il controllo nella prima fase).
		if (!this->m_singularities->empty() && this->m_singularities->getFirstLabel() == EPSILON) {
			DEBUG_LOG(COLOR_PINK("SCENARIO 0"));
			this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_0] += 1;

			// Riga 5 (ma anche riga 15)
			// Estrazione del primo elemento della coda
			DEBUG_LOG("Estrazione della singolarità con etichetta EPSILON riferita allo stato iniziale");
			Singularity* initial_singularity = this->m_singularities->pop();

			// Preparazione dei riferimenti allo stato e alla label
			ConstructedState* initial_dfa_state = initial_singularity->getState();

			// Riga 6
			// Calcolo la epsilon-chiusura dello stato iniziale sul DFA, denotata anche |D
			Extension d0_eps_closure = ConstructedState::computeEpsilonClosure(initial_dfa_state);

			// Calcolo la epsilon-chiusura dello stato iniziale sull'NFA, denotata anche |N
			Extension n0_eps_closure = ConstructedState::computeEpsilonClosure(nfa->getInitialState());

			// Calcolo l'insieme |U di stati unsafe
			Extension unsafe_states = Extension();
			for (State* dfa_closure_state : d0_eps_closure) {
				ConstructedState* c_dfa_closure_state = static_cast<ConstructedState*> (dfa_closure_state);

				// Considero solo quelli unsafe
				if (c_dfa_closure_state->isUnsafe(initial_dfa_state, EPSILON)) {
					unsafe_states.insert(dfa_closure_state);
					// Inoltre, per velocizzare il futuro controllo, applico un marchio ad ogni stato unsafe
					// Non avrò bisogno di rimuoverlo perché ogni stato unsafe verrà eliminato.
					c_dfa_closure_state->setMarked(true);

					DEBUG_LOG("Identificato uno stato unsafe: %s", c_dfa_closure_state->getName().c_str());
				}
			}

			// Riga 7
			// Estendo l'estensione di d0 a |N
			initial_dfa_state->replaceExtensionWith(n0_eps_closure);
			DEBUG_LOG("L'estensione dello stato iniziale del DFA è stata aggiornata a: %s", initial_dfa_state->getName().c_str());

			// Rimuovo le eps-transizioni uscenti dallo stato iniziale
			for (State* eps_child : initial_dfa_state->getChildren(EPSILON)) {
				DEBUG_LOG("Disconnetto la epsilon-transizione che collega il nodo iniziale %s al figlio %s", initial_dfa_state->getName().c_str(), eps_child->getName().c_str());
				initial_dfa_state->disconnectChild(EPSILON, eps_child);
			}

			// // Creo le singolarità creatisi a causa dell'aggiornamento dell'estensione
			// // Per farlo utilizzo una mappa che conta quante volte ho una transizione uscente dall'estensione con una certa etichetta. Se ne ho più di una, alla fine dovrò creare una singolarità. Allo stesso modo creerò una singolarità se una transizione uscente da uno stato dell'estensione è "genitore" di una eps-transizione.
			// map<string, int> labels_counter = map<string, int>();
			//
			// // Scorro su tutti gli stati della eps-chiusura e estraggo le singolarità
			// for (State* nfa_state : n0_eps_closure) {
			//
			// 	// Scorro su tutte le label delle transizioni uscenti dello stato dell'estensione
			// 	for (auto &trans : nfa_state->getExitingTransitionsRef()) {
			// 		string label = trans.first;
			//
			// 		// Non considero quelle con etichetta epsilon
			// 		if (label == EPSILON) {
			// 			continue;
			// 		}
			//
			// 		// Se la mappa non contiene questa etichetta, la creo
			// 		if (!labels_counter.count(label)) {
			// 			labels_counter[label] = 0;
			// 		}
			//
			// 		// Verifico se lo stato figlio ha eps-transizioni uscenti
			// 		for (State* nfa_child : trans.second) {
			// 			if (nfa_child->hasExitingTransition(EPSILON)) {
			// 				// In caso affermativo, creo la singolarità
			// 				this->addSingularityToList(initial_dfa_state, label);
			// 				// A questo punto la singolarità già esiste, non ha senso crearne altre che comunque non verranno conteggiate. Le escludo mettendo un valore basso alla mappa. In questo modo il risultato finale non sarà positivo e non raddoppierà la singolarità.
			// 				labels_counter[label] = -99999;
			// 				// Poi non ha neanche più senso continuare con il ciclo.
			// 				break;
			// 			}
			// 		}
			//
			// 		// Infine, aggiungo il numero di transizioni con una data label alla mappa
			// 		labels_counter[label] += trans.second.size();
			// 	}
			// }
			//
			// // Alla fine controllo il numero di etichette totali uscenti dagli stati dell'estensione.
			// for (auto &pair : labels_counter) {
			// 	// Se ho più di un certo numero
			// 	if (pair.second > 1) {
			// 		this->addSingularityToList(initial_dfa_state, pair.first);
			// 	}
			// }

			// Controllo tutte le transizioni uscenti dagli stati della nuova estensione
			DEBUG_LOG("Scorro su tutti gli stati dell'estensione per creare le necessarie singolarità");
			for (State* nfa_state : n0_eps_closure) {

				DEBUG_LOG("Considero lo stato %s", nfa_state->getName().c_str());
				// Salto lo stato iniziale
				if (nfa_state == nfa->getInitialState()) {
					DEBUG_LOG("Corrisponde allo stato iniziale, quindi lo salto");
					continue;
				}

				for (auto &pair : nfa_state->getExitingTransitionsRef()) {
					if (pair.first != EPSILON) {
						this->addSingularityToList(initial_dfa_state, pair.first);
					}
				}
			}


			// Riga 8
			// Scorro su tutti gli stati UNSAFE
			for (State* unsafe : unsafe_states) {

				// Riga 8
				// Per tutte le transizioni uscenti da uno stato unsafe
				for (auto &pair : unsafe->getExitingTransitionsRef()) {
					// Controllo che l'etichetta non sia EPSILON
					if (pair.first == EPSILON) {
						continue;
					}

					for (State* unsafe_child : pair.second) {
						ConstructedState* c_unsafe_child = static_cast<ConstructedState*> (unsafe_child);

						// Verifico che il figlio NON sia nell'insieme di stati unsafe
						if (!c_unsafe_child->isMarked()) {
							// Riga 9
							// Se NON lo è, creo una transizione
							initial_dfa_state->connectChild(pair.first, unsafe_child);
							DEBUG_LOG("Creazione della transizione:  %s --(%s)--> %s", initial_dfa_state->getName().c_str(), pair.first.c_str(), unsafe_child->getName().c_str());
						}
					}
				}

				// Riga 11
				// Per tutte le transizioni entranti in uno stato unsafe
				for (auto &pair : unsafe->getIncomingTransitionsRef()) {
					// Controllo che l'etichetta non sia EPSILON
					if (pair.first == EPSILON) {
						continue;
					}

					for (State* unsafe_parent : pair.second) {
						ConstructedState* c_unsafe_parent = static_cast<ConstructedState*> (unsafe_parent);

						// Verifico che il padre NON sia nell'insieme di stati unsafe
						if (!c_unsafe_parent->isMarked()) {
							// Riga 12
							// Se NON lo è, creo una singolarità
							this->addSingularityToList(c_unsafe_parent, pair.first);
						}
					}
				}
			}

			// Riga 14
			// Rimuovo dal DFA tutti gli stati unsafe
			for (State* unsafe : unsafe_states) {
				DEBUG_LOG("Rimuovo lo stato unsafe: %s", unsafe->getName().c_str());
				dfa->removeState(unsafe);
				this->m_singularities->removeSingularitiesOfState(static_cast<ConstructedState*>(unsafe));
			}

		}


		/***** SCENARI S_1 e S_2 *****/

		DEBUG_LOG("Inizia il ciclo sulla coda di singolarità.");

		// Finché la coda delle singolarità non si svuota
		while (!this->m_singularities->empty()) {

			DEBUG_MARK_PHASE( "Nuova iterazione per una nuova singolarità" ) {

			DEBUG_LOG( "Stampa dell'automa finale FINO A QUI:" );
			IF_DEBUG_ACTIVE( AutomataDrawer drawer = AutomataDrawer(dfa) );
			IF_DEBUG_ACTIVE( std::cout << drawer.asString() << std::endl );
			DEBUG_WAIT_USER_ENTER();

			DEBUG_LOG( "Lista delle singolarità attuali (#%d):", this->m_singularities->size());
			IF_DEBUG_ACTIVE( this->m_singularities->printSingularities() );

			// Estrazione del primo elemento della coda
			Singularity* current_singularity = this->m_singularities->pop();
			DEBUG_LOG( "Estrazione della singolarità corrente: %s", current_singularity->toString().c_str());

			DEBUG_WAIT_USER_ENTER();

			// Preparazione dei riferimenti allo stato e alla label
			ConstructedState* current_singularity_state = current_singularity->getState();
			string current_singularity_label = current_singularity->getLabel();

			// Computo l'ell-closure della singolarità.
			Extension nfa_l_closure = current_singularity_state->computeLClosureOfExtension(current_singularity_label); // Nell'algoritmo è rappresentata con un N in grassetto.
			string nfa_l_closure_name = ConstructedState::createNameFromExtension(nfa_l_closure);
			DEBUG_LOG("|N| = %s", nfa_l_closure_name.c_str());


			/***** SCENARIO S_1 (UNO) *****/

			// NON esistono transizioni uscenti dallo stato che abbiano la label della singolarità
			if (!current_singularity_state->hasExitingTransition(current_singularity_label)) {
				DEBUG_LOG(COLOR_PINK("SCENARIO 1"));
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_1] += 1;

				// Controllo se esiste già lo stesso stato nell'automa
				if (dfa->hasState(nfa_l_closure_name)) {

					// Aggiunta della transizione dallo stato corrente a quello appena trovato
					State* child = dfa->getState(nfa_l_closure_name);
					current_singularity_state->connectChild(current_singularity_label, child);
					DEBUG_LOG("Creazione della transizione: %s --(%s)--> %s",
							current_singularity_state->getName().c_str(), current_singularity_label.c_str(), child->getName().c_str());

					// Sistemo la distanza (CREDO!!!)
					this->runDistanceRelocation(child, current_singularity_state->getDistance() + 1);

				}
				// Non esiste uno stato equivalente con estensione |N
				else {

					// Creazione di un nuovo stato State apposito e collegamento da quello corrente
					ConstructedState* new_state = new ConstructedState(nfa_l_closure);
					dfa->addState(new_state);
					current_singularity_state->connectChild(current_singularity_label, new_state);
					new_state->setDistance(current_singularity_state->getDistance() + 1);

					DEBUG_LOG("Creazione della transizione: %s --(%s)--> %s",
							current_singularity_state->getName().c_str(), current_singularity_label.c_str(), new_state->getName().c_str());

					// Per ogni transizione uscente dall'estensione, viene creato e aggiunto alla lista una nuova singolarità
					// Nota: si sta prendendo a riferimento l'NFA associato
					for (string label : new_state->getLabelsExitingFromExtension()) {
						if (label != EPSILON) {
							this->addSingularityToList(new_state, label);
						}
					}

				}

			}

			// Esistono transizioni uscenti dallo stato che abbiano la label <current_singularity_label> della singolarità
			else {
				// Verifico lo status di tali transizioni, per verificare se posso applicare lo scenario 2
				bool scenario_2_flag = false;
				DEBUG_LOG("Verifico le condizioni per il verificarsi dello Scenario 2");

				// Devono esistere almeno due ell-figli
				if (current_singularity_state->getChildrenRef(current_singularity_label).size() > 1) {
					DEBUG_LOG("Lo stato %s ha almeno due transizioni uscenti marcate dall'etichetta %s", current_singularity_state->getName().c_str(), current_singularity_label.c_str());
					scenario_2_flag = true;
				}
				// OPPURE
				else {
					State* current_singularity_child = current_singularity_state->getChild(current_singularity_label);
					DEBUG_LOG("Lo stato %s ha una sola transizione uscente marcata dall'etichetta %s, che arriva allo stato %s", current_singularity_state->getName().c_str(), current_singularity_label.c_str(), current_singularity_child->getName().c_str());

					// L'unico figlio esistente deve avere una eps-transizione uscente
					if (current_singularity_child->hasExitingTransition(EPSILON)) {
						DEBUG_LOG("Lo stato figlio %s ha una epsilon-transizione uscente", current_singularity_child->getName().c_str());
						scenario_2_flag = true;
					}
					// OPPURE
					else {
						DEBUG_LOG("Lo stato figlio %s non ha epsilon-transizioni uscenti", current_singularity_child->getName().c_str());
						ConstructedState* c_current_singularity_child = static_cast<ConstructedState*> (current_singularity_child);
						// L'unico figlio esistente deve avere estensione diversa da |N
						if (!c_current_singularity_child->hasExtension(nfa_l_closure)) {
							DEBUG_LOG("Lo stato figlio ha un'estensione differente dall'estensione |N corrente: %s", nfa_l_closure_name.c_str());
							scenario_2_flag = true;
						}
					}
				}


				// Se non ho le condizioni per applicare lo scenario 2, esco dal ciclo e pesco una nuova sing.
				if (!scenario_2_flag) {
					continue;
				}

				/***** SCENARIO S_2 (DUE) *****/

				DEBUG_LOG(COLOR_PINK("SCENARIO 2"));
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_2] += 1;

				Extension dfa_l_closure = current_singularity_state->computeLClosure(current_singularity_label);
				DEBUG_LOG("La ell-chiusura sul DFA (denotata |D) contiene %lu stati", dfa_l_closure.size());

				// Calcolo la lista di stati UNSAFE
				Extension unsafe_states;
				for (State* ell_child : dfa_l_closure) {
					ConstructedState* c_ell_child = static_cast<ConstructedState*> (ell_child);

					if (c_ell_child->isUnsafe(current_singularity_state, current_singularity_label)) {
						// Aggiungo lo stato alla lista di stati unsafe
						unsafe_states.insert(ell_child);
						c_ell_child->setMarked(true);
						// Provo a rimuovere fin da subito le singolarità degli stati unsafe
						this->m_singularities->removeSingularitiesOfState(static_cast<ConstructedState*>(c_ell_child));
					}
				}
				DEBUG_LOG("L'insieme di stati unsafe del DFA (denotato |U) contiene %lu stati", unsafe_states.size());

				ConstructedState* dfa_new_state = static_cast<ConstructedState*> (dfa->getState(nfa_l_closure_name));

				// Controllo se esiste già lo stesso stato nell'automa && non è uno stato unsafe
				if (dfa_new_state != NULL && !dfa_new_state->isMarked()) {
					// Al momento questo pezzo non esiste nello pseudocodice
					DEBUG_LOG("Esiste già uno stato (safe) con la stessa estensione |N, quindi non ho bisogno di crearlo");
				}
				// Non esiste uno stato equivalente con estensione |N
				else {
					// Riga 28
					DEBUG_LOG("Creazione di un nuovo stato con estensione |N");
					dfa_new_state = new ConstructedState(nfa_l_closure);
					// Imposto la distanza, solo perché serve nelle singolarità
					dfa_new_state->setDistance(current_singularity_state->getDistance() + 1);
					dfa->addState(dfa_new_state);
				}

				// Riga 28
				// Per ogni transizione uscente dall'estensione, viene creato e aggiunto alla lista una nuova singolarità
				// Nota: si sta prendendo a riferimento l'NFA associato
				for (string label : dfa_new_state->getLabelsExitingFromExtension()) {
					if (label != EPSILON) {
						this->addSingularityToList(dfa_new_state, label);
					}
				}

				// Riga 29
				// Rimuovo tutte le transizioni rappresentate dalla singolarità
				for (State* current_singularity_child : current_singularity_state->getChildren(current_singularity_label)) {
					DEBUG_LOG("Rimozione della transizione:  %s --(%s)--> %s", current_singularity_state->getName().c_str(), current_singularity_label.c_str(), current_singularity_child->getName().c_str());
					current_singularity_state->disconnectChild(current_singularity_label, current_singularity_child);
				}

				// Righe 30-35
				for (State* unsafe : unsafe_states) {

					// Per tutte le transizioni uscenti da |U
					for (auto &pair : unsafe->getExitingTransitionsRef()) {

						// Escludiamo l'etichetta EPSILON
						if (pair.first == EPSILON) {
							continue;
						}

						// Scorro sui figli dello stato unsafe
						for (State* unsafe_child : pair.second) {
							ConstructedState* c_unsafe_child = static_cast<ConstructedState*> (unsafe_child);

							// Se la transizione arriva in uno stato NON unsafe (= safe), quindi fuori da |U
							if (!c_unsafe_child->isMarked()) {
								// Riga 31
								// Creo una transizione
								dfa_new_state->connectChild(pair.first, c_unsafe_child);
								DEBUG_LOG("Creazione della transizione:  %s --(%s)--> %s", dfa_new_state->getName().c_str(), pair.first.c_str(), c_unsafe_child->getName().c_str());
							}
						}
					}

					// Per tutte le transizioni entranti in |U
					for (auto &pair : unsafe->getIncomingTransitionsRef()) {

						// Escludiamo l'etichetta EPSILON
						if (pair.first == EPSILON) {
							continue;
						}

						// Scorro sui genitori dello stato unsafe
						for (State* unsafe_parent : pair.second) {
							ConstructedState* c_unsafe_parent = static_cast<ConstructedState*> (unsafe_parent);

							// Se la transizione arriva da uno stato NON unsafe (= safe), quindi fuori da |U
							if (!c_unsafe_parent->isMarked()) {
								// Riga 34
								// Creo una singolarità
								this->addSingularityToList(c_unsafe_parent, pair.first);
							}
						}
					}
				}

				// Riga 36
				// Rimuovo dal DFA tutti gli stati unsafe
				for (State* unsafe : unsafe_states) {
					DEBUG_LOG("Rimuovo lo stato unsafe: %s", unsafe->getName().c_str());
					dfa->removeState(unsafe);
					// Nota: le singolarità sono già state rimosse a priori
					//this->m_singularities->removeSingularitiesOfState(static_cast<ConstructedState*>(unsafe));
				}

				// Riga 37
				// Connetto lo stato della singolarità al nuovo stato, attraverso la label della singolarità
				current_singularity_state->connectChild(current_singularity_label, dfa_new_state);
				// this->m_singularities->sort(); // Lo faccio sotto
				// Qui dovrei impostare la distanza. Tuttavia l'ho già fatto prima, perché serve per la singolarità
				//dfa_new_state->setDistance(current_singularity_state->getDistance() + 1);

				// Riga 38
				// Controllo se esiste uno stato duplicato con la stessa estensione
				// In caso affermativo, unisco i due stati. Il che significa unire:
				// - Le due estensioni (che sono la stessa, quindi non le tocco)
				// - Tutte le transizioni di uno nell'altro
				// - Unificare le singolarità nella lista
				DEBUG_LOG("Verifico se esiste un altro stato nel DFA con estensione pari a |N : %s", nfa_l_closure_name.c_str());

				// Estrazione di tutti gli stati con il nome previsto
				vector<State*> namesake_states = dfa->getStatesByName(nfa_l_closure_name);

				// Controllo se esiste più di uno stato con la medesima estensione
				if (namesake_states.size() > 1) {
					DEBUG_LOG("E' stato trovato più di uno stato con la stessa estensione \"%s\"", nfa_l_closure_name.c_str());

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
					bool removed = dfa->removeState(max_dist_state);		// Rimuove il riferimento dello stato
					DEBUG_ASSERT_TRUE( removed );

					// All'interno della lista di singolarità, elimino ogni occorrenza allo stato con distanza massima,
					// salvando tuttavia le label dei singularity che erano presenti.
					set<string> max_dist_singularities_labels = this->m_singularities->removeSingularitiesOfState(max_dist_state);

					// Per tutte le label salvate, se la singolarità corrispondente alla label NON è presente nello stato con distanza minima, la aggiungo
					for (string singularity_label : max_dist_singularities_labels) {
						if (singularity_label != EPSILON && !(min_dist_state == current_singularity_state && singularity_label == current_singularity_label)) {
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

			} // End Scenario 2
		}
		} // End Singularity cycle

		this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_TOTAL] =
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_0] +
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_1] +
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_2];

		this->getRuntimeStatsValuesRef()[IMPACT] =
				((double) this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_TOTAL])
				/ dfa->getTransitionsCount();

		// Restituisco il DFA determinizzato
		return dfa;
	}

	/**
	 * Metodo privato.
	 * Fornisce un'implementazione della procedura "Distance Relocation".
	 * Modifica la distanza di una sequenza di nodi secondo i valori passati come argomento. La modifica
	 * viene poi propagata sui figli finché la nuova distanza risulta migliore. La propagazione avviene
	 * in maniera "width-first".
	 */
	void QuickSubsetConstruction::runDistanceRelocation(list<pair<State*, int>> relocation_sequence) {
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
	void QuickSubsetConstruction::runDistanceRelocation(State* state, int new_distance) {
		pair<State*, int> new_pair(state, new_distance);
		list<pair<State*, int>> list;
		list.push_back(new_pair);
		this->runDistanceRelocation(list);
	}

	/**
	 * Aggiunge una singularity alla lista, occupandosi della creazione e del fatto che possano esserci duplicati.
	 * Eventualmente, segnala anche gli errori.
	 */
	void QuickSubsetConstruction::addSingularityToList(ConstructedState* singularity_state, string singularity_label) {
		Singularity* new_singularity = new Singularity(singularity_state, singularity_label);
		// Provo ad inserire il singularity nella lista
		if (this->m_singularities->insert(new_singularity)) {
			// Caso in cui non sono presenti singularity uguali
			DEBUG_LOG("Aggiungo alla lista la Singularity %s" , new_singularity->toString().c_str());
		} else {
			// Caso in cui esistono singularity duplicati
			DEBUG_LOG("La Singularity %s è già presente nella lista, pertanto non è stata aggiunta" , new_singularity->toString().c_str());
			delete new_singularity;
		}
	}

} /* namespace quicksc */
