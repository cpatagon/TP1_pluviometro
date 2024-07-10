/** 
 * @file pluviómetro.cpp
 * @brief Proyecto pluviómetro
 * 
 * Este archivo contiene la implementación del proyecto pluviómetro 2 utilizando MBED.
 */

#include "mbed.h"
#include "arm_book_lib.h"
#include "weather_station.h"

// Constantes de configuración
#define TIME_INI  1593561600  ///< 1 de julio de 2020, 00:00:00
#define BAUD_RATE 9600  ///< Velocidad de comunicación serial
#define DELAY_BETWEEN_TICK 500  ///< 500 ms
#define SWITCH_TICK_RAIN BUTTON1  ///< Botón para detectar lluvia
#define RAINFALL_CHECK_INTERVAL 1  ///< Intervalo de verificación de lluvia en minutos
#define MM_PER_TICK 0.2f  ///< 0.2 mm de agua por tick
#define RAINFALL_COUNT_INI 0  ///< Contador de lluvia inicial
#define LAST_MINUTE_INI -1  ///< Último minuto inicial

// Mensajes y formatos
#define MSG_RAIN_DETECTED " - Rain detected\r\n"  ///< Mensaje de lluvia detectada
#define MSG_ACCUMULATED_RAINFALL " - Accumulated rainfall: "  ///< Mensaje de lluvia acumulada
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"  ///< Formato de tiempo completo
#define DATE_FORMAT "%Y-%m-%d %H:%M"  ///< Formato de fecha y hora

// Sensores
void initializeSensors();
bool isRaining();

// Análisis de Datos
void analyzeRainfall();
void accumulateRainfall();
bool hasTimePassedMinutesRTC(int minutes);

// Actuación
void actOnRainfall();
void reportRainfall();
void printRain(const char* buffer);
const char* DateTimeNow(void);
void printAccumulatedRainfall();

// Variables globales
BufferedSerial pc(USBTX, USBRX, BAUD_RATE);  ///< Comunicación serial

DigitalOut alarmLed(LED1);  ///< LED de alarma
DigitalIn tickRain(SWITCH_TICK_RAIN);  ///< Botón de detección de lluvia
DigitalOut tickLed(LED2);  ///< LED de tick

int rainfallCount = RAINFALL_COUNT_INI;  ///< Contador de lluvia
int lastMinute = LAST_MINUTE_INI;  ///< Último minuto

/**
 * @brief Función principal
 * 
 * Esta función inicializa los sensores y entra en un bucle infinito para detectar lluvia y reportar la lluvia acumulada.
 */
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
 * @brief Verifica si ha pasado el tiempo especificado en minutos
 * 
 * @param minutes Cantidad de minutos a verificar
 * @return true si ha pasado el tiempo especificado, false en caso contrario
 */
bool hasTimePassedMinutesRTC(int minutes) {
    static time_t lastTime = 0;
    time_t currentTime = time(NULL);

    if (difftime(currentTime, lastTime) >= minutes * 60) {
        lastTime = currentTime;
        return true;
    }

    return false;
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
