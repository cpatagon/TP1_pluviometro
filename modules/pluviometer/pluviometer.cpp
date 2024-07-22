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
#include "pluviometer.h"

// Prototipos de funciones internas
static void detect_rain(Pluviometer* pluvio);
static void accumulate_rain(Pluviometer* pluvio);
static void report_rain(Pluviometer* pluvio);
static void button_pressed(Pluviometer* pluvio);
static void debounce_finished(Pluviometer* pluvio);
static void print_report(Pluviometer* pluvio);

// Función de inicialización
void Pluviometer_init(Pluviometer* pluvio, PinName button_pin, PinName led_pin, PinName tx_pin, PinName rx_pin) {
    pluvio->state = INACTIVO;
    pluvio->report_interval = 1; // Por defecto 1 minuto
    pluvio->tick_count = 0;
    pluvio->accumulated_rain_tenths_mm = 0;
    pluvio->debounce_active = false;
    pluvio->led = new DigitalOut(led_pin);
    pluvio->button = new InterruptIn(button_pin);
    pluvio->button->fall(callback(button_pressed, pluvio));
    pluvio->rain_timer.start();
    pluvio->serial = new BufferedSerial(tx_pin, rx_pin, 9600);
    pluvio->event_queue = mbed_event_queue();
    pluvio->event_thread = new Thread(osPriorityNormal, 1024);
    pluvio->event_thread->start(callback(pluvio->event_queue, &EventQueue::dispatch_forever));
    time(&pluvio->last_report_time); // Capturar el tiempo de inicio
    pluvio->ticker.attach(callback(report_rain, pluvio), std::chrono::minutes(1)); // Llamar cada minuto
}

// Función para actualizar el estado del pluviómetro
void Pluviometer_update(Pluviometer* pluvio) {
    switch (pluvio->state) {
        case INACTIVO:
            *(pluvio->led) = 0;
            break;
        case DETECTANDO_LLUVIA:
            *(pluvio->led) = 1;
            detect_rain(pluvio);
            break;
        case ACUMULANDO:
            accumulate_rain(pluvio);
            break;
        case REPORTANDO:
            report_rain(pluvio);
            break;
    }
}

// Función para obtener la cantidad de lluvia acumulada en décimas de mm
uint32_t Pluviometer_get_rainfall(Pluviometer* pluvio) {
    return pluvio->accumulated_rain_tenths_mm;
}

// Función para configurar el intervalo de reporte
void Pluviometer_set_report_interval(Pluviometer* pluvio, uint32_t interval) {
    pluvio->report_interval = interval;
}

// Función para configurar el reloj con la fecha y hora actual
void Pluviometer_set_current_time(time_t current_time) {
    set_time(current_time);
}

// Función para detectar lluvia
static void detect_rain(Pluviometer* pluvio) {
    if (pluvio->tick_count > 0) {
        pluvio->state = ACUMULANDO;
    }
}

// Función para acumular lluvia
static void accumulate_rain(Pluviometer* pluvio) {
    pluvio->accumulated_rain_tenths_mm += pluvio->tick_count * TICK_RAIN_TENTHS_MM;
    pluvio->tick_count = 0;
    pluvio->state = INACTIVO;
}

// Función para reportar lluvia
static void report_rain(Pluviometer* pluvio) {
    pluvio->event_queue->call(print_report, pluvio);
    pluvio->accumulated_rain_tenths_mm = 0;
    pluvio->last_report_time = time(NULL);

    // Programar el siguiente reporte
    pluvio->ticker.attach(callback(report_rain, pluvio), std::chrono::minutes(pluvio->report_interval));
}

// Función para imprimir el reporte
static void print_report(Pluviometer* pluvio) {
    time_t current_time;
    time(&current_time);

    struct tm *timeinfo = localtime(&current_time);
    char buffer[128];
    int length = snprintf(buffer, sizeof(buffer), "Rainfall: %u tenths of mm at %04d-%02d-%02d %02d:%02d\n", 
                          pluvio->accumulated_rain_tenths_mm, 
                          timeinfo->tm_year + 1900, 
                          timeinfo->tm_mon + 1, 
                          timeinfo->tm_mday, 
                          timeinfo->tm_hour, 
                          timeinfo->tm_min);
    
    if (length > 0) {
        pluvio->serial->write(buffer, length);
    }
}

// Función que se llama cuando se presiona el botón
static void button_pressed(Pluviometer* pluvio) {
    if (!pluvio->debounce_active) {
        pluvio->debounce_active = true;
        pluvio->debounce_timeout.attach(callback(debounce_finished, pluvio), std::chrono::milliseconds(50)); // 50 ms para el antirrebote
        pluvio->tick_count++;
        pluvio->state = DETECTANDO_LLUVIA;
    }
}

// Función que se llama cuando termina el antirrebote
static void debounce_finished(Pluviometer* pluvio) {
    pluvio->debounce_active = false;
    pluvio->state = INACTIVO;
}

// Función para resetear el pluviómetro
static void reset(Pluviometer* pluvio) {
    pluvio->tick_count = 0;
    pluvio->accumulated_rain_tenths_mm = 0;
}
