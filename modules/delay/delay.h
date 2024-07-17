#include <stdint.h>   /**< Para incluir los tipos uint32_t */
#include <stdbool.h>  /**< Para incluir los tipos bool (booleanos) */

#include "stm32f4xx_hal.h"         /**< Inclusión de HAL */


typedef uint32_t tick_t;  /**< Definición de tipo para los ticks */
typedef bool bool_t;      /**< Definición de tipo para los valores booleanos */

typedef struct {
    tick_t startTime;  /**< Tiempo de inicio del retardo */
    tick_t duration;   /**< Duración del retardo en ticks */
    bool_t running;    /**< Estado del retardo (activo/inactivo) */
} delay_t;

/**
 * @brief Inicializa una estructura de retardo con una duración específica.
 *
 * Esta función inicializa la estructura de retardo con la duración especificada
 * y establece el estado del retardo como no activo (running = false).
 *
 * @param delay Puntero a la estructura de retardo.
 * @param duration Duración del retardo en ticks.
 */
void delayInit(delay_t *delay, tick_t duration);

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
bool_t delayRead(delay_t *delay);

/**
 * @brief Modifica la duración de un retardo existente.
 *
 * Esta función permite cambiar dinámicamente la duración de un retardo ya inicializado.
 *
 * @param delay Puntero a la estructura de retardo.
 * @param duration Nueva duración del retardo en ticks.
 */
void delayWrite(delay_t *delay, tick_t duration);




