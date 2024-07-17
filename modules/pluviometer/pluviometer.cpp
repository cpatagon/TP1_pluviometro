/*
 * Nombre del archivo: pluviometer.c
 * Descripción: Implementación de la biblioteca para la detección y reporte de lluvias.
 * Autor: Luis Gómez P.
 * Derechos de Autor: (C) 2023 Luis Gómez P.
 * Licencia: GNU General Public License v3.0
 * 
 * Este programa es software libre: puedes redistribuirlo y/o modificarlo
 * bajo los términos de la Licencia Pública General GNU publicada por
 * la Free Software Foundation, ya sea la versión 3 de la Licencia, o
 * (a tu elección) cualquier versión posterior.
 * 
 * Este programa se distribuye con la esperanza de que sea útil,
 * pero SIN NINGUNA GARANTÍA; sin siquiera la garantía implícita
 * de COMERCIABILIDAD o APTITUD PARA UN PROPÓSITO PARTICULAR. Ver la
 * Licencia Pública General GNU para más detalles.
 * 
 * Deberías haber recibido una copia de la Licencia Pública General GNU
 * junto con este programa. Si no es así, visita <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

/** @file
 ** @brief Implementación de la biblioteca para la detección y reporte de lluvias.
 **/

/* === Headers files inclusions =============================================================== */
#include "mbed.h"
#include "arm_book_lib.h"
#include "debounce.h"
#include "pluviometer.h"

/* === Macros definitions ====================================================================== */


/* === Private data type declarations ========================================================== */

/* === Private variable declarations =========================================================== */

DigitalOut alarmLed(LED1);
DigitalOut tickLed(LED2);
DigitalIn tickRain(SWITCH_TICK_RAIN);  ///< Botón de detección de lluvia

int rainfallCount = RAINFALL_COUNT_INI;  ///< Contador de lluvia
int lastMinute = LAST_MINUTE_INI;  ///< Último minuto

static delay_t debounceDelay;
static bool buttonPressed = false;
static delay_t analyzeDelay;


/* === Private function declarations =========================================================== */

// Análisis de Datos
void analyzeRainfall();
void accumulateRainfall();
bool hasTimePassedMinutesRTC(int waiting_seconds);

// Actuación 
void printRain(const char* buffer);
void printAccumulatedRainfall();
const char* DateTimeNow(void);

// Variables globales
BufferedSerial pc(USBTX, USBRX, BAUD_RATE);  ///< Comunicación serial




/* === Private function implementation ========================================================= */

void initializeDebounce() {
    debounceFSM_init();
    delayInit(&debounceDelay, 40);  // 40 ms de debounce
}


void updateDebounce(){
  debounceFSM_update(&debounceDelay);
}


/**
 * @brief Analiza la lluvia detectada
 * 
 * Imprime la hora actual y acumula la lluvia detectada.
 */
void analyzeRainfall() {
  static bool analyzing = false;

    if (!analyzing) {
        // Comenzar el análisis
        const char* currentTime = DateTimeNow();
        printRain(currentTime);
        accumulateRainfall();
        analyzing = true;
        delayWrite(&analyzeDelay, DELAY_BETWEEN_TICK);  // Reinicia el delay
    } else if (delayRead(&analyzeDelay)) {
        // El delay ha terminado
        analyzing = false;
    }
}

/**
 * @brief Acumula la cantidad de lluvia detectada
 */
void accumulateRainfall() {
    rainfallCount++;
}

/**
 * @brief Imprime un mensaje de detección de lluvia
 * 
 * @param buffer Cadena de caracteres con la hora actual
 */
void printRain(const char* buffer) {
    pc.write(buffer, strlen(buffer));
    pc.write(MSG_RAIN_DETECTED, strlen(MSG_RAIN_DETECTED));
}

/**
 * @brief Imprime la cantidad de lluvia acumulada
 * 
 * Calcula e imprime la cantidad de lluvia acumulada en el formato "YYYY-MM-DD HH:MM - Accumulated rainfall: X.XX mm".
 */
void printAccumulatedRainfall() {
    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    char dateTime[80];
    strftime(dateTime, sizeof(dateTime), DATE_FORMAT, timeinfo);

    // Calcular la lluvia acumulada en décimas de mm
    int accumulatedRainfall = rainfallCount * MM_PER_TICK; // MM_PER_TICK es ahora 0.1 para décimas de mm

    // Preparar el buffer para imprimir
    char buffer[100];
    int len = sprintf(buffer, "%s%s%d.%01d mm\n", 
                      dateTime, MSG_ACCUMULATED_RAINFALL, accumulatedRainfall / 10, accumulatedRainfall % 10);
    
    // Imprimir el resultado
    pc.write(buffer, len);
}

/**
 * @brief Obtiene la fecha y hora actual
 * 
 * @return Cadena de caracteres con la fecha y hora actual en formato "%Y-%m-%d %H:%M:%S"
 */
const char* DateTimeNow() {
    time_t seconds = time(NULL);
    static char bufferTime[80];
    strftime(bufferTime, sizeof(bufferTime), TIME_FORMAT, localtime(&seconds));
    return bufferTime;
}

/* === Public function implementation ========================================================== */

/**
 * @brief Inicializa los sensores
 * 
 * Configura el modo del botón de detección de lluvia y apaga los LEDs.
 */
void initializeSensors() {
    tickRain.mode(PullDown);
    alarmLed = OFF;
    tickLed = OFF;
    set_time(TIME_INI); ///< Configurar la fecha y hora inicial
    delayInit(&analyzeDelay, DELAY_BETWEEN_TICK);
}

/**
 * @brief Verifica si está lloviendo
 * 
 * @return true si el botón de detección de lluvia está activado, false en caso contrario
 */
bool isRaining() {
    return readKey();
}

/**
 * @brief Actúa en base a la detección de lluvia
 * 
 * Enciende los LEDs de alarma y tick, y analiza la lluvia detectada.
 */
void actOnRainfall() {
    alarmLed = ON;
    tickLed = ON;
    analyzeRainfall();
}

/**
 * @brief Reporta la lluvia acumulada
 * 
 * Imprime la cantidad de lluvia acumulada y resetea el contador de lluvia.
 */
void reportRainfall() {
    printAccumulatedRainfall();
    rainfallCount = RAINFALL_COUNT_INI;
}

/* === End of documentation ==================================================================== */
