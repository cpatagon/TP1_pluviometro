/*
 * API_delay.c
 *
 *  Created on: 06-07-2023
 *      Author: lgomez
 */

/**
 * @file delay.c
 * @brief Implementación de funciones para gestionar retardos en microcontroladores.
 */

#include "stm32f4xx_hal.h"         /**< Biblioteca HAL */


#include <assert.h>
#include "delay.h"

/**
 * @brief Inicializa una estructura de retardo con una duración específica.
 *
 * Esta función inicializa la estructura de retardo con la duración especificada
 * y establece el estado del retardo como no activo (running = false).
 *
 * @param delay Puntero a la estructura de retardo.
 * @param duration Duración del retardo en ticks.
 */
void delayInit(delay_t *delay, tick_t duration)
{
    assert(delay != NULL);
    assert(duration >= 0);

    delay->duration = duration;
    delay->running = false;  // Inicializa el retardo como no activo
}

/**
 * @brief Verifica y gestiona el estado de un retardo.
 *
 * Esta función verifica el estado del retardo:
 * - Si el retardo no está activo (running = false), guarda el tiempo de inicio
 *   y activa el retardo (running = true).
 * - Si el retardo está activo (running = true), verifica si el tiempo transcurrido
 *   desde el inicio es mayor o igual a la duración especificada. En caso afirmativo,
 *   desactiva el retardo y retorna true indicando que el tiempo de retardo ha sido
 *   cumplido.
 *
 * @param delay Puntero a la estructura de retardo.
 * @return true si el tiempo de retardo ha sido cumplido, false en caso contrario.
 */
bool_t delayRead(delay_t *delay)
{
    static bool_t retValue = false;

    assert(delay != NULL);
    assert(delay->duration >= 0);

    if (delay->running == false) {
        // Si el retardo no está corriendo, guarda el tiempo de inicio y lo activa
        delay->startTime = HAL_GetTick();
        delay->running = true;
    } else {
        // Si el retardo está corriendo, verifica si se ha cumplido la duración
        if ((HAL_GetTick() - delay->startTime) >= delay->duration) {
            delay->running = false;  // Desactiva el retardo
            retValue = true;         // Indica que el tiempo de retardo ha sido cumplido
        }
    }

    return retValue;
}

/**
 * @brief Modifica la duración de un retardo existente.
 *
 * Esta función permite cambiar dinámicamente la duración de un retardo ya inicializado.
 *
 * @param delay Puntero a la estructura de retardo.
 * @param duration Nueva duración del retardo en ticks.
 */
void delayWrite(delay_t *delay, tick_t duration)
{
    assert(delay != NULL);
    assert(duration >= 0);

    delay->duration = duration;  // Actualiza la duración del retardo
}

