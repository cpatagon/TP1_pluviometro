/** 
 * @file pluviómetro.cpp
 * @brief Proyecto pluviómetro
 * 
 * Este archivo contiene la implementación del proyecto pluviómetro 2 utilizando MBED.
 */

#include "mbed.h"
#include "arm_book_lib.h"
#include "pluviometer.h"


// Sensores
bool hasTimePassedMinutesRTC(int minutes);

// Actuación
const char* DateTimeNow(void);


int main()
{
    initializeSensors();

    while (true) {
        if (isRaining()) {
            actOnRainfall();
        } else {
            alarmLed = OFF;
            tickLed = OFF;
        }

        if (hasTimePassedMinutesRTC(RAINFALL_CHECK_INTERVAL)) {
            reportRainfall();
        }
    }
}


/**
 * @brief Verifica si ha pasado el tiempo especificado en minutos
 * 
 * @param minutes Cantidad de minutos a verificar
 * @return true si ha pasado el tiempo especificado, false en caso contrario
 */
bool hasTimePassedMinutesRTC(int waiting_seconds) {
    static time_t lastTime = 0;
    time_t currentTime;

    // Obtener el tiempo actual del RTC
    currentTime = rtc_read();

    // Calcular la diferencia de tiempo en segundos
    time_t timeDifference = currentTime - lastTime;

    // Verificar si ha pasado el tiempo especificado
    if (timeDifference >= waiting_seconds) {
        lastTime = currentTime;
        return true;
    }

    return false;
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
