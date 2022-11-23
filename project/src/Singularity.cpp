/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Singularity.cpp
 *
 * 
 * This source file contains the definition of the classes Singularity and SingularityList.
 * 
 * A SingularityList guarantees:
 * - the uniqueness of the Singularity objects it contains
 * - the order of the Singularity objects it contains
 * - a fast access to the first Singularity object in the list
 */

#include "Singularity.hpp"

#include "Alphabet.hpp"
#include "Debug.hpp"

namespace quicksc {

	/**
	 * Constructor.
	 */
	Singularity::Singularity(ConstructedState* state, string label) {
		if (state == NULL) {
			DEBUG_LOG_ERROR("Impossibile creare una singolarità con stato vuoto");
		}
		this->m_state = state;
		this->m_label = label;
	}

	/**
	 * Destructor.
	 * 
	 * NOTE: the destructor of the state is not called.
	 */
	Singularity::~Singularity() {}

	/**
	 * Getter for the state.
	 */
	ConstructedState* Singularity::getState() {
		return this->m_state;
	}

	/**
	 * Getter for the label.
	 */
	string Singularity::getLabel() {
		return this->m_label;
	}

	/**
	 * Returns a string representation of the singularity.
	 */
	string Singularity::toString() {
		return ("(" + this->m_state->getName() + ", " + SHOW(this->m_label) + ")\033[33m[" + std::to_string(this->m_state->getDistance()) + "]\033[0m");
	}

	/**
	 * Override of the "<" operator.
	 */
	bool Singularity::operator<(const Singularity& rhs) const {
		return (this->compare(rhs) < 0);
	}

	/**
	 * Override of the "==" operator.
	 */
	bool Singularity::operator==(const Singularity& rhs) const {
		return (this->compare(rhs) == 0);
	}

	/**
	 * Comparison function between two Singularity objects, depending on:
	 * 1) The distance of the state from the initial state of the automaton.
	 * 2) The name of the state.
	 * 3) The label.
	 */
	int Singularity::compare(const Singularity& rhs) const {
		// Check the distance of the states
		if (this->m_state->getDistance() == rhs.m_state->getDistance()) {
			// Case: Distances of the states are equal

			// Check the names of the states
			if (this->m_state->getName() == rhs.m_state->getName()) {
				// Case: Names of the states are equal

				// The comparison is done on the labels
				return this->m_label.compare(rhs.m_label);

			} else {
				// Case: Names of the states are different
				// The comparison is done on the names of the states
				return this->m_state->getName().compare(rhs.m_state->getName());
			}
		} else {
			// Case: Distances of the states are different
			// The comparison is done on the distances of the states
			return (this->m_state->getDistance() - rhs.m_state->getDistance());
		}
	}

	/**
	 * Constructor.
	 */
	SingularityList::SingularityList() : m_set() {}

	/**
	 * Destructor.
	 */
	SingularityList::~SingularityList() {}

	/**
	 * If the SingularityList is empty (i.e. it has no elements in it), returns TRUE. Otherwise, returns FALSE.
	 */
	bool SingularityList::empty() {
		return this->m_set.empty();
	}

	/**
	 * Getter for the size of the list, i.e. the number of elements in the list.
	 */
	unsigned int SingularityList::size() {
		return this->m_set.size();
	}

	/**
	 * Inserts a new Singularity in the list, only if it is not already present.
	 * If the insertion is successful, returns TRUE.
	 */
	bool SingularityList::insert(Singularity* new_singularity) {
		return ((this->m_set.insert(new_singularity)).second);
	}

	/**
	 * Extracts the first element of the list.
	 */
	Singularity* SingularityList::pop() {
		Singularity* first = *(this->m_set.begin());
		this->m_set.erase(this->m_set.begin());
		return first;
	}

	/**
	 * Returns the label of the first singularity of the list.
	 * NOTE: (IMPORTANT) the singularity is not removed from the list.
	 */
	string SingularityList::getFirstLabel() {
		Singularity* first = *(this->m_set.begin());
		return first->getLabel();
	}

	/**
	 * Prints all the remaining singularities in the list, in order.
	 */
	void SingularityList::printSingularities() {
		for (Singularity* b : this->m_set) {
			std::cout << b->toString() << std::endl;
		}
	}

	/**
	 * Removes all the singularities of the list related to a particular state.
	 * The singularities are deleted.
	 * Moreover, it returns all the labels that belonged to those singularities.
	 */
	set<string> SingularityList::removeSingularitiesOfState(ConstructedState* target_state) {
		DEBUG_LOG("Printing the singularities of the list for the state %s", target_state->getName().c_str());
		IF_DEBUG_ACTIVE(printSingularities());

		set<string> removed_labels = set<string>();
		for (auto singularity_iterator = this->m_set.begin(); singularity_iterator != this->m_set.end(); /* No increment */) {

			if ((*singularity_iterator)->getState() == target_state) {
				removed_labels.insert((*singularity_iterator)->getLabel());
				this->m_set.erase(singularity_iterator++);

			} else {
				++singularity_iterator;
			}
		}

		DEBUG_LOG("Printing the singularities of the list for the state %s after the removal", target_state->getName().c_str());
		IF_DEBUG_ACTIVE(printSingularities());

		return removed_labels;
	}

	/**
	 * Function that reorders the elements of the list of singularities.
	 * It is advisable to call this function rarely, because the sorting is not particularly efficient,
	 * given the use of a set (which should require automatic sorting).
	 *
	 * There are not many other solutions, in the sense that it is possible that the singularities change internally (for the distances)
	 * and therefore the list must be reordered. I could have used a list with priority (logarithmic insertion and constant removal)
	 * but the uniqueness control would still have required a <set>, which has logarithmic complexity regardless.
	 */
	void SingularityList::sort() {
		set<Singularity*, SingularityComparator> new_set = set<Singularity*, SingularityComparator>();
		for (auto it = this->m_set.begin(); it != this->m_set.end(); it++) {
			new_set.insert(*it);
		}
		this->m_set = new_set;
	}

}
