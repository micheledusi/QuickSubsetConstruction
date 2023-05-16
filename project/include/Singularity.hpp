/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Singularity.hpp
 *
 *
 * This header file contains the definition of the Singularity class as a couple of a BidirectionalConstructedState and a string.
 * This definition has its own header file because it is used in many other classes; this reduces the number of
 * dependencies of the other classes and increases the modularity of the code.
 *
 * This header file contains also the definition of the SingularityList class, which is a list of Singularity objects.
 * This class provides some utility methods to manipulate the list of Singularity objects, such as the insertion
 * (with automatic ordering and no duplicates) and the removal of Singularity object with the lowest distance.
 */

#ifndef INCLUDE_SINGULARITY_HPP_
#define INCLUDE_SINGULARITY_HPP_

#include <string>

#include "State.hpp"

using std::set;
using std::string;

namespace quicksc {

	/** Declaration of the Singularity class. */
	class Singularity {

	private:
		BidirectionalConstructedState* m_state;
		string m_label;

	public:
		Singularity(BidirectionalConstructedState* state, string label);
		~Singularity();

		BidirectionalConstructedState* getState();
		string getLabel();
		string toString();

		bool operator<(const Singularity& rhs) const;
		bool operator==(const Singularity& rhs) const;
		int compare(const Singularity& rhs) const;
	};

	struct SingularityComparator {
		bool operator() (const Singularity* lhs, const Singularity* rhs) const {
			return *lhs < *rhs;
		}
	};

	/** Declaration of the SingularityList class. */
	class SingularityList {

	private:
		set<Singularity*, SingularityComparator> m_set;

	public:
		SingularityList();
		~ SingularityList();

		bool empty();
		unsigned int size();
		bool insert(Singularity* new_singularity);
		Singularity* pop();
		string getFirstLabel();
		set<string> removeSingularitiesOfState(BidirectionalConstructedState* state);
		double getAverageLevel();
		void sort();
		void printSingularities();

	};

}

#endif /* INCLUDE_SINGULARITY_HPP_ */
