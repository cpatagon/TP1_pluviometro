/**
 * @Autor Luis Gómez
 * @file pluviómetro.cpp
 * @brief Proyecto pluviómetro
 * 
 * Este archivo contiene la implementación del proyecto pluviómetro 2 utilizando MBED.
 */

#include "mbed.h"
#include "arm_book_lib.h"
#include "pluviometer.h"

#define RAINFALL_CHECK_INTERVAL 60  ///< Intervalo de verificación de lluvia en segundos


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





