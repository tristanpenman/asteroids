#ifndef ASTEROIDS_PLATFORM_LOOP_H
#define ASTEROIDS_PLATFORM_LOOP_H

#include "types.h"

typedef void (*main_loop_fn_t)(bool draw);
typedef void (*loop_exit_handler_fn_t)(void);

/**
 * Set the function that will be called on the next iteration of the event loop
 *
 * @param  main_loop_fn  pointer to the function to be called
 */
void loop_set_main(main_loop_fn_t main_loop_fn);

/**
 * Start running the main event loop
 *
 * Execution continues until loop_cancel is called. Synchronous platforms
 * return the requested exit code after the current iteration completes.
 */
int loop_run(void);

/** Set a handler for asynchronous event-loop exit (used by browser builds). */
void loop_set_exit_handler(loop_exit_handler_fn_t handler);

/**
 * Cancel next iteration of the event loop
 *
 * Execution continues until the current iteration of the event loop has
 * completed, at which point loop_run returns the requested code.
 *
 * @param exit_code exit code returned by loop_run
 */
void loop_cancel(int exit_code);

#endif
