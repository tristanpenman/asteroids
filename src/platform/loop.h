#ifndef ASTEROIDS_PLATFORM_LOOP_H
#define ASTEROIDS_PLATFORM_LOOP_H

#include "types.h"

typedef void (*main_loop_fn_t)(bool draw);

/**
 * Set the function that will be called on the next iteration of the event loop
 *
 * @param  main_loop_fn  pointer to the function to be called
 */
void loop_set_main(main_loop_fn_t main_loop_fn);

/**
 * Start running the main event loop
 *
 * Execution will continue until loop_cancel is called and the final
 * iteration is complete, at which point the program will terminate.
 */
void loop_run(void);

/**
 * Cancel next iteration of the event loop
 *
 * Execution will continue until the current iteration of the event loop has
 * completed, at which point the program will terminate.
 *
 * @param exit_code exit code that be returned to OS when progrma terminates
 */
void loop_cancel(int exit_code);

#endif
