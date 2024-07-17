#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include "mbed.h"

#include <stdint.h>  /* For standard uint32_t types */
#include <stdbool.h> /* For standard boolean types */

#include "stm32f4xx_hal.h"          /* HAL library inclusion */


#include "delay.h" /* Inclusion of the file defining delay_t */

typedef uint32_t tick_t; // Which library should be included for this to compile?
typedef bool bool_t; // Which library should be included for this to compile?

/**
 * @brief   Reads an internal variable and returns whether the key was pressed.
 *          If the function returns true, it resets the state of the variable (sets it to false).
 *
 * @param   None
 * @retval  bool_t: the state of the key (true if the key was pressed, false otherwise)
 */
bool_t readKey();

/**
 * @brief   Initializes the debounce FSM, setting the initial state.
 *
 * @param   None
 * @retval  None
 */
void debounceFSM_init();

/**
 * @brief   Updates the debounce FSM.
 *          This function reads the inputs, evaluates the transition conditions according to the FSM's current state,
 *          and updates the current state and outputs accordingly.
 *
 * @param   delay: pointer to the delay instance
 * @retval  None
 */
void debounceFSM_update(delay_t* delay);
#endif // DEBOUNCE_H

