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
#include <cstdint>
#include <string.h>
#include <cstdio>
#include <cstring>
#include <stdarg.h>


// Descomentar la siguiente línea para habilitar la impresión de estados
// (solo en modo prueba)
// #define DEBUG_PRINT_ESTADOS

#define DEBOUNCE_TIME_MS 100
#define TICK_VALUE 2 // 2 décimas de mm por tick


#define REINICIAR_TICKS_CICLO(p) ((p)->ticks = 0)
#define NUMERO_TICKS_CICLO(p) ((p)->ticks)



#ifdef DEBUG_PRINT_ESTADOS
    static void debug_print(Pluviometro* p, const char* mensaje);
#endif


static const char* estado_a_cadena(Estado estado);
static void cambiar_estado(Pluviometro* p, Estado nuevo_estado);
static void manejar_interrupcion(Pluviometro* p);
static void iniciar_acumulacion(Pluviometro* p);
static void finalizar_acumulacion(Pluviometro* p);
static void imprimir_cabecera_datos(Pluviometro* p);
static char* obtener_fecha_hora_actual(void);
static void callback_reporte(void* context);
static void boton_isr(void* context);
static void enviar_uart(Pluviometro* p);
static void reiniciar_tiempo_ciclo(Pluviometro* p);
static char* cabecera_datos(Pluviometro* p);

void pluviometro_imprimir(Pluviometro* p, const char* format, ...);
static int32_t calcular_lluvia_acumulada(Pluviometro* p);

static void reportar_lluvia(Pluviometro* p);


static int32_t calcular_lluvia_acumulada(Pluviometro* p) {
    return NUMERO_TICKS_CICLO(p) * TICK_VALUE;   // Asumiendo que TICK_VALUE es esta en decima de mm
}

void pluviometro_imprimir(Pluviometro* p, const char* format, ...) {
    char buffer[512];  // Aumentamos el tamaño para manejar mensajes más largos
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length > 0 && length < sizeof(buffer)) {
        p->serial->write(buffer, length);
    } else {
        const char* error_msg = "Error: mensaje demasiado largo o formato inválido\n";
        p->serial->write(error_msg, strlen(error_msg));
    }
}

void callback_reporte(void* context) {
    Pluviometro* p = (Pluviometro*)context;
    p->flag_reporte = true;
    #ifdef DEBUG_PRINT_ESTADOS
        debug_print(p, "Flag de reporte activado");
    #endif
}

void boton_isr(void* context) {
    Pluviometro* p = (Pluviometro*)context;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(p->debounce_timer->elapsed_time()).count() > DEBOUNCE_TIME_MS) {
        p->bandera_precipitacion = true;
        p->debounce_timer->reset();
    }
}

void pluviometro_configurar_ubicacion(Pluviometro* p, const char* este, const char* norte) {
    strncpy(p->ubicacion_este, este, sizeof(p->ubicacion_este) - 1);
    p->ubicacion_este[sizeof(p->ubicacion_este) - 1] = '\0';  
    strncpy(p->ubicacion_norte, norte, sizeof(p->ubicacion_norte) - 1);
    p->ubicacion_norte[sizeof(p->ubicacion_norte) - 1] = '\0';  
}

void pluviometro_init(Pluviometro* p, PinName pin_boton, PinName pin_led, PinName tx, PinName rx, int intervalo_reporte) {
    p->boton = new InterruptIn(pin_boton);
    p->led = new DigitalOut(pin_led);
    p->serial = new BufferedSerial(tx, rx, 115200);
    p->timer = new Timer();
    p->debounce_timer = new Timer();

    p->estado = ESCUCHANDO;
    REINICIAR_TICKS_CICLO(p);

    p->bandera_precipitacion = false;
    p->debounce_timer->start();

    p->boton->fall(callback(boton_isr, p));
    p->serial->set_format(8, BufferedSerial::None, 1);
    p->timer->start();

    p->intervalo = intervalo_reporte; 
    p->ticker_reporte.attach(callback(&callback_reporte, p), std::chrono::seconds(intervalo_reporte));
    p->estado = INICIALIZANDO;
    p->cabecera_impresa = false;

}
void pluviometro_actualizar(Pluviometro* p) {
      // Manejar la bandera de precipitación primero
    if (p->bandera_precipitacion) {
        p->bandera_precipitacion = false;
        manejar_interrupcion(p);
    }

    // Manejar la bandera de reporte
    if (p->flag_reporte) {
        p->flag_reporte = false;
        p->tiempo_actual = time(NULL);
        cambiar_estado(p, REPORTANDO);
    }

    // Máquina de estados
    switch (p->estado) {
        case INICIALIZANDO:
            if (!p->cabecera_impresa) {
                imprimir_cabecera_datos(p);
                p->cabecera_impresa = true;
            }
            cambiar_estado(p, ESCUCHANDO);
            break;
        case ESCUCHANDO:
            if (p->bandera_precipitacion) {
                cambiar_estado(p, DETECTANDO_LLUVIA);
            }
            break;

        case DETECTANDO_LLUVIA:
            iniciar_acumulacion(p);
            break;

        case ACUMULANDO:
            // No hacemos nada aquí, solo esperamos la próxima interrupción
            break;

        case REPORTANDO:
            #ifdef DEBUG_PRINT_ESTADOS
                debug_print(p, "Entrando en estado REPORTANDO");
            #endif
            finalizar_acumulacion(p);
            reportar_lluvia(p);
            enviar_uart(p);
            reiniciar_tiempo_ciclo(p);
            REINICIAR_TICKS_CICLO(p);
            cambiar_estado(p, ESCUCHANDO);
            break;
    }
}

void reiniciar_tiempo_ciclo(Pluviometro* p){
    p->timer->reset();
}

void suma_ticks_ciclo(Pluviometro* p){
    p->ticks++;
}

char* obtener_fecha_hora_actual() {
    static char buffer[20];  // Tamaño suficiente para "YYYY-MM-DD HH:MM:SS\0"
    time_t tiempo_actual;
    struct tm *tiempo_local;

    time(&tiempo_actual);
    tiempo_local = localtime(&tiempo_actual);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tiempo_local);
    return buffer;
}

static void reportar_lluvia(Pluviometro* p) {
    char* fecha_hora = obtener_fecha_hora_actual();
    int lluvia_acumulada = calcular_lluvia_acumulada(p);
    
    snprintf(p->buffer, sizeof(p->buffer),
             "%s, %d.%d\n",
             fecha_hora,
             lluvia_acumulada / 10,
             lluvia_acumulada % 10);
    
    #ifdef DEBUG_PRINT_ESTADOS
        debug_print(p, "Prueba reporte_lluvia: ");
    #endif
}

static void enviar_uart(Pluviometro* p) {
    pluviometro_imprimir(p, "%s", p->buffer);

}


void pluviometro_configurar_fecha_hora(Pluviometro* p, int year, int month, int day, int hours, int minutes, int seconds) {
    struct tm tiempo_config = {0};
    tiempo_config.tm_year = year - 1900;
    tiempo_config.tm_mon = month - 1;  // Los meses en struct tm van de 0 a 11
    tiempo_config.tm_mday = day;
    tiempo_config.tm_hour = hours;
    tiempo_config.tm_min = minutes;
    tiempo_config.tm_sec = seconds;
    p->tiempo_actual = mktime(&tiempo_config);
    set_time(p->tiempo_actual);
}



char* cabecera_datos(Pluviometro* p){
    char buffer[400];
    int length = snprintf(buffer, sizeof(buffer),
                            "# Pluviometro inicializado a las %s.\n"
                            "# Intervalo de reporte: %d segundos.\n"
                            "# Puerto serie: 115200 baudios, 8 bits de datos, sin paridad, 1 bit de parada.\n"
                            "# Ubicacion: Este UTM %s, Norte UTM %s\n"
                            "# Fecha [YYYY-MM-DD] Hora [HH:MM:SS], Precipitacion Acumulada [mm]\n",
                            obtener_fecha_hora_actual(), 
                            p->intervalo, 
                            p->ubicacion_este, 
                            p->ubicacion_norte
                       );
return buffer;
}

void imprimir_cabecera_datos(Pluviometro* p) {
    #ifdef DEBUG_PRINT_ESTADOS
        debug_print(p, "¡¡¡ IMPORTANTE !!!: PLUVIOMETRO EN MODO DE PRUEBA\n");
    #endif
    pluviometro_imprimir(p, "%s", cabecera_datos(p));
}

void manejar_interrupcion(Pluviometro* p) {
    // Incrementar el contador de ticks
    suma_ticks_ciclo(p);
    // Alternar el estado del LED
    p->led->write(!p->led->read());

    #ifdef DEBUG_PRINT_ESTADOS
        char debug_buffer[50];
        snprintf(debug_buffer, sizeof(debug_buffer), "Tick detectado. Total: %d", NUMERO_TICKS_CICLO(p));
        debug_print(p, debug_buffer);
    #endif

    // Cambiar el estado si está escuchando
    if (p->estado == ESCUCHANDO) {
        cambiar_estado(p, DETECTANDO_LLUVIA);
    }
    // Reiniciar el temporizador si ya estamos acumulando
    else if (p->estado == ACUMULANDO) {
        reiniciar_tiempo_ciclo(p);
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
        pluviometro_imprimir(p, "%s", buffer);
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
        case INICIALIZANDO:
            return "INICIALIZANDO";
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
        pluviometro_imprimir(p, "%s\n", mensaje);
    #endif
}


void pluviometro_configurar_intervalo(Pluviometro* p, int nuevo_intervalo) {
    p->ticker_reporte.detach();
    p->ticker_reporte.attach(callback(&callback_reporte, p), std::chrono::seconds(nuevo_intervalo));
    p->intervalo = nuevo_intervalo;  // Añade esta línea para actualizar el intervalo en la estructura
}

