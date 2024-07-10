# Proyecto Pluviómetro

**autor: Luis Gómez**

Este proyecto implementa un pluviómetro utilizando la plataforma MBED. El sistema mide la cantidad de lluvia mediante un sensor de tick y reporta la cantidad de lluvia acumulada a través de la comunicación serial.

## Descripción

El sistema se compone de sensores para detectar la lluvia, un análisis de datos para acumular y reportar la cantidad de lluvia, y actuadores para indicar cuando se ha detectado lluvia.

![Figura 1: Esquema de un pluviómetro de balancín (fuente. Segerer et al., 2006)
](doc/fig/pluviometro.png)

**Figura 1:** *Esquema de un pluviómetro de balancín (fuente. Segerer et al., 2006)*

## Estructura del Código

### Sensores

- **initializeSensors()**: Inicializa los sensores configurando el modo del botón de detección de lluvia y apagando los LEDs.
- **isRaining()**: Verifica si está lloviendo comprobando el estado del botón de detección de lluvia.

### Análisis de Datos

- **analyzeRainfall()**: Analiza la lluvia detectada, imprime la hora actual y acumula la cantidad de lluvia detectada.
- **accumulateRainfall()**: Incrementa el contador de lluvia.
- **hasTimePassedMinutesRTC(int minutes)**: Verifica si ha pasado el tiempo especificado en minutos desde la última comprobación.

### Actuación

- **actOnRainfall()**: Enciende los LEDs de alarma y tick, y analiza la lluvia detectada.
- **reportRainfall()**: Imprime la cantidad de lluvia acumulada y resetea el contador de lluvia.
- **printRain(const char* buffer)**: Imprime un mensaje indicando que se ha detectado lluvia.
- **DateTimeNow()**: Obtiene la fecha y hora actual en formato `"%Y-%m-%d %H:%M:%S"`.
- **printAccumulatedRainfall()**: Imprime la cantidad de lluvia acumulada en el formato `"YYYY-MM-DD HH:MM - Accumulated rainfall: X.XX mm"`.

## Uso

### Requisitos

- Placa compatible con MBED
- Sensor de lluvia (tick)
- LEDs para indicaciones
- Comunicación serial configurada

### Configuración

1. Clonar el repositorio en tu entorno de desarrollo MBED.
2. Compilar y cargar el código en tu placa MBED.
3. Conectar los componentes de hardware (sensor de lluvia, LEDs, etc.) a la placa MBED según el esquema del proyecto.

### Ejecución

1. Inicializar los sensores llamando a la función `initializeSensors()`.
2. El programa principal ejecutará un bucle infinito que:
    - Detectará si está lloviendo y actuará en consecuencia.
    - Verificará a intervalos regulares la cantidad de lluvia acumulada y la reportará.

### Ejemplo de Salida

```plaintext
2024-07-01 12:00:00 - Rain detected
2024-07-01 12:01 - Accumulated rainfall: 0.20 mm

