
#include "uart_handler.h"
#include <cstdarg>
#include <cstring>

void uart_init(UARTHandler* handler, PinName tx, PinName rx, int baud) {
    handler->serial = new BufferedSerial(tx, rx, baud);
    handler->serial->set_format(8, BufferedSerial::None, 1);
}

void uart_printf(UARTHandler* handler, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int length = vsnprintf(handler->buffer, sizeof(handler->buffer), format, args);
    va_end(args);

    if (length > 0) {
        if (length < sizeof(handler->buffer)) {
            handler->serial->write(handler->buffer, length);
        } else {
            // El mensaje es demasiado largo, lo dividimos en partes
            for (int i = 0; i < length; i += sizeof(handler->buffer) - 1) {
                int chunk_length = snprintf(handler->buffer, sizeof(handler->buffer), 
                                            "%.*s", sizeof(handler->buffer) - 1, format + i);
                handler->serial->write(handler->buffer, chunk_length);
            }
        }
    } else {
        const char* error_msg = "Error: formato inválido\n";
        handler->serial->write(error_msg, strlen(error_msg));
    }
}

void uart_write(UARTHandler* handler, const char* data, size_t length) {
    handler->serial->write(data, length);
}

void uart_deinit(UARTHandler* handler) {
    delete handler->serial;
    handler->serial = nullptr;
}

