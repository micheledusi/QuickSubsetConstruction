/*
 * Singularity.cpp
 *
 * Implementazione della struttura Singularity e SingularityList.
 * Una SingularityList garantisce:
 * - ordinamento dele singolarità.
 * - accesso al primo elemento della lista.
 * - unicità degli elementi.
 */

#include "Singularity.hpp"

#include "Alphabet.hpp"
#include "Debug.hpp"

namespace quicksc {

	/**
	 * Costruttore.
	 */
	Singularity::Singularity(ConstructedState* state, string label) {
		if (state == NULL) {
			DEBUG_LOG_ERROR("Impossibile creare una singolarità con stato vuoto");
		}
		this->m_state = state;
		this->m_label = label;
	}

	/**
	 * Distruttore.
	 * Nota: lo stato a cui fa riferimento il singularity NON è eliminato
	 * quando viene distrutto il singularity.
	 */
	Singularity::~Singularity() {}

	/**
	 * Restituisce lo stato della Singularity.
	 */
	ConstructedState* Singularity::getState() {
		return this->m_state;
	}

	/**
	 * Restituisce la label della Singularity.
	 */
	string Singularity::getLabel() {
		return this->m_label;
	}

	/**
	 * Restituisce una descrizione testuale della Singularity.
	 */
	string Singularity::toString() {
		return ("(" + this->m_state->getName() + ", " + SHOW(this->m_label) + ")\033[33m[" + std::to_string(this->m_state->getDistance()) + "]\033[0m");
	}

	/**
	 * Override dell'operatore less.
	 */
	bool Singularity::operator<(const Singularity& rhs) const {
		return (this->compare(rhs) < 0);
	}

	/**
	 * Override dell'operatore equal.
	 */
	bool Singularity::operator==(const Singularity& rhs) const {
		return (this->compare(rhs) == 0);
	}

	/**
	 * Funzione di comparazione di due Singularity sulla base di:
	 * 1) La distanza dello stato dallo stato iniziale dell'automa.
	 * 2) Il nome dello stato.
	 * 3) La label.
	 */
	int Singularity::compare(const Singularity& rhs) const {
		// Verifico le distanze degli stati
		if (this->m_state->getDistance() == rhs.m_state->getDistance()) {
			// Caso: Distanze degli stati uguali

			// Verifico i nomi degli stati
			if (this->m_state->getName() == rhs.m_state->getName()) {
				// Caso: Nomi degli stati uguali

				// Il confronto si opera sulle labels
				return this->m_label.compare(rhs.m_label);

			} else {
				// Caso: Nomi degli stati diversi
				// Il confronto si opera sui nomi
				return this->m_state->getName().compare(rhs.m_state->getName());
			}
		} else {
			// Caso: Distanze degli stati diverse
			// Il confronto si opera sulle distanze
			return (this->m_state->getDistance() - rhs.m_state->getDistance());
		}
	}

	/**
	 * Costruttore.
	 * Si occupa di inizializzare le strutture dati.
	 */
	SingularityList::SingularityList() : m_set() {}

	/**
	 * Distruttore vuoto
	 */
	SingularityList::~SingularityList() {}

	/**
	 * Restituisce l'informazione riguardo alla presenza di almeno
	 * una singolarità nella lista.
	 */
	bool SingularityList::empty() {
		return this->m_set.empty();
	}

	/**
	 * Restituisce la dimensione della lista, i.e. il numero di elementi.
	 */
	unsigned int SingularityList::size() {
		return this->m_set.size();
	}

	/**
	 * Inserisce un nuovo singularity all'interno della lista,
	 * solamente se questo singularity non è già presente.
	 * In caso l'inserimento vada a buon fine, restituisce TRUE.
	 */
	bool SingularityList::insert(Singularity* new_singularity) {
		// Tento l'inserimento all'interno del Set;
		return ((this->m_set.insert(new_singularity)).second);
	}

	/**
	 * Estrae il primo elemento della lista.
	 */
	Singularity* SingularityList::pop() {
		Singularity* first = *(this->m_set.begin());
		this->m_set.erase(this->m_set.begin());
		return first;
//		return (this->m_set.extract(this->m_set.begin()).value()); // Vecchia implementazione, funzionante solo con C++17
	}

	/**
	 * Restituisce (SENZA ESTRARRE NULLA) l'etichetta della prima singolarità della lista.
	 */
	string SingularityList::getFirstLabel() {
		Singularity* first = *(this->m_set.begin());
		return first->getLabel();
	}

	/**
	 * Stampa tutte le singolarità rimanenti nella lista, in ordine.
	 */
	void SingularityList::printSingularities() {
		for (Singularity* b : this->m_set) {
			std::cout << b->toString() << std::endl;
		}
	}

	/**
	 * Rimuove tutti i singularities della lista relativi ad un particolare stato.
	 * Le singolarità vengono eliminate.
	 * Inoltre, restituisce tutte le label che appartenenvano a quele singolarità.
	 */
	set<string> SingularityList::removeSingularitiesOfState(ConstructedState* target_state) {
		DEBUG_LOG("Stampa di tutti le singolarità attualmente presenti:");
		IF_DEBUG_ACTIVE(printSingularities());

		set<string> removed_labels = set<string>();
		for (auto singularity_iterator = this->m_set.begin(); singularity_iterator != this->m_set.end(); /* No increment */) {

			DEBUG_LOG_SUCCESS("Iterazione sulla singolarità %s", (*singularity_iterator)->toString().c_str());
			if ((*singularity_iterator)->getState() == target_state) {

				DEBUG_LOG("Ho trovato una singolarità associata allo stato %s da rimuovere dalla lista delle singolarità", target_state->getName().c_str());

				DEBUG_LOG("Memorizzo la label %s", (*singularity_iterator)->getLabel().c_str());
				removed_labels.insert((*singularity_iterator)->getLabel());

				DEBUG_LOG("Rimuovo il singularity %s", (*singularity_iterator)->toString().c_str());
				this->m_set.erase(singularity_iterator++);

			} else {

				DEBUG_LOG("La singolarità %s non è associata allo stato %s", (*singularity_iterator)->toString().c_str(), target_state->getName().c_str());
				++singularity_iterator;
			}
		}


		DEBUG_LOG("Stampa di tutti le singolarità rimasti nella lista:");
		IF_DEBUG_ACTIVE(printSingularities());

		return removed_labels;
	}

	/**
	 * Funzione che riordina gli elementi della lista di singolarità.
	 * E' opportuno chiamare raramente questa funzione, poiché il sorting non è particolarmente efficiente,
	 * dato l'uso sottostante di un set (che dovrebbe richiedere il sorting automatico).
	 *
	 * Non ci sono molte altre soluzioni, nel senso che è possibile che le singolarità cambino internamente (per le distanze)
	 * e che perciò la lista sia da riordinare. Avrei potuto usare una lista con priorità (inserimento logaritmico e rimozione costante)
	 * ma il controllo sull'unicità mi avrebbe comunque richiesto un <set>, che ha complessitù logaritmica a prescindere.
	 */
	void SingularityList::sort() {
		set<Singularity*, SingularityComparator> new_set = set<Singularity*, SingularityComparator>();
		for (auto it = this->m_set.begin(); it != this->m_set.end(); it++) {
			new_set.insert(*it);
		}
		this->m_set = new_set;
	}

}
