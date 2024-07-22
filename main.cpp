#include "mbed.h"
#include "pluviometer.h"

// Pines de la placa Nucleo F429ZI
#define BUTTON_PIN USER_BUTTON
#define LED_PIN LED1
#define TX_PIN USBTX
#define RX_PIN USBRX

// Definición del pluviómetro
Pluviometer pluvio;

// Función principal
int main() {
    // Inicializar el pluviómetro
    Pluviometer_init(&pluvio, BUTTON_PIN, LED_PIN, TX_PIN, RX_PIN);

    // Configurar el intervalo de reporte a 1 minuto (por defecto ya está configurado a 1 minuto)
    Pluviometer_set_report_interval(&pluvio, 1);

    // Configurar la hora actual del sistema (año, mes, día, hora, minuto, segundo)
    struct tm current_time;
    current_time.tm_year = 2023 - 1900;  // Año actual - 1900
    current_time.tm_mon = 6 - 1;         // Mes (0-11, donde 0 es enero)
    current_time.tm_mday = 1;            // Día del mes
    current_time.tm_hour = 16;           // Hora (24 horas)
    current_time.tm_min = 0;             // Minuto
    current_time.tm_sec = 0;             // Segundo
    time_t epoch_time = mktime(&current_time);
    Pluviometer_set_current_time(epoch_time);

    // Bucle principal
    while (1) {
        // Actualizar el estado del pluviómetro
        Pluviometer_update(&pluvio);

        // Esperar un pequeño tiempo antes de la próxima actualización
        ThisThread::sleep_for(100ms);
    }
}