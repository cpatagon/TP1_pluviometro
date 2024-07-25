#include "mbed.h"
#include "pluviometer.h"

int main() {
    Pluviometro pluviometro;
    pluviometro_init(&pluviometro, USER_BUTTON, LED1, USBTX, USBRX);
    pluviometro_configurar_fecha_hora(&pluviometro, 2024, 7, 21, 12, 0, 0);
    pluviometro_configurar_intervalo(&pluviometro, 60); // Intervalo de 1 minuto
    
    while (1) {
        pluviometro_actualizar(&pluviometro);
        wait_us(10000); // Esperar 10 ms
    }
}