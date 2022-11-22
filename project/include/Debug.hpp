/*
 * Michele Dusi, Gianfranco Lamperti
 * Quick Subset Construction
 * 
 * Debug.hpp
 *
 *
 * This file is part of a simple library for debugging purposes.
 * It offers a set of macros that can be used to print debug messages.
 * 
 * To activate the debug messages, the DEBUG_MODE macro must be defined.
 * To deactivate the debug messages, the DEBUG_MODE macro must be undefined (you can comment it).
 */

#ifndef INCLUDE_DEBUG_HPP_
#define INCLUDE_DEBUG_HPP_

#include <iostream>
#include <stdio.h>
#include <string>

namespace quicksc {

	/**
	 * Activates or deactivates the debug messages (the debug mode) by respectively defining or undefining the DEBUG_MODE macro.
	 */
//	#define DEBUG_MODE

	/**
	 * Activates or deactivates the functionality of waiting the user to press a key for continuing the execution.
	 * The macro must be defined as "true" or "false".
	 */
	#define WAIT_FOR_USER false


	/***********************************************************/

	// Utility macros

	/** Concatenates two values x and y */
	#define _CONCAT( x, y )				x ## y
	#define CONCAT( x, y )				_CONCAT( x, y )

	/** "Stringifies" the value of a parameter x */
	#define _STRINGIFY( x ) 			#x
	#define STRINGIFY( x )				_STRINGIFY( x )

	/** Prefix for the definition of unique IDs for variables */
	#define ID_PREFIX 					debug_var_

	/** Macro for the generation of unique IDs. It assumes there's only one ID per line. */
	#define UNIQUE_ID					CONCAT( ID_PREFIX, __LINE__ )

	/** Adds square brackets to the text, in order to present the tag in an ordered way */
	#define TAG_BRACKETS( text ) 		"[" text "]"

	/** 
	 * Definitions of colours, according to the ANSI escape functionalities.
	 * (The ANSI escape sequences are used to change the colour of the text in the console.)
	 * To use this, the console must support ANSI escape sequences.
	 */
	#define COLOR_RED( text ) 			"\033[1;31m" text "\033[0m"
	#define COLOR_GREEN( text ) 		"\033[1;32m" text "\033[0m"
	#define COLOR_YELLOW( text ) 		"\033[1;33m" text "\033[0m"
	#define COLOR_BLUE( text ) 			"\033[1;34m" text "\033[0m"
	#define COLOR_MAGENTA( text ) 		"\033[1;35m" text "\033[0m"
	#define COLOR_CYAN( text ) 			"\033[1;36m" text "\033[0m"

	#define COLOR_PURPLE( text ) 		"\033[1;38;5;89m" text "\033[0m"
	#define COLOR_PINK( text ) 			"\033[1;38;5;204m" text "\033[0m"

	/** Samples of common TAGs */
	#define DEBUG_TAG 					TAG_BRACKETS( COLOR_CYAN  ( "DEBUG"   ) )
	#define SUCCESS_TAG 				TAG_BRACKETS( COLOR_GREEN ( "SUCCESS" ) )
	#define FAIL_TAG					TAG_BRACKETS( COLOR_YELLOW( "FAIL"    ) )
	#define ERROR_TAG					TAG_BRACKETS( COLOR_RED   ( "ERROR"   ) )

	/**
	 * This macro is used to return errors in the library.
	*/
	#define _DEBUG_ERROR()				printf("ERRORE: impossibile utilizzare questa funzionalità di debug al momento.\n")


	// Checks for the debug mode

	/**
	 * This macro expands to the code that must be executed if the debug mode is active.
	 * Otherwise, the macro expands to nothing.
	 * 
	 * It is programmer's duty to ensure that the code inside the macro is valid even if the debug mode is not active.
	 */
	#ifdef DEBUG_MODE
		#define IF_DEBUG_ACTIVE( ... ) __VA_ARGS__ 		// This expands the code
	#else
		#define IF_DEBUG_ACTIVE( ... ) 					// This expands to nothing
	#endif

	// Check for the C++ version
	#if __cplusplus <= 201703 && defined __GNUC__ && !defined __clang__ && !defined __EDG__
		#define VA_OPT_UNSUPPORTED
	#endif

	#ifdef VA_OPT_UNSUPPORTED
		#define DEBUG_LOG( message, ...) IF_DEBUG_ACTIVE( _DEBUG_ERROR() )
		#define DEBUG_LOG_SUCCESS( message, ...) IF_DEBUG_ACTIVE( _DEBUG_ERROR() )
		#define DEBUG_LOG_FAIL( message, ...) IF_DEBUG_ACTIVE( _DEBUG_ERROR() )
		#define DEBUG_LOG_ERROR( message, ...) IF_DEBUG_ACTIVE( _DEBUG_ERROR() )
		#define DEBUG_MARK_PHASE( phase_name, ...) IF_DEBUG_ACTIVE( _DEBUG_ERROR(); )
	#else

	/**
	 * This macro prints a log message in the console, if the debug mode is active.
	 * The log contains the file name, the line number and the custom message, plus the variable arguments.
	 * The printing of the message and the arguments work as in the *printf* function.
	 *
	 * Notes:
	 * __FILE__ expands to the name of the file where the macro is called.
	 * __LINE__ expands to the line number where the macro is called, as an unsigned integer.
	 * __VA_ARGS__ expands to the variable arguments.
	 * __VA_OPT__(x) is a macro that expands to x if there are variable arguments, otherwise it expands to nothing.
	 */
	#define DEBUG_LOG( message, ...) 																				\
		IF_DEBUG_ACTIVE(																							\
			printf( DEBUG_TAG " %s(%u) : " message "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__ )				\
		)

	/**
	 * This macro prints a success message in the console, if the debug mode is active.
	 * The message contains the file name, the line number and the custom message, plus the variable arguments.
	 * For further info, see the macro DEBUG_LOG.
	 */
	#define DEBUG_LOG_SUCCESS( message, ...) 														\
		DEBUG_LOG( SUCCESS_TAG " " message __VA_OPT__(,) __VA_ARGS__ )

	/**
	 * This macro prints a fail message in the console, if the debug mode is active.
	 * The message contains the file name, the line number and the custom message, plus the variable arguments.
	 * For further info, see the macro DEBUG_LOG.
	 */
	#define DEBUG_LOG_FAIL( message, ...) 															\
		DEBUG_LOG( FAIL_TAG " " message __VA_OPT__(,) __VA_ARGS__ )

	/**
	 * This macro prints an error message in the console, if the debug mode is active.
	 * The message contains the file name, the line number and the custom message, plus the variable arguments.
	 * For further info, see the macro DEBUG_LOG.
	 */
	#define DEBUG_LOG_ERROR( message, ...) 															\
		DEBUG_LOG( ERROR_TAG " " message __VA_OPT__(,) __VA_ARGS__ )

	/**
	 * This macro is used to mark the beginning and the end of a new phase of the program, with two log messages.
	 * If both the messages are printed, the phase is completed correctly.
	 *
	 * Usage example:
	 *
	 *		DEBUG_MARK_PHASE( name_of_the_phase_to_mark , with , optional , parameters ) {
	 *			...
	 * 			statements
	 *			...
	 *		}
	 *
	 * Note: the macro defines a new scope; therefore, the statements inside the macro must be enclosed in curly brackets.
	 * If no curly brackets are used, the macro will signal the completion only of the next single statement.
	 * 
	 * This macro can be nested inside itself, with multiple block inside one another. In this case, the curly brackets are mandatory.
	 * However, it is not possible to call this macro multiple times *on the same line* (because it uses the __LINE__ macro function to
	 * generate the name of the variables, and the variables must be different for each call).
	 */
	#define DEBUG_MARK_PHASE( phase_name, ... )														\
		IF_DEBUG_ACTIVE(																			\
			_DEBUG_MARK_PHASE( UNIQUE_ID, phase_name __VA_OPT__(,) __VA_ARGS__ )					\
		)

	#define _DEBUG_MARK_PHASE( counter_id, phase_name, ... )										\
		int counter_id = 0;																			\
		for ( 																						\
				DEBUG_ENTERING_PHASE( phase_name __VA_OPT__(,) __VA_ARGS__ );						\
				counter_id < 1; 																	\
				DEBUG_EXITING_PHASE( phase_name __VA_OPT__(,) __VA_ARGS__ ), counter_id++			\
			)
	/**
	 * Comments:
	 * The macro uses a for loop, with a single iteration, to define a new scope. This allows to execute
	 * the phase marking at the beginning and at the end of the scope.
	 * For the cycle to be executed once, it must be "transparent" from outside, so a generated variable is used.
	 */

	#define DEBUG_ENTERING_PHASE( phase_name, ... )													\
		DEBUG_LOG_SUCCESS( "Entering phase [%s] \"" COLOR_MAGENTA( phase_name ) "\"", debugAcquireTicket().c_str() __VA_OPT__(,) __VA_ARGS__ )

	#define DEBUG_EXITING_PHASE( phase_name, ... )													\
		DEBUG_LOG_SUCCESS( "Exiting phase  [%s] \"" COLOR_MAGENTA( phase_name ) "\"", debugReleaseTicket().c_str() __VA_OPT__(,) __VA_ARGS__ )

	#endif


	/**
	 * This macro function accepts a variable name as argument.
	 * It prints an information message indicating if the variable is NULL or NOT NULL.
	 * It expects a NULL variable, so the the printed message is green if the value is NULL, red otherwise.
	 */
	#define DEBUG_ASSERT_NULL( variable_id ) 														\
		if (variable_id == NULL) {																	\
			DEBUG_LOG_SUCCESS( "Variable \"%s\" == NULL, expected NULL", #variable_id );			\
		} else {																					\
			DEBUG_LOG_FAIL( "Variable \"%s\" == NOT NULL, expected NULL", #variable_id );			\
		}

	/**
	 * This macro function accepts a variable name as argument.
	 * It prints an information message indicating if the variable is NULL or NOT NULL.
	 * It expects a NOT NULL variable, so the the printed message is green if the value is NOT NULL, red otherwise.
	 */
	#define DEBUG_ASSERT_NOT_NULL( variable_id )													\
		if (variable_id != NULL) {																	\
			DEBUG_LOG_SUCCESS( "Variable \"%s\" == NOT NULL, expected NOT NULL", #variable_id );	\
		} else {																					\
			DEBUG_LOG_FAIL( "Variable \"%s\" == NULL, expected NOT NULL", #variable_id );			\
		}

	/**
	 * This macro function accepts a condition as argument.
	 * It prints an information message indicating if the condition is true or false.
	 * It expects a true condition, so the the printed message is green if the condition is true, red otherwise.
	 */
	#define DEBUG_ASSERT_TRUE( condition ) 															\
		if (condition) {																			\
			DEBUG_LOG_SUCCESS( "Condition (%s) == TRUE, expected TRUE", #condition );				\
		} else {																					\
			DEBUG_LOG_FAIL( "Condition (%s) == FALSE, expected TRUE", #condition );					\
		}

	/**
	 * This macro function accepts a condition as argument.
	 * It prints an information message indicating if the condition is true or false.
	 * It expects a false condition, so the the printed message is green if the condition is false, red otherwise.
	 */
	#define DEBUG_ASSERT_FALSE( condition )															\
		if (!(condition)) {																			\
			DEBUG_LOG_SUCCESS( "Condition (%s) == FALSE, expected FALSE", #condition );				\
		} else {																					\
			DEBUG_LOG_FAIL( "Condition (%s) == TRUE, expected FALSE", #condition );					\
		}

	/**
	 * This macro function allows to wait the user to press a key before continuing.
	 * It is useful to pause the execution of the program, to allow the user to read the printed messages.
	 * 
	 * Note: it uses the getchar() function, so -if the user inserts multiple characters- only the first one is read.
	 * The others characters might interfere with the next getchar() call.
	 */
	#define DEBUG_WAIT_USER_ENTER()																	\
		if (WAIT_FOR_USER) {																		\
			DEBUG_LOG( COLOR_RED( "Press ENTER..." ) );												\
			IF_DEBUG_ACTIVE(getchar();)																\
		}


	 // Function prototypes

	 using std::string;

	 string debugAcquireTicket();
	 string debugReleaseTicket();

}

#endif /* INCLUDE_DEBUG_HPP_ */
