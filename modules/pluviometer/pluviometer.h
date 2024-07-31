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




/**
 * @brief Estados del pluviómetro.
 *
 * Esta enumeración define los diferentes estados en los que puede estar el pluviómetro
 * durante su operación.
 */
typedef enum {
    INICIALIZANDO,
    ESCUCHANDO,        /**< El pluviómetro está en espera de precipitación */
    DETECTANDO_LLUVIA, /**< Se ha detectado precipitación y se está iniciando la medición */
    ACUMULANDO,        /**< El pluviómetro está acumulando datos de precipitación */
    REPORTANDO         /**< El pluviómetro está generando un reporte de precipitación */
} Estado;

/**
 * @brief Estructura principal del pluviómetro.
 *
 * Esta estructura contiene todos los componentes y variables necesarios
 * para el funcionamiento del pluviómetro.
 */
typedef struct {
    InterruptIn* boton;       /**< Botón para detectar precipitaciones */
    DigitalOut* led;          /**< LED indicador */
    BufferedSerial* serial;   /**< Interfaz serial para comunicación */
    Timer* timer;             /**< Temporizador principal */
    Timer* debounce_timer;    /**< Temporizador para el antirrebote del botón */
    Estado estado;            /**< Estado actual del pluviómetro */
    int ticks;                /**< Contador de ticks de precipitación */
    int intervalo;            /**< Intervalo de reporte en segundos */
    time_t tiempo_actual;     /**< Tiempo actual del sistema */
    volatile bool bandera_precipitacion; /**< Indica si se ha detectado precipitación */
    char buffer[100];         /**< Buffer para mensajes */
    volatile bool report_ready; /**< Indica si un reporte está listo */
    volatile bool flag_reporte; /**< Bandera para indicar que se debe generar un reporte */
    Ticker ticker_reporte;    /**< Ticker para generar interrupciones periódicas */
    char ubicacion_este[20];  /**< Coordenada Este UTM de la ubicación */
    char ubicacion_norte[20]; /**< Coordenada Norte UTM de la ubicación */
    bool cabecera_impresa;
} Pluviometro;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Inicializa un pluviómetro.
 *
 * Esta función inicializa un pluviómetro con los parámetros especificados.
 *
 * @param p Puntero a la estructura Pluviometro a inicializar.
 * @param pin_boton Pin del botón para detectar precipitaciones.
 * @param pin_led Pin del LED indicador.
 * @param tx Pin de transmisión para la comunicación serial.
 * @param rx Pin de recepción para la comunicación serial.
 * @param intervalo_reporte Intervalo de tiempo (en segundos) entre reportes.
 *
 * @note Esta función debe ser llamada antes de usar cualquier otra función del pluviómetro.
 */
void pluviometro_init(Pluviometro* p, PinName pin_boton, PinName pin_led, PinName tx, PinName rx, int intervalo_reporte);

/**
 * @brief Actualiza el estado del pluviómetro.
 *
 * Esta función debe ser llamada periódicamente para actualizar el estado del pluviómetro,
 * procesar las precipitaciones detectadas y generar reportes si es necesario.
 *
 * @param p Puntero a la estructura Pluviometro a actualizar.
 */
void pluviometro_actualizar(Pluviometro* p);

/**
 * @brief Configura la ubicación geográfica del pluviómetro.
 *
 * Establece las coordenadas UTM (Este y Norte) de la ubicación del pluviómetro.
 *
 * @param p Puntero a la estructura Pluviometro.
 * @param este Coordenada Este UTM como cadena de caracteres.
 * @param norte Coordenada Norte UTM como cadena de caracteres.
 */
void pluviometro_configurar_ubicacion(Pluviometro* p, const char* este, const char* norte);

/**
 * @brief Configura el intervalo de reporte del pluviómetro.
 *
 * Establece un nuevo intervalo de tiempo entre reportes de precipitación.
 *
 * @param p Puntero a la estructura Pluviometro.
 * @param nuevo_intervalo Nuevo intervalo de reporte en segundos.
 */
void pluviometro_configurar_intervalo(Pluviometro* p, int nuevo_intervalo);

/**
 * @brief Configura la fecha y hora del pluviómetro.
 *
 * Establece la fecha y hora actual para el pluviómetro. Esta función debe usarse
 * para sincronizar el reloj interno del dispositivo.
 *
 * @param p Puntero a la estructura Pluviometro.
 * @param year Año (ej. 2024).
 * @param month Mes (1-12).
 * @param day Día del mes (1-31).
 * @param hours Hora (0-23).
 * @param minutes Minutos (0-59).
 * @param seconds Segundos (0-59).
 */
void pluviometro_configurar_fecha_hora(Pluviometro* p, int year, int month, int day, int hours, int minutes, int seconds);


#ifdef __cplusplus
}
#endif

#endif // PLUVIOMETER_H

