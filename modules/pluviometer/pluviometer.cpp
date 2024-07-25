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
#include "mbed.h"
#include "pluviometer.h"
#include <cstdio>
#include <cstring>
#include <chrono>

#define DEBOUNCE_TIME_MS 50
#define TICK_VALUE 2 // 2 décimas de mm por tick

volatile int timeout_flag = 0;

void timeout_isr() {
    timeout_flag = 1;
}

void boton_isr(void* context) {
    Pluviometro* p = (Pluviometro*)context;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(p->debounce_timer->elapsed_time()).count() > DEBOUNCE_TIME_MS) {
        p->bandera_precipitacion = true;
        p->debounce_timer->reset();
    }
}

void pluviometro_init(Pluviometro* p, PinName pin_boton, PinName pin_led, PinName tx, PinName rx) {
    p->boton = new InterruptIn(pin_boton);
    p->led = new DigitalOut(pin_led);
    p->serial = new BufferedSerial(tx, rx, 115200);
    p->timer = new Timer();
    p->debounce_timer = new Timer();

    p->estado = ESCUCHANDO;
    p->ticks = 0;
    p->intervalo = 60;
    p->bandera_precipitacion = false;
    p->report_ready = false;
    p->debounce_timer->start();

    p->boton->fall(callback(boton_isr, p));
    p->serial->set_format(8, BufferedSerial::None, 1);
    p->timer->start();

    pluviometro_mensaje_inicio(p); // Mostrar mensaje de inicio
}
void pluviometro_actualizar(Pluviometro* p) {
    if (p->bandera_precipitacion) {
        p->bandera_precipitacion = false;
        manejar_interrupcion(p);
    }

    if (std::chrono::duration_cast<std::chrono::seconds>(p->timer->elapsed_time()).count() >= p->intervalo) {
        finalizar_acumulacion(p);
    }

    switch (p->estado) {
        case ESCUCHANDO:
            if (std::chrono::duration_cast<std::chrono::seconds>(p->timer->elapsed_time()).count() >= p->intervalo) {
                pluviometro_reportar_lluvia(p);
            }
            break;
        case DETECTANDO_LLUVIA:
            iniciar_acumulacion(p);
            break;
        case ACUMULANDO:
            break;
        case REPORTANDO:
            pluviometro_reportar_lluvia(p);
            break;
    }
}

void pluviometro_reportar_lluvia(Pluviometro* p) {
    time_t tiempo = p->tiempo_actual;
    struct tm *tiempo_local = localtime(&tiempo);
    
    int lluvia_acumulada = p->ticks * TICK_VALUE;
    snprintf(p->buffer, sizeof(p->buffer), "%04d-%02d-%02d %02d:%02d:%02d - Lluvia acumulada: %d.%d mm\n",
             tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday,
             tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec,
             lluvia_acumulada / 10, lluvia_acumulada % 10);
    
    p->serial->write(p->buffer, strlen(p->buffer));
    
    p->ticks = 0;
    p->timer->reset();
    p->timer->start();
    cambiar_estado(p, ESCUCHANDO);
}

void pluviometro_configurar_intervalo(Pluviometro* p, int segundos) {
    p->intervalo = segundos;
}

void pluviometro_configurar_fecha_hora(Pluviometro* p, int year, int month, int day, int hours, int minutes, int seconds) {
    struct tm tiempo_config = {0};
    tiempo_config.tm_year = year - 1900;
    tiempo_config.tm_mon = month - 1;
    tiempo_config.tm_mday = day;
    tiempo_config.tm_hour = hours;
    tiempo_config.tm_min = minutes;
    tiempo_config.tm_sec = seconds;
    
    p->tiempo_actual = mktime(&tiempo_config);
    set_time(p->tiempo_actual);
}

// Actualizar el mensaje de inicio si es necesario
void pluviometro_mensaje_inicio(Pluviometro* p) {
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "Pluviometro inicializado.\nIntervalo de reporte: %d segundos.\nConfiguracion de puerto serie: 115200 baudios, 8 bits de datos, sin paridad, 1 bit de parada.\nEstado inicial: ESCUCHANDO\n", p->intervalo);
    p->serial->write(buffer, strlen(buffer));
}

void manejar_interrupcion(Pluviometro* p) {
    // Incrementar el contador de ticks
    p->ticks++;
    // Alternar el estado del LED
    p->led->write(!p->led->read());
    
    // Cambiar el estado si está escuchando
    if (p->estado == ESCUCHANDO) {
        cambiar_estado(p, DETECTANDO_LLUVIA);
    }
    // Reiniciar el temporizador si ya estamos acumulando
    else if (p->estado == ACUMULANDO) {
        p->timer->reset();
    }
}


void cambiar_estado(Pluviometro* p, Estado nuevo_estado) {
    p->estado = nuevo_estado;
    // Imprimir el nuevo estado por UART
    #ifdef DEBUG_PRINT_ESTADOS
        // Imprimir el nuevo estado por UART solo si DEBUG_PRINT_ESTADOS está definido

        const char* estado_str = estado_a_cadena(nuevo_estado);
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Estado actual: %s\n", estado_str);
        p->serial->write(buffer, strlen(buffer));
    #endif
}


void iniciar_acumulacion(Pluviometro* p) {
    cambiar_estado(p, ACUMULANDO);
    p->timer->reset();
    p->timer->start();
}

void finalizar_acumulacion(Pluviometro* p) {
    p->tiempo_actual = time(NULL);
    cambiar_estado(p, REPORTANDO);
}

const char* estado_a_cadena(Estado estado) {
    switch (estado) {
        case ESCUCHANDO:
            return "ESCUCHANDO";
        case DETECTANDO_LLUVIA:
            return "DETECTANDO_LLUVIA";
        case ACUMULANDO:
            return "ACUMULANDO";
        case REPORTANDO:
            return "REPORTANDO";
        default:
            return "DESCONOCIDO";
    }
}
// para escribir mensales de depuracion 
static void debug_print(Pluviometro* p, const char* mensaje) {
    #ifdef DEBUG_PRINT_ESTADOS
    p->serial->write(mensaje, strlen(mensaje));
    #endif
}