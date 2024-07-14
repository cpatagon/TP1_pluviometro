/*
 * Nombre del archivo: pluviometer.c
 * Descripción: [Breve descripción del archivo]
 * Autor: Luis Gómez P.
 * Derechos de Autor: (C) 2023 [Tu nombre o el de tu organización]
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
 ** @brief 
 **/

/* === Headers files inclusions =============================================================== */



/* === Macros definitions ====================================================================== */


/* === Private data type declarations ========================================================== */

/* === Private variable declarations =========================================================== */

DigitalIn tickRain(SWITCH_TICK_RAIN);  ///< Botón de detección de lluvia


int rainfallCount = RAINFALL_COUNT_INI;  ///< Contador de lluvia
int lastMinute = LAST_MINUTE_INI;  ///< Último minuto

/* === Private function declarations =========================================================== */


void analyzeRainfall();
void accumulateRainfall();
void printRain(const char* buffer);
void printAccumulatedRainfall();

/* === Public variable definitions ============================================================= */


/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

/**
 * @brief Analiza la lluvia detectada
 * 
 * Imprime la hora actual y acumula la lluvia detectada.
 */
void analyzeRainfall() {
    const char* currentTime = DateTimeNow();
    printRain(currentTime);
    accumulateRainfall();
    thread_sleep_for(DELAY_BETWEEN_TICK);
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
    
    int rainfallInteger = (int)(rainfallCount * MM_PER_TICK);
    int rainfallDecimal = (int)((rainfallCount * MM_PER_TICK - rainfallInteger) * 100);
    
    char buffer[100];
    int len = sprintf(buffer, "%s%s%d.%02d mm\n", 
                      dateTime, MSG_ACCUMULATED_RAINFALL, rainfallInteger, rainfallDecimal);
    
    pc.write(buffer, len);
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
}

/**
 * @brief Verifica si está lloviendo
 * 
 * @return true si el botón de detección de lluvia está activado, false en caso contrario
 */
bool isRaining() {
    return (tickRain == ON);
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
