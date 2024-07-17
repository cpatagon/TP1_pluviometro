/** 
 * @file pluviómetro.cpp
 * @brief Proyecto pluviómetro
 * 
 * Este archivo contiene la implementación del proyecto pluviómetro 2 utilizando MBED.
 */

#include "mbed.h"
#include "arm_book_lib.h"
#include "pluviometer.h"

#define RAINFALL_CHECK_INTERVAL 60  ///< Intervalo de verificación de lluvia en segundos


// Sensores
bool hasTimePassedMinutesRTC(int minutes);



int main()
{
    initializeSensors();
    initializeDebounce();

    while (true) {
      updateDebounce();
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




