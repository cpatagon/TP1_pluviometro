/*
 * Nombre del archivo: main.cpp
 * Descripción: Implementación de reporte de lluvias usando un pluviómetro.
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
#include "uart_handler.h"



#define TIEMPO_REPORTE_PLUVIOMETRO 60 // en segundos
#define UBICACION_ESTE_UTM "691249.92"
#define UBICACION_NORTE_UTM "5711836.83"

int main() {
    // Inicializar el manejador UART
    UARTHandler uart_handler;
    uart_init(&uart_handler, USBTX, USBRX, 115200);
    int counter = 0;
    uart_printf(&uart_handler, "12Test UART #%d\r\n", counter);
    ThisThread::sleep_for(1s);



    Pluviometro pluviometro;
    pluviometro_init(&pluviometro, USER_BUTTON, LED1, USBTX, USBRX, TIEMPO_REPORTE_PLUVIOMETRO);
    pluviometro_configurar_fecha_hora(&pluviometro, 2024, 7, 21, 12, 0, 0);
    pluviometro_configurar_intervalo(&pluviometro, TIEMPO_REPORTE_PLUVIOMETRO );
    pluviometro_configurar_ubicacion(&pluviometro,  UBICACION_ESTE_UTM, UBICACION_NORTE_UTM ); 
 
    
    while (1) {
        pluviometro_actualizar(&pluviometro);
    }
}