/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * QuickSubsetConstruction.cpp
 *
 * 
 * This source file implements the QuickSubsetConstruction class, i.e. the class that implements the Quick Subset Construction algorithm.
 * The QuickSubsetCOnstruction class is a subclass of the DeterminizationAlgorithm class, and it's the algorithm studied in the paper.
 */

#include "QuickSubsetConstruction.hpp"

#include <algorithm>

#include "AutomataDrawer.hpp"
#include "Properties.hpp"
#include "Timer.hpp"

//#define DEBUG_MODE
#include "Debug.hpp"

namespace quicksc {

	/**
	 * Empty constructor.
	 * It sets every field to a null value. In order to execute the algorithm it's necessary to call the "loadInputs" method.
	 * It sets the name of the class and the abbreviation of the algorithm, calling the parent constructor.
	 */
	QuickSubsetConstruction::QuickSubsetConstruction(Configurations* configurations)
	: DeterminizationAlgorithm(QSC_ABBR, QSC_NAME) {
		this->m_singularities = NULL;
	}

	/**
	 * Destructor.
	 * It deletes the elements used internally by the algorithm. It doesn't delete the inputs (because they are generated outside)
	 * and it doesn't delete the final result (because it's still used outside).
	 */
	QuickSubsetConstruction::~QuickSubsetConstruction() {
		if (this->m_singularities != NULL) {
			delete this->m_singularities;
		}
	}

	/**
	 * Method that cleans all the internal variables, used by previous executions of the algorithm.
	 * It's called at the beginning of runAutomatonTranslation (that starts the resolution of a
	 * translation problem) and of runAutomatonCheckup (that starts the resolution of a determinization problem).
	 */
	void QuickSubsetConstruction::cleanInternalStatus() {
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
	 * Definition of the runtime statistics of the algorithm.
	 */
	vector<RuntimeStat> QuickSubsetConstruction::getRuntimeStatsList() {
        vector<RuntimeStat> list = DeterminizationAlgorithm::getRuntimeStatsList();
		list.push_back(IMPACT);								// Number of processed singularities over the number of transitions
		list.push_back(EXPECTED_IMPACT); 
		list.push_back(EXPECTED_GAIN);
		list.push_back(NUMBER_SINGULARITIES_CHECKUP);
		list.push_back(NUMBER_SINGULARITIES_SCENARIO_0);
		list.push_back(NUMBER_SINGULARITIES_SCENARIO_1);
		list.push_back(NUMBER_SINGULARITIES_SCENARIO_2);
		list.push_back(NUMBER_SINGULARITIES_TOTAL);
		list.push_back(LEVEL_SINGULARITIES_CHECKUP);
		list.push_back(LEVEL_SINGULARITIES_TOTAL);
		list.push_back(CLONING_TIME);
		list.push_back(RESTRUCTURING_TIME);
		list.push_back(DISTANCE_RELOCATION_TIME);

		return list;
    }

	/**
	 * Returns the NFA passed as parameter, optionally converting its states to the proper State class.
	*/
	Automaton* QuickSubsetConstruction::prepareInputAutomaton(Automaton* nfa) const {
		return nfa->cloneConvertingStates<BidirectionalState>();
	}

	/**
	 * Executes the algorithm on the given inputs.
	 */
	Automaton* QuickSubsetConstruction::run(Automaton* nfa) {
		this->cleanInternalStatus();

		// Input acquisition
		DEBUG_ASSERT_NOT_NULL(nfa);

		// Draws the original automaton
		IF_DEBUG_ACTIVE( AutomataDrawer drawer = AutomataDrawer(nfa) );
		IF_DEBUG_ACTIVE( std::cout << drawer.asString() << std::endl );
		DEBUG_WAIT_USER_ENTER();

		// Istanziation of the automaton to be returned
		this->m_singularities = new SingularityList();
		Automaton* dfa = new Automaton();

		// Local auxiliary variables
		map<BidirectionalState*, BidirectionalConstructedState*> states_map = map<BidirectionalState*, BidirectionalConstructedState*>(); 
		// Since it's necessary to generate an automaton isomorphic to the original one, 
		// this map maintains the correspondence between the states of the NFA and the ones of the DFA.


		/**************************
			FIRST PHASE - CLONING

			In this phase, the algorithm clones the NFA, creating the DFA.
			While doing this, it identifies the singularities of the NFA.

		 **************************/

		MEASURE_NANOSECONDS( cloning_time ) {

			// Iterating on all the states of the input automaton to create the corresponding states
			for (State* nfa_state : nfa->getStatesVector()) {
				BidirectionalState* nfa_bidirectional_state = dynamic_cast<BidirectionalState*>(nfa_state);

				// Creating a copied state in the DFA
				// The copied state will be a ConstructedState, that is a subclass of the State class.
				Extension extension;
				extension.insert(nfa_bidirectional_state);
				BidirectionalConstructedState* dfa_state = new BidirectionalConstructedState(extension);
				dfa->addState(dfa_state);
				dfa_state->setLevel(nfa_bidirectional_state->getLevel());

				// In order to maintain the association, we insert a new couple in the map
				states_map[nfa_bidirectional_state] = dfa_state;

			}
			// Once all the states are created, we can start to create the transitions

			// Iterating on all the states of the input automaton to create the corresponding transitions
			for (State* nfa_state : nfa->getStatesVector()) {
				BidirectionalState* nfa_bidirectional_state = dynamic_cast<BidirectionalState*>(nfa_state);
				DEBUG_LOG("Considering NFA's state %s", nfa_bidirectional_state->getName().c_str());

				// Retrieve the state created in the previous phase, associated to the state of the original automaton
				BidirectionalConstructedState* dfa_state = states_map[nfa_bidirectional_state];

				// Iterating over all the transitions outgoing from the NFA's state
				for (auto &pair : nfa_bidirectional_state->getExitingTransitions()) {

					// Current label
					string current_label = pair.first;

					// This value becomes true as soon as the singularity is added. Avoids useless checks
					bool added_singularity_flag = false;

					// ***** SINGULARITY of type (1) ***** 
					// If the state is the initial one, and the label is the empty string, it's a singularity
					if (nfa->isInitial(nfa_bidirectional_state) && current_label == EPSILON) {
						DEBUG_LOG("The NFA needs a singularity of type (1), i.e. a singularity for the initial state and the epsilon string");
						// Add the singularity, that will be the first to be processed
						this->addSingularityToList(dfa_state, current_label);
						added_singularity_flag = true;
					}

					// For all the children states reached by transitions marked with the original label
					for (State* nfa_child : pair.second) {
						BidirectionalState* nfa_bidirectional_child = dynamic_cast<BidirectionalState*>(nfa_child);

						// Exclude the case with looping epsilon-transition
						if (nfa_bidirectional_child == nfa_bidirectional_state && current_label == EPSILON) {
							continue;
						}

						// Inserting the transition of the NFA in the copy of the DFA
						// The transition is marked with the same label, whatever it is
						State* dfa_child = states_map[nfa_bidirectional_child];
						dfa_state->connectChild(current_label, dfa_child);

						// ***** SINGULARITY of type (2) ***** 
						// If the transition has an epsilon-transition outgoing from the child node, and it's not an epsilon-transition
						if (!added_singularity_flag && current_label != EPSILON && nfa_bidirectional_child->hasExitingTransition(EPSILON)) {

							// I need to check that it's not a looping epsilon-transition
							bool is_not_epsilon_ring = false;
							for (State* nfa_grandchild : nfa_bidirectional_child->getChildren(EPSILON)) {
								if (nfa_grandchild != nfa_bidirectional_child) {
									is_not_epsilon_ring = true;
									break;
								}
							}

							// If it's not a looping epsilon-transition, it's a singularity
							if (is_not_epsilon_ring) {
								this->addSingularityToList(dfa_state, current_label);
							}
						}

					} // Iterating over all the children states reached by transitions marked with the original label

					// ***** SINGULARITY of type (2) *****
					// Checks if the states reached by the current label are more than one
					// In this case, there's a singularity
					if (!added_singularity_flag && current_label != EPSILON && pair.second.size() > 1) {
						this->addSingularityToList(dfa_state, current_label);
					}

				} // Iteration over labels

			} // Iterations over states

			// Set the initial state of the DFA copy, based on the initial state of the NFA, and compute the levels
			BidirectionalState* nfa_initial_state = dynamic_cast<BidirectionalState*>(nfa->getInitialState());
			dfa->setInitialState(states_map[nfa_initial_state]);

		} // End measuring cloning time
		this->getRuntimeStatsValuesRef()[CLONING_TIME] = cloning_time;

		// Saving the number of singularities at the beginning of the algorithm, and their average level
		this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_CHECKUP] = this->m_singularities->size();
		this->getRuntimeStatsValuesRef()[LEVEL_SINGULARITIES_CHECKUP] = this->m_singularities->getAverageLevel();


		/**************************
			SECOND PHASE - RESTRUCTURING

			In this phase, the algorithm restructures the DFA, in order to remove the singularities.
		 **************************/

		double singularities_level_sum = 0;	// Auxiliary variable used to compute the average level of the singularities

		MEASURE_NANOSECONDS(restructuring_time) {

			/***** SCENARIO S_0 (ZERO) *****/

			// Checks on the first singularity of the list.
			// Check if the first singularity is the one related to the initial state with the epsilon label. 
			// To do so, it's enough to check the label (the only possible epsilon-singularity is in fact related to the initial state, 
			// as the check is done in the first phase).
			if (!this->m_singularities->empty() && this->m_singularities->getFirstLabel() == EPSILON) {
				DEBUG_LOG(COLOR_PINK("SCENARIO 0"));
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_0] += 1;

				// Extracting the first element of the list
				DEBUG_LOG("Extracting the first singularity");
				Singularity* initial_singularity = this->m_singularities->pop();

				// Preparing the references to the state and the label
				BidirectionalConstructedState* initial_dfa_state = initial_singularity->getState();

				// Compute the epsilon closure of the initial state on the DFA, denoted also |D
				Extension d0_eps_closure = ConstructedState::computeEpsilonClosure(initial_dfa_state);

				// Compute the epsilon closure of the initial state on the NFA, denoted also |N
				Extension n0_eps_closure = ConstructedState::computeEpsilonClosure(nfa->getInitialState());

				// Computing the unsafe states set, denoted also {U}
				Extension unsafe_states = Extension();
				for (State* dfa_closure_state : d0_eps_closure) {
					BidirectionalConstructedState* c_dfa_closure_state = dynamic_cast<BidirectionalConstructedState*> (dfa_closure_state);

					// Considering the unsafe states
					if (c_dfa_closure_state->isUnsafe(initial_dfa_state, EPSILON)) {
						unsafe_states.insert(dfa_closure_state);
						// Also, to speed up the next check, I mark the unsafe states.
						// The mark hasn't a specific meaning, but it's useful to speed up the check.
						// The mark is not removed, because it's not needed: the state is unsafe, so it will be removed anyway.
						c_dfa_closure_state->setMarked(true);

						DEBUG_LOG("Unsafe state identified: %s", c_dfa_closure_state->getName().c_str());
					}
				}

				// Extending the extension of d0 to |N
				initial_dfa_state->replaceExtensionWith(n0_eps_closure);
				DEBUG_LOG("The extension of the initial state of the DFA is updated to : %s", initial_dfa_state->getName().c_str());

				// Removing the epsilon-transitions outgoing from the initial state
				for (State* eps_child : initial_dfa_state->getChildren(EPSILON)) {
					DEBUG_LOG("Disconnecting the epsilon-transition that connects the initial node %s to the child %s", initial_dfa_state->getName().c_str(), eps_child->getName().c_str());
					initial_dfa_state->disconnectChild(EPSILON, eps_child);
				}

				// Check all the outgoing transitions from the states of the new extension
				DEBUG_LOG("Iterating over all the states of the extension to create the necessary singularities");
				for (State* nfa_state : n0_eps_closure) {

					DEBUG_LOG("Considero lo stato %s", nfa_state->getName().c_str());
					// Skip the initial state
					if (nfa_state == nfa->getInitialState()) {
						DEBUG_LOG("It corresponds to the initial state, so I skip it");
						continue;
					}

					for (auto &pair : nfa_state->getExitingTransitionsRef()) {
						if (pair.first != EPSILON) {
							this->addSingularityToList(initial_dfa_state, pair.first);
						}
					}
				}

				// Iterating over all the unsafe states
				for (State* unsafe : unsafe_states) {
					BidirectionalConstructedState* bc_unsafe = dynamic_cast<BidirectionalConstructedState*> (unsafe);

					// For all the outgoing transitions of the unsafe state
					for (auto &pair : bc_unsafe->getExitingTransitionsRef()) {
						// If the label is not epsilon
						if (pair.first == EPSILON) {
							continue;
						}

						for (State* unsafe_child : pair.second) {
							ConstructedState* c_unsafe_child = dynamic_cast<ConstructedState*> (unsafe_child);

							// Check if the child is NOT unsafe
							if (!c_unsafe_child->isMarked()) {
								// If it's not unsafe, then I add the transition from the initial state
								initial_dfa_state->connectChild(pair.first, unsafe_child);
								DEBUG_LOG("Creating transition:  %s --(%s)--> %s", initial_dfa_state->getName().c_str(), pair.first.c_str(), unsafe_child->getName().c_str());
							}
						}
					}

					// For all the transitions entering the unsafe state
					for (auto &pair : bc_unsafe->getIncomingTransitionsRef()) {
						// If the label is not epsilon
						if (pair.first == EPSILON) {
							continue;
						}

						for (State* unsafe_parent : pair.second) {
							BidirectionalConstructedState* bc_unsafe_parent = dynamic_cast<BidirectionalConstructedState*> (unsafe_parent);

							// Check if the parent is NOT unsafe
							if (!bc_unsafe_parent->isMarked()) {
								// If it's not unsafe, then I create the singularity
								this->addSingularityToList(bc_unsafe_parent, pair.first);
							}
						}
					}
				}

				// Deleting the unsafe states
				for (State* unsafe : unsafe_states) {
					DEBUG_LOG("Removing the unsafe state: %s", unsafe->getName().c_str());
					dfa->removeState(unsafe);
					this->m_singularities->removeSingularitiesOfState(dynamic_cast<BidirectionalConstructedState*>(unsafe));
				}

			}


			/***** SCENARI S_1 e S_2 *****/

			DEBUG_LOG("Now the cycle over the singularities begins");

			// Until there are singularities to process
			while (!this->m_singularities->empty()) {

				DEBUG_MARK_PHASE( "Nuova iterazione per una nuova singolarità" ) {

				DEBUG_LOG("Printing the current situation of the automaton");
				IF_DEBUG_ACTIVE( AutomataDrawer drawer = AutomataDrawer(dfa) );
				IF_DEBUG_ACTIVE( std::cout << drawer.asString() << std::endl );
				DEBUG_WAIT_USER_ENTER();

				DEBUG_LOG("Current singularities list (#%d):", this->m_singularities->size());
				IF_DEBUG_ACTIVE( this->m_singularities->printSingularities() );

				// Extracting the first singularity, on the top of the list/queue
				Singularity* current_singularity = this->m_singularities->pop();
				DEBUG_LOG("Extracting the current singularity: %s", current_singularity->toString().c_str());

				DEBUG_WAIT_USER_ENTER();

				// References to the state and the label of the singularity
				BidirectionalConstructedState* current_singularity_state = current_singularity->getState();
				string current_singularity_label = current_singularity->getLabel();

				// Compute the ell-clousure of the state of the singularity, with the singularity label
				Extension nfa_l_closure = current_singularity_state->computeLClosureOfExtension(current_singularity_label); // In the algorithm, this is called "|N|" (a bold "N")
				string nfa_l_closure_name = BidirectionalConstructedState::createNameFromExtension(nfa_l_closure);
				DEBUG_LOG("|N| = %s", nfa_l_closure_name.c_str());


				/***** SCENARIO S_1 (ONE) *****/

				// If there are no outgoing transitions from the state of the singularity with the label of the singularity
				if (!current_singularity_state->hasExitingTransition(current_singularity_label)) {
					DEBUG_LOG(COLOR_PINK("SCENARIO 1"));
					this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_1] += 1;
					singularities_level_sum += current_singularity_state->getLevel();

					// If the state already exists in the DFA
					if (dfa->hasState(nfa_l_closure_name)) {

						// Adding the transition from the state of the singularity to the state of the ell-closure
						BidirectionalConstructedState* child = dynamic_cast<BidirectionalConstructedState*>(dfa->getState(nfa_l_closure_name));
						current_singularity_state->connectChild(current_singularity_label, child);
						DEBUG_LOG("Creating the transition: %s --(%s)--> %s",
								current_singularity_state->getName().c_str(), current_singularity_label.c_str(), child->getName().c_str());

						// Fix the level
						this->runLevelRelocation(child, current_singularity_state->getLevel() + 1);

					}
					// If the state does not exists in the DFA
					else {

						// Create a new state and connect it to the current state
						BidirectionalConstructedState* new_state = new BidirectionalConstructedState(nfa_l_closure);
						dfa->addState(new_state);
						current_singularity_state->connectChild(current_singularity_label, new_state);
						new_state->setLevel(current_singularity_state->getLevel() + 1);

						DEBUG_LOG("Creating the transition: %s --(%s)--> %s",
								current_singularity_state->getName().c_str(), current_singularity_label.c_str(), new_state->getName().c_str());

						// For each outgoing transition from the extension, a new singularity is created and added to the list
						// Note: the NFA is taken as reference
						for (string label : new_state->getLabelsExitingFromExtension()) {
							if (label != EPSILON) {
								this->addSingularityToList(new_state, label);
							}
						}

					}

				}

				// Otherwise, there are transitions outgoing from the state with the singularity label as the marking label
				else {
					// Check the status of such transitions, to verify if the scenario 2 can be applied
					bool scenario_2_flag = false;
					DEBUG_LOG("Checking the conditions for the scenario 2");

					// CONDITION: there must be at least two ell-children
					if (current_singularity_state->getChildrenRef(current_singularity_label).size() > 1) {
						DEBUG_LOG("The state %s has at least two transitions marked with label %s", current_singularity_state->getName().c_str(), current_singularity_label.c_str());
						scenario_2_flag = true;
					}
					// Otherwise
					else {
						State* current_singularity_child = current_singularity_state->getChild(current_singularity_label);
						DEBUG_LOG("The state %s has only one transitions marked with label %s, reaching state %s", current_singularity_state->getName().c_str(), current_singularity_label.c_str(), current_singularity_child->getName().c_str());

						// The only existing child must have an outgoing epsilon transition
						if (current_singularity_child->hasExitingTransition(EPSILON)) {
							DEBUG_LOG("The child state %s has an exiting epsilon-transition", current_singularity_child->getName().c_str());
							scenario_2_flag = true;
						}
						// Otherwise
						else {
							DEBUG_LOG("The child state %s has no exiting epsilon-transitions", current_singularity_child->getName().c_str());
							BidirectionalConstructedState* c_current_singularity_child = dynamic_cast<BidirectionalConstructedState*> (current_singularity_child);
							if (!c_current_singularity_child->hasExtension(nfa_l_closure)) {
								DEBUG_LOG("The child state has an extension different from |N|; its extension is %s", nfa_l_closure_name.c_str());
								scenario_2_flag = true;
							}
						}
					}


					// If the scenario 2 does not apply, the cycle ends and we go to the next singularity
					if (!scenario_2_flag) {
						continue;
					}

					/***** SCENARIO S_2 (TWO) *****/

					DEBUG_LOG(COLOR_PINK("SCENARIO 2"));
					this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_2] += 1;
					singularities_level_sum += current_singularity_state->getLevel();

					Extension dfa_l_closure = current_singularity_state->computeLClosure(current_singularity_label);
					DEBUG_LOG("The ell-closure on the DFA (denoted |D) contains %lu states", dfa_l_closure.size());

					// Computing the list of unsafe states
					set<BidirectionalConstructedState*, State::Comparator> unsafe_states;
					for (State* ell_child : dfa_l_closure) {
						BidirectionalConstructedState* c_ell_child = dynamic_cast<BidirectionalConstructedState*> (ell_child);

						if (c_ell_child->isUnsafe(current_singularity_state, current_singularity_label)) {
							// Adding to the list the state
							unsafe_states.insert(c_ell_child);
							c_ell_child->setMarked(true);
							// Removing the outgoing singularities from the state
							this->m_singularities->removeSingularitiesOfState(dynamic_cast<BidirectionalConstructedState*>(c_ell_child));
						}
					}

					BidirectionalConstructedState* dfa_new_state = dynamic_cast<BidirectionalConstructedState*> (dfa->getState(nfa_l_closure_name));

					// Checks if the state already exists in the DFA and it is not unsafe
					if (dfa_new_state != NULL && !dfa_new_state->isMarked()) {
						DEBUG_LOG("There is already a state (safe) with the same extension |N, so I do not need to create it");
					}
					// There's no state with extension |N
					else {
						DEBUG_LOG("Creating a new state with extension |N");
						dfa_new_state = new BidirectionalConstructedState(nfa_l_closure);
						dfa_new_state->setLevel(current_singularity_state->getLevel() + 1);
						dfa->addState(dfa_new_state);
					}

					// For each outgoing transition from the extension, a new singularity is created and added to the list
					for (string label : dfa_new_state->getLabelsExitingFromExtension()) {
						if (label != EPSILON) {
							this->addSingularityToList(dfa_new_state, label);
						}
					}

					// Remove all the transitions represented by the singularity
					for (State* current_singularity_child : current_singularity_state->getChildren(current_singularity_label)) {
						DEBUG_LOG("Deleting the transition:  %s --(%s)--> %s", current_singularity_state->getName().c_str(), current_singularity_label.c_str(), current_singularity_child->getName().c_str());
						current_singularity_state->disconnectChild(current_singularity_label, current_singularity_child);
					}
					
					for (BidirectionalConstructedState* unsafe : unsafe_states) {


						// For each outgoing transition from |U
						for (auto &pair : unsafe->getExitingTransitionsRef()) {

							// We skip the EPSILON label
							if (pair.first == EPSILON) {
								continue;
							}

							// Iterating over the children of the unsafe state
							for (State* unsafe_child : pair.second) {
								BidirectionalConstructedState* c_unsafe_child = dynamic_cast<BidirectionalConstructedState*> (unsafe_child);

								// If the transition arrives in a state that is not unsafe, so outside from |U
								if (!c_unsafe_child->isMarked()) {
									// The transition is added to the DFA
									dfa_new_state->connectChild(pair.first, c_unsafe_child);
									DEBUG_LOG("Creating the transition:  %s --(%s)--> %s", dfa_new_state->getName().c_str(), pair.first.c_str(), c_unsafe_child->getName().c_str());
								}
							}
						}

						// For each incoming transition to |U
						for (auto &pair : unsafe->getIncomingTransitionsRef()) {

							// We exclude the EPSILON label
							if (pair.first == EPSILON) {
								continue;
							}

							// Iterating over the parents of the unsafe state
							for (State* unsafe_parent : pair.second) {
								BidirectionalConstructedState* c_unsafe_parent = dynamic_cast<BidirectionalConstructedState*> (unsafe_parent);

								// If the transition arrives from a state that is not unsafe, so outside from |U
								if (!c_unsafe_parent->isMarked()) {
									// A new singularity is created and added to the list
									this->addSingularityToList(c_unsafe_parent, pair.first);
								}
							}
						}
					}

					// Removing the unsafe states from the DFA
					for (State* unsafe : unsafe_states) {
						DEBUG_LOG("Removing the unsafe state: %s", unsafe->getName().c_str());
						dfa->removeState(unsafe);
					}

					// Connecting the singularity state to the new state, through the singularity label
					current_singularity_state->connectChild(current_singularity_label, dfa_new_state);

					// Extracting all the states with a given name (i.e. the same extension)
					vector<State*> namesake_states = dfa->getStatesByName(nfa_l_closure_name);

					// If there's more than one state with the same name, it means that there are two states with the same extension
					if (namesake_states.size() > 1) {
						DEBUG_LOG("More than one state with the same extension \"%s\" has been found", nfa_l_closure_name.c_str());

						BidirectionalConstructedState* first_state = dynamic_cast<BidirectionalConstructedState*>(namesake_states[0]);
						BidirectionalConstructedState* second_state = dynamic_cast<BidirectionalConstructedState*>(namesake_states[1]);

						BidirectionalConstructedState* min_dist_state;
						BidirectionalConstructedState* max_dist_state;

						if (first_state->getLevel() < second_state->getLevel()) {
							min_dist_state = first_state;
							max_dist_state = second_state;
						} else {
							min_dist_state = second_state;
							max_dist_state = first_state;
						}

						DEBUG_ASSERT_TRUE( min_dist_state->getLevel() <= max_dist_state->getLevel() );

						DEBUG_MARK_PHASE("Copying the transitions") {
							min_dist_state->copyAllTransitionsOf(max_dist_state);
						}

						// Removing the state from the DFA
						bool removed = dfa->removeState(max_dist_state);
						DEBUG_ASSERT_TRUE( removed );

						// In the list of singularity, I remove every occurrence to the state with maximum level,
						// however saving the labels of the singularity that were present.
						set<string> max_dist_singularities_labels = this->m_singularities->removeSingularitiesOfState(max_dist_state);

						// For all the saved labels, if the singularity corresponding to the label is NOT present in the state with minimum level, I add it
						for (string singularity_label : max_dist_singularities_labels) {
							if (singularity_label != EPSILON && !(min_dist_state == current_singularity_state && singularity_label == current_singularity_label)) {
								this->addSingularityToList(min_dist_state, singularity_label);
							}
						}

						// Level relocation procedure on all the children of the state with minimum level, because the children acquired from the state
						// with maximum level must be modified
						list<pair<BidirectionalState*, int>> to_be_relocated_list;
						for (auto &trans : min_dist_state->getExitingTransitionsRef()) {
							for (State* child : trans.second) {
								BidirectionalState* bidir_child = dynamic_cast<BidirectionalState*> (child);
								to_be_relocated_list.push_back(pair<BidirectionalState*, int>(bidir_child, min_dist_state->getLevel() + 1));
							}
						}
						this->runLevelRelocation(to_be_relocated_list);
						this->m_singularities->sort();

					}

				} // End Scenario 2
			}
			} // End Singularity cycle

		} // End measuring restructuring time
		this->getRuntimeStatsValuesRef()[RESTRUCTURING_TIME] = restructuring_time;

		this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_TOTAL] =
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_0] +
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_1] +
				this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_SCENARIO_2];

		this->getRuntimeStatsValuesRef()[LEVEL_SINGULARITIES_TOTAL] = 
				singularities_level_sum 
				/ ((double) this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_TOTAL]);

		this->getRuntimeStatsValuesRef()[IMPACT] =
				((double) this->getRuntimeStatsValuesRef()[NUMBER_SINGULARITIES_TOTAL])
				/ dfa->getTransitionsCount();

		double exp_impact = ((double) this->getRuntimeStatsValuesRef()[IMPACT]) * SCALE_FACTOR_QSC;
		this->getRuntimeStatsValuesRef()[EXPECTED_IMPACT] = exp_impact;

		this->getRuntimeStatsValuesRef()[EXPECTED_GAIN] = exp_impact <= 1.0 ?
				1.0 - exp_impact :
				1.0 / exp_impact - 1.0;

		// Returns the DFA
		return dfa;
	}

	/**
	 * Private method.
	 * Provides an implementation of the "Level Relocation" procedure.
	 * Modifies the level of a sequence of nodes according to the values passed as argument. The modification
	 * is then propagated on the children until the new level is better. The propagation occurs
	 * in a "width-first" way.
	 */
	void QuickSubsetConstruction::runLevelRelocation(list<pair<BidirectionalState*, int>> relocation_sequence) {
		MEASURE_NANOSECONDS( dist_reloc_time ) {
			while (!relocation_sequence.empty()) {
				auto current = relocation_sequence.front();
				relocation_sequence.pop_front();
				BidirectionalState* current_state = current.first;

				DEBUG_LOG("Execution of \"Level Relocation\" on the state %s", current_state->getName().c_str());

				if (current_state->getLevel() > current.second) {
					DEBUG_LOG("The level has been reduced from %u to %u", current_state->getLevel(), current.second);
					current_state->setLevel(current.second);

					for (auto &trans : current_state->getExitingTransitionsRef()) {
						for (State* child : trans.second) {
							BidirectionalState* bidirectional_child = dynamic_cast<BidirectionalState*>(child);
							relocation_sequence.push_back(pair<BidirectionalState*, int>(bidirectional_child, current.second + 1));
						}
					}
				}
			}
		}
		this->getRuntimeStatsValuesRef()[DISTANCE_RELOCATION_TIME] += dist_reloc_time;
	}

	/**
	 * Private method.
	 * Wrapper for the "runLevelRelocation" function that requires a list of pairs (State*, int) as input.
	 * Since more than once, inside the "Singularity Processing" algorithm, the "Level Relocation" procedure
	 * is called with a single argument, this method provides a useful interface to simplify the construction
	 * of the parameters of the call.
	 */
	void QuickSubsetConstruction::runLevelRelocation(BidirectionalState* state, int new_level) {
		pair<BidirectionalState*, int> new_pair(state, new_level);
		list<pair<BidirectionalState*, int>> list;
		list.push_back(new_pair);
		this->runLevelRelocation(list);
	}

	/**
	 * Adds a singularity to the list, taking care of the creation and the fact that there can be duplicates.
	 * Eventually, it also signals the errors.
	 */
	void QuickSubsetConstruction::addSingularityToList(BidirectionalConstructedState* singularity_state, string singularity_label) {
		Singularity* new_singularity = new Singularity(singularity_state, singularity_label);
		if (this->m_singularities->insert(new_singularity)) {
			DEBUG_LOG("Adding the singularity %s to the list" , new_singularity->toString().c_str());
		} else {
			DEBUG_LOG("The singularity %s is already present in the list, thus it has not been added" , new_singularity->toString().c_str());
			delete new_singularity;
		}
	}

} /* namespace quicksc */
