#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include "mbed.h"

typedef struct {
    BufferedSerial* serial;
    char buffer[512];  // Buffer interno para formatear mensajes
} UARTHandler;

// Inicializa el manejador UART
void uart_init(UARTHandler* handler, PinName tx, PinName rx, int baud);

// Envía una cadena formateada a través de UART
void uart_printf(UARTHandler* handler, const char* format, ...);

// Envía datos crudos a través de UART
void uart_write(UARTHandler* handler, const char* data, size_t length);

// Libera los recursos del manejador UART
void uart_deinit(UARTHandler* handler);

#endif // UART_HANDLER_H