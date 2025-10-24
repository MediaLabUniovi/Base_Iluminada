# Base Iluminada

Proyecto de control de una base iluminada mediante **Arduino Nano**, un **encoder rotatorio** y una **tira LED Neopixel**.  
El sistema permite modificar la intensidad o el color de la iluminación a través del movimiento y pulsación del encoder, ofreciendo una interfaz física simple e intuitiva.

## 🎯 Objetivo

Diseñar una base con iluminación controlable, que sirva como módulo independiente o parte de un proyecto mayor (por ejemplo, una estructura interactiva o decorativa).  
El encoder permite ajustar parámetros de forma precisa y cómoda.

## ⚙️ Componentes principales

- **Arduino Nano**  
- **Encoder rotatorio** con pulsador integrado (CLK, DT, SW)  
- **Tira LED Neopixel** (Adafruit WS2812 o compatible)  
- **Fuente de alimentación de 5 V**

## 💡 Funcionamiento

- **Giro del encoder:** ajusta el brillo de los LEDs de forma progresiva.  
- **Pulsación del encoder:** puede alternar modos de color o encendido/apagado.  
- El programa usa la librería `Encoder` para el control rotativo y `Adafruit_NeoPixel` para la gestión de los LEDs RGB.  
- Se incluye una constante `DIR_SIGN` para invertir el sentido de giro si el encoder lo requiere, y `STEP_SIZE_BRIGHT` para definir el incremento de brillo por paso.

## 🧠 Estructura del código

- `src/main.cpp`: lógica principal del sistema.  
- `platformio.ini`: configuración del entorno PlatformIO.  
- `include/`, `lib/`, `test/`: carpetas estándar del proyecto PlatformIO.  
- `.gitignore`: exclusiones de compilación y archivos temporales.

## 🔧 Configuración de pines (Arduino Nano)

| Elemento        | Pin asignado |
|-----------------|---------------|
| Encoder CLK     | D2 |
| Encoder DT      | D3 |
| Encoder SW      | D4 |
| Tira Neopixel   | D6 |
| Nº de LEDs      | 8 (ajustable) |

## 🧰 Librerías utilizadas

- [`Encoder`](https://www.pjrc.com/teensy/td_libs_Encoder.html)  
- [`Adafruit_NeoPixel`](https://github.com/adafruit/Adafruit_NeoPixel)

## 🚀 Futuras mejoras

- Implementar diferentes modos de iluminación (color fijo, fade, animaciones).  

