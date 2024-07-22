/*
 * Nombre del archivo: pluviometer.h
 * Descripción: Biblioteca para la detección y reporte de lluvias usando un pluviómetro.
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
#ifndef PLUVIOMETER_H
#define PLUVIOMETER_H

#include "mbed.h"

// Constantes
#define TICK_RAIN_TENTHS_MM 2   // Cada tick representa 2 décimas de mm de lluvia caída

// Estados del pluviómetro
typedef enum {
    INACTIVO,
    DETECTANDO_LLUVIA,
    ACUMULANDO,
    REPORTANDO
} PluviometerState;

typedef struct {
    PluviometerState state;
    uint32_t report_interval; // Intervalo de reporte en minutos
    uint32_t tick_count;
    uint32_t accumulated_rain_tenths_mm;
    time_t last_report_time;
    bool debounce_active;
    DigitalOut *led;
    InterruptIn *button;
    Ticker ticker;
    Timeout debounce_timeout;
    Timer rain_timer;
    BufferedSerial *serial;
    EventQueue *event_queue;
    Thread *event_thread;
} Pluviometer;

// Función de inicialización
void Pluviometer_init(Pluviometer* pluvio, PinName button_pin, PinName led_pin, PinName tx_pin, PinName rx_pin);

// Función para actualizar el estado del pluviómetro
void Pluviometer_update(Pluviometer* pluvio);

// Función para obtener la cantidad de lluvia acumulada en décimas de mm
uint32_t Pluviometer_get_rainfall(Pluviometer* pluvio);

// Función para configurar el intervalo de reporte (en minutos)
void Pluviometer_set_report_interval(Pluviometer* pluvio, uint32_t interval);

// Función para configurar el reloj con la fecha y hora actual
void Pluviometer_set_current_time(time_t current_time);

#endif // PLUVIOMETER_H
