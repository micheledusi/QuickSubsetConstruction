/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Timer.hpp
 *
 * 
 * This file contains the definition of a useful macro for time measuring.
 * It can be used every time you want to measure the time spent to execute a block of code.
 */

#include <chrono>

namespace quicksc {
    
    /**
     * Macro function that measures the time spent to execute a block of code.
     * It stores the time in a variable (declared internally to the macro) whose name can be inserted as a parameter.
     * 
     * Example:
     * 
     * MEASURE_MILLISECONDS( time_variable ) {
     * 		// Code to be measured
     * }
     * printf("Time spent: %ul ms", time_variable);
     * 
     */
    #define MEASURE_MILLISECONDS( ms_result ) 											\
        unsigned long int ms_result = 0; 												\
        auto CONCAT( ms_result, _start ) = chrono::high_resolution_clock::now(); 		\
        for (	int CONCAT( ms_result, _for_counter ) = 0; 								\
                CONCAT( ms_result, _for_counter ) < 1;									\
                CONCAT( ms_result, _for_counter++ ),									\
                ms_result = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - CONCAT( ms_result, _start )).count() )
    
} // namespace quicksc