/*
 * SubsetConstruction.cpp
 *
 * Project: TranslatedAutomata
 *
 * Implementa l'algoritmo di "Subset Construction" per la generazione di DFA
 * equivalenti ad un NFA di partenza.
 *
 */

#include "SubsetConstruction.hpp"

#include <queue>

#include "Debug.hpp"
#include "Properties.hpp"
#include "State.hpp"

namespace quicksc {

	/**
	 * Costruttore vuoto.
	 * Imposta il nome della classe e l'abbreviazione dell'algoritmo, chiamando il costruttore padre.
	 */
	SubsetConstruction::SubsetConstruction() : DeterminizationAlgorithm(SC_ABBR, SC_NAME) {};

	/**
	 * Distruttore vuoto.
	 */
	SubsetConstruction::~SubsetConstruction() {};

    /**
     * Esegue l'algoritmo "Subset Construction".
     * Nota: siamo sempre nel caso in cui NON esistono epsilon-transizioni.
     */
	Automaton* SubsetConstruction::run(Automaton* nfa) {

		// Creo l'automa a stati finiti deterministico, inizialmente vuoto
		Automaton* dfa = new Automaton();

        // Creo lo stato iniziale per il DFA
		Extension initial_dfa_extension;
		State* nfa_initial_state = nfa->getInitialState();
		DEBUG_ASSERT_NOT_NULL(nfa_initial_state);
		initial_dfa_extension.insert(nfa_initial_state);
		Extension epsilon_closure = ConstructedState::computeEpsilonClosure(initial_dfa_extension);
		ConstructedState * initial_dfa_state = new ConstructedState(epsilon_closure);

		// Inserisco lo stato all'interno del DFA
        dfa->addState(initial_dfa_state);

        // Stack per gli stati ancora da processare (singularity)
        std::queue<ConstructedState*> singularities_stack;

        // Inserisco come singularity di partenza il nodo iniziale
        singularities_stack.push(initial_dfa_state);

        // Finché nella queue sono presenti delle singularity
        while (! singularities_stack.empty()) {

        	// Estraggo il primo elemento della queue
        	ConstructedState* current_state = singularities_stack.front();			// Ottengo un riferimento all'elemento estratto
            singularities_stack.pop();								// Rimuovo l'elemento

            // Per tutte le label che marcano transizioni uscenti da questo stato
            for (string l : current_state->getLabelsExitingFromExtension()) {
            	// Salto le epsilon-transizioni
            	if (l == EPSILON) {
            		continue;
            	}

            	// Computo la l-closure dello stato e creo un nuovo stato DFA
            	Extension l_closure = current_state->computeLClosureOfExtension(l);
            	ConstructedState* new_state = new ConstructedState(l_closure);
            	DEBUG_LOG("Dallo stato %s, con la label %s, ho creato lo stato %s",
            			current_state->getName().c_str(),
						l.c_str(),
						new_state->getName().c_str());

                // Verifico se lo stato DFA creato è vuoto
                if (new_state->isExtensionEmpty()) {
                	// Se sì, lo elimino e procedo
                	DEBUG_LOG("Estensione vuota, salto l'iterazione");
                    delete new_state;
                    continue;
                }
                // Verifico se lo stato DFA creato è già presente nel DFA (come nome)
                else if (dfa->hasState(new_state->getName())) {
                	// Se sì, lo stato estratto dalla queue può essere eliminato
                	DEBUG_LOG("Lo stato è già presente, lo elimino e recupero quello vecchio");
                	ConstructedState* tmp_state = new_state;
                    new_state = (ConstructedState*) dfa->getState(tmp_state->getName());
                    delete tmp_state;
                }
                // Se si tratta di uno stato "nuovo"
                else {
                	// Lo aggiungo al DFA e alla queue
                	DEBUG_LOG("Lo stato è nuovo, lo aggiungo all'automa");
                    dfa->addState(new_state);
                    singularities_stack.push(new_state);
                }

                // Effettuo la connessione:
                //	state--(l)-->new_state
                current_state->connectChild(l, new_state);
            }
        }

        // Imposto lo stato iniziale
        // Questa operazione sistema le distanze in automatico
        dfa->setInitialState(initial_dfa_state);

        return dfa;
	}
}
