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
#include <string.h>
#include <cstdio>
#include <cstring>
#include <chrono>

#define DEBOUNCE_TIME_MS 100
#define TICK_VALUE 2 // 2 décimas de mm por tick


/**
 * @brief Imprime el mensaje de inicio del pluviómetro.
 *
 * Esta función genera y muestra un mensaje de inicio que incluye
 * información sobre la configuración inicial del pluviómetro.
 *
 * @param p Puntero a la estructura Pluviometro.
 */
static void pluviometro_mensaje_inicio(Pluviometro* p);

/**
 * @brief Genera y envía un reporte de lluvia.
 *
 * Esta función calcula la cantidad de lluvia acumulada y envía
 * un reporte a través de la interfaz serial.
 *
 * @param p Puntero a la estructura Pluviometro.
 */
static void pluviometro_reportar_lluvia(Pluviometro* p);

/**
 * @brief Convierte un estado del pluviómetro a su representación en cadena.
 *
 * @param estado El estado del pluviómetro a convertir.
 * @return const char* Cadena que representa el estado.
 */
static const char* estado_a_cadena(Estado estado);

/**
 * @brief Cambia el estado del pluviómetro.
 *
 * Esta función actualiza el estado del pluviómetro y realiza
 * las acciones necesarias asociadas al cambio de estado.
 *
 * @param p Puntero a la estructura Pluviometro.
 * @param nuevo_estado El nuevo estado al que se cambiará.
 */
static void cambiar_estado(Pluviometro* p, Estado nuevo_estado);

/**
 * @brief Maneja la interrupción generada por la detección de lluvia.
 *
 * Esta función se llama cuando se detecta una precipitación y
 * actualiza los contadores y estados del pluviómetro.
 *
 * @param p Puntero a la estructura Pluviometro.
 */
static void manejar_interrupcion(Pluviometro* p);

/**
 * @brief Inicia el proceso de acumulación de lluvia.
 *
 * Esta función se llama cuando se comienza a detectar lluvia
 * y prepara el pluviómetro para acumular datos.
 *
 * @param p Puntero a la estructura Pluviometro.
 */
static void iniciar_acumulacion(Pluviometro* p);

/**
 * @brief Finaliza el proceso de acumulación de lluvia.
 *
 * Esta función se llama cuando se deja de detectar lluvia o
 * cuando se cumple el intervalo de reporte.
 *
 * @param p Puntero a la estructura Pluviometro.
 */
static void finalizar_acumulacion(Pluviometro* p);

/**
 * @brief Imprime la cabecera de los datos del pluviómetro.
 *
 * Esta función genera y envía una cabecera que describe los campos
 * de los datos que se reportarán.
 *
 * @param p Puntero a la estructura Pluviometro.
 */
static void imprimir_cabecera_datos(Pluviometro* p);

/**
 * @brief Obtiene la fecha y hora actual como una cadena formateada.
 *
 * @return char* Cadena con la fecha y hora actual en formato "YYYY-MM-DD HH:MM:SS".
 */
static char* obtener_fecha_hora_actual(void);

/**
 * @brief Función de callback para el reporte periódico.
 *
 * Esta función se llama periódicamente para indicar que es hora de generar un reporte.
 *
 * @param context Puntero al contexto (estructura Pluviometro).
 */
static void callback_reporte(void* context);

/**
 * @brief Manejador de la interrupción del botón (ISR).
 *
 * Esta función se llama cuando se detecta una pulsación del botón,
 * indicando una precipitación.
 *
 * @param context Puntero al contexto (estructura Pluviometro).
 */
static void boton_isr(void* context);


void callback_reporte(void* context) {
    Pluviometro* p = (Pluviometro*)context;
    p->flag_reporte = true;
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
    p->ubicacion_este[sizeof(p->ubicacion_este) - 1] = '\0';  // Asegura terminación nula
    strncpy(p->ubicacion_norte, norte, sizeof(p->ubicacion_norte) - 1);
    p->ubicacion_norte[sizeof(p->ubicacion_norte) - 1] = '\0';  // Asegura terminación nula
}

void pluviometro_init(Pluviometro* p, PinName pin_boton, PinName pin_led, PinName tx, PinName rx, int intervalo_reporte) {
    p->boton = new InterruptIn(pin_boton);
    p->led = new DigitalOut(pin_led);
    p->serial = new BufferedSerial(tx, rx, 115200);
    p->timer = new Timer();
    p->debounce_timer = new Timer();

    p->estado = ESCUCHANDO;
    p->ticks = 0;

    p->bandera_precipitacion = false;
    p->debounce_timer->start();

    p->boton->fall(callback(boton_isr, p));
    p->serial->set_format(8, BufferedSerial::None, 1);
    p->timer->start();

    p->intervalo = intervalo_reporte; 
    p->ticker_reporte.attach(callback(&callback_reporte, p), std::chrono::seconds(intervalo_reporte));


    pluviometro_mensaje_inicio(p);
    imprimir_cabecera_datos(p);
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
            finalizar_acumulacion(p);
            pluviometro_reportar_lluvia(p);
            p->timer->reset();
            cambiar_estado(p, ESCUCHANDO);
            break;
    }
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

void pluviometro_reportar_lluvia(Pluviometro* p) {
    char* fecha_hora = obtener_fecha_hora_actual();
    int lluvia_acumulada = p->ticks * TICK_VALUE;
    
    snprintf(p->buffer, sizeof(p->buffer),
             "%s, %d.%d\n",
             fecha_hora,
             lluvia_acumulada / 10,
             lluvia_acumulada % 10);
    
    p->serial->write(p->buffer, strlen(p->buffer));
    
    p->ticks = 0;
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

// Actualizar el mensaje de inicio si es necesario
void pluviometro_mensaje_inicio(Pluviometro* p) {
    char buffer[500];
    char* fecha_hora = obtener_fecha_hora_actual();
    snprintf(buffer, sizeof(buffer), 
    "# Pluviometro inicializado a las %s.\n"
    "# Intervalo de reporte: %d segundos.\n"
    "# Puerto serie:\n" 
    "#    115200 baudios, 8 bits de datos,\n" 
    "#    sin paridad, 1 bit de parada.\n"
    "# Ubicacion: Este UTM %s, Norte UTM %s\n",
    fecha_hora, 
    p->intervalo,  
    p->ubicacion_este, 
    p->ubicacion_norte);
    p->serial->write(buffer, strlen(buffer));
}

void imprimir_cabecera_datos(Pluviometro* p) {
    const char* cabecera = "# Fecha [YYYY-MM-DD] Hora [HH:MM:SS], Precipitacion Acumulada [mm]\n";
    p->serial->write(cabecera, strlen(cabecera));
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


void pluviometro_configurar_intervalo(Pluviometro* p, int nuevo_intervalo) {
    p->ticker_reporte.detach();
    p->ticker_reporte.attach(callback(&callback_reporte, p), std::chrono::seconds(nuevo_intervalo));
    p->intervalo = nuevo_intervalo;  // Añade esta línea para actualizar el intervalo en la estructura
}
