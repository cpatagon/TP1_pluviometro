/*
 * debounce.c
 *
 *  Created on: 18-07-2023
 *      @Author: Luis Gómez
 *      @Hardware: STM32F429ZI
 *      @Objective: Implement a program that toggles the LED2 frequency
 *      between 100 ms and 500 ms each time the key is pressed. The
 *      program should use the debounce functions of the API debounce module
 *      and the non-blocking delays of the API_delay module.
 */
#include "mbed.h"
#include "stm32f4xx_hal.h"  		/* <- HAL include */

#include <assert.h>
#include "debounce.h"



// Define el botón usando la API de Mbed
static DigitalIn userButton(BUTTON1, PullUp);

/*
 * Definition of the states for the state machine
 */

typedef enum{
	BUTTON_UP,       // Button released
	BUTTON_FALLING,  // Button being pressed
	BUTTON_DOWN,     // Button pressed
	BUTTON_RAISING,  // Button being released
} debounceState_t;

// Global and private state variable declaration
static debounceState_t  currentState;

/*
 * Declare a global private variable of type bool_t that
 * is set to true when a falling edge occurs
 * and is set to false when the readKey() function is called.
 */
static bool_t PressButton=true;

/*
 * Function that changes from true to false or from false to true
 * if the button is pressed without bouncing.
 */
bool_t readKey() {
    bool_t temp = PressButton;
    PressButton = false;
    return temp;
}

static void buttonPressed(void);

/*
 * @brief   Initializes the FSM with the defined value
 *          at the start of the model.
 *
 * @param   None
 * @retval  None
 */
void debounceFSM_init(){
	/* Initialize Estado */
	assert(&PressButton!=NULL);
	currentState=BUTTON_UP;
}

/*
 * @brief   Updates the debounce FSM.
 *          This function reads the inputs, evaluates the transition conditions according to the FSM's current state,
 *          and updates the current state and outputs accordingly.
 *
 * @param   delay: pointer to the delay instance
 * @retval  None
 */
void debounceFSM_update(delay_t* delay) {
    assert(delay != NULL);
    assert(&currentState != NULL);

    switch (currentState) {
    case BUTTON_UP:
        if (userButton.read()) {
            currentState = BUTTON_FALLING;
        }
        break;
    case BUTTON_FALLING:
        if (userButton.read() && delayRead(delay)) {
            currentState = BUTTON_DOWN;
            buttonPressed();
        } else {
            currentState = BUTTON_UP;
        }
        break;
    case BUTTON_DOWN:
        if (!userButton.read()) {
            currentState = BUTTON_RAISING;
        }
        break;
    case BUTTON_RAISING:
        if (!userButton.read() && delayRead(delay)) {
            currentState = BUTTON_UP;
        } else {
            currentState = BUTTON_DOWN;
        }
        break;
    default:
        assert(0);
    }
}

/*
 * @brief   Toggles the state of LED2.
 *
 * @param   None
 * @retval  None
 */
static void buttonPressed(){
          PressButton = true;
}

/*función que invierte  el estado del LED3 */
/*static void buttonReleased()
{
}
*/

