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
#include "stm32f4xx_hal.h"  		/* <- HAL include */
#include "stm32f4xx_nucleo_144.h" 	/* <- BSP include */

#include <assert.h>
#include "debounce.h"

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
bool_t readKey(){
    return PressButton;
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
	return;
}

/*
 * @brief   Updates the debounce FSM.
 *          This function reads the inputs, evaluates the transition conditions according to the FSM's current state,
 *          and updates the current state and outputs accordingly.
 *
 * @param   delay: pointer to the delay instance
 * @retval  None
 */
void debounceFSM_update(delay_t* delay){
	assert(delay!=NULL);
	assert(&currentState!=NULL);

	switch (currentState){
	/*
	 * In the BUTTON_UP state, it checks whether the button remains unpressed.
	 * Otherwise, that is, if the button is pressed, the state changes to BUTTON_FALLING.
	 */
	case BUTTON_UP:
		if (BSP_PB_GetState(BUTTON_USER)){
	    	currentState=BUTTON_FALLING;
		}
		break;
		/*
		 * In the BUTTON_FALLING state, it checks if the button remains pressed for 40 ms.
		 * If it remains pressed (condition 'yes'), the state changes to BUTTON_DOWN and the
		 * state of LED1 is toggled. If the button does not remain pressed (condition 'no'),
		 * the state changes back to BUTTON_UP, interpreting this event as a bounce.
		 */
	case BUTTON_FALLING:
		if (BSP_PB_GetState(BUTTON_USER) && delayRead(delay)){
	    	currentState=BUTTON_DOWN;
	    	buttonPressed();
	    	//PressButton = !(PressButton);
		}
		else {
			currentState=BUTTON_UP;
		}
		break;
	/*
	 * In the BUTTON_DOWN state, if the button is released, the state changes to BUTTON_RAISING.
	 */
	case BUTTON_DOWN:
		if (!BSP_PB_GetState(BUTTON_USER)){
	    	currentState=BUTTON_RAISING;
		}
		break;
	/*
	 * In the BUTTON_RAISING state, it checks if the button remains unpressed for a certain delay.
	 * If it remains unpressed (condition 'yes'), the state changes back to BUTTON_UP and
	 * the buttonReleased function is called. If the button is pressed again (condition 'no'),
	 * the state changes back to BUTTON_DOWN.
	 */
	case BUTTON_RAISING:
		if (!BSP_PB_GetState(BUTTON_USER) && delayRead(delay)){
	    	currentState=BUTTON_UP;
	    	//buttonReleased();
		}
		else {
			currentState=BUTTON_DOWN;
		}
		break;
	default:
			/* Handle unexpected state */
		assert(0);
	}
	return;
}

/*
 * @brief   Toggles the state of LED2.
 *
 * @param   None
 * @retval  None
 */
static void buttonPressed(){
	PressButton = !(PressButton);
	return;
}

/*función que invierte  el estado del LED3 */
/*static void buttonReleased()
{
}
*/

