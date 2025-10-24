#include <Arduino.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>

// -----------------------------
// Pines (Arduino Nano)
#define ENCODER_CLK_PIN 2   // A (CLK) -> INT0
#define ENCODER_DT_PIN  3   // B (DT)  -> INT1
#define ENCODER_SW_PIN  4   // SW con pull-up interno
#define NEOPIXEL_PIN    6   // DIN de la tira
#define NUM_PIXELS      8   // ajusta al numero de LEDs

// -----------------------------
// Encoder (ABS + signo de direccion)
Encoder miEncoder(ENCODER_CLK_PIN, ENCODER_DT_PIN);
const int  STEPS_PER_NOTCH = 4;   // prueba 1/2/4 segun tu encoder
const int  DIR_SIGN        = 1;   // pon 1 o -1 para invertir el sentido

// Tamaño del paso logico por notch del encoder
const int STEP_SIZE_BRIGHT = 5;   // brillo en pasos de 5
const int STEP_SIZE_HUE    = 5;   // color en pasos de 5 grados

// -----------------------------
// Boton
const unsigned long DEBOUNCE_MS   = 30;
const unsigned long LONG_PRESS_MS = 600;
bool lastBtnState   = HIGH; // por pull-up
bool btnPressed     = false;
unsigned long lastDebounceMs = 0;
unsigned long pressedAtMs    = 0;

// -----------------------------
// Modo y potencia
enum Modo { MODO_BRILLO, MODO_COLOR };
Modo  modo    = MODO_BRILLO; // por defecto regula brillo
bool  powerOn = true;        // encendido al arrancar

// -----------------------------
// NeoPixel
Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
uint16_t hue   = 0;    // 0..359
uint8_t  bright= 128;  // 0..255

// -----------------------------
// Utilidades

// HSV -> RGB (h:0..359, s:0..255, v:0..255)
static void hsv2rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b)
{
  uint8_t region = h / 60;
  uint16_t remainder = (h % 60) * 255 / 60;

  uint8_t p = (uint16_t)v * (255 - s) / 255;
  uint8_t q = (uint16_t)v * (255 - (s * remainder / 255)) / 255;
  uint8_t t = (uint16_t)v * (255 - (s * (255 - remainder) / 255)) / 255;

  switch (region) {
    default:
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
}

// Aplica el estado actual a toda la tira
void aplicarTira()
{
  if (!powerOn) {
    strip.clear();
    strip.show();
    return;
  }

  strip.setBrightness(bright);

  uint8_t r, g, b;
  hsv2rgb(hue, 255, 255, r, g, b); // saturacion y valor base a 255
  uint32_t c = strip.Color(r, g, b);

  for (uint16_t i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

// Sincroniza la cuenta del encoder con la variable activa (brillo/hue)
// para evitar saltos al cambiar de modo.
// Usamos notches = valor/paso.
void syncEncoderWithCurrentValue()
{
  long targetNotches;
  if (modo == MODO_BRILLO) {
    targetNotches = (long)bright / STEP_SIZE_BRIGHT;
  } else {
    targetNotches = (long)hue / STEP_SIZE_HUE;
  }
  miEncoder.write(targetNotches * STEPS_PER_NOTCH * DIR_SIGN);
}

// Manejo de boton con deteccion de pulsacion corta/larga
void actualizarBoton()
{
  bool lectura = digitalRead(ENCODER_SW_PIN);

  if (lectura != lastBtnState) {
    lastDebounceMs = millis();
    lastBtnState   = lectura;
  }

  if ((millis() - lastDebounceMs) > DEBOUNCE_MS) {
    if (!btnPressed && lectura == LOW) {
      btnPressed  = true;
      pressedAtMs = millis();
    } else if (btnPressed && lectura == HIGH) {
      unsigned long dur = millis() - pressedAtMs;
      btnPressed = false;

      if (dur >= LONG_PRESS_MS) {
        // Larga: encender/apagar
        powerOn = !powerOn;
        Serial.print("Power: ");
        Serial.println(powerOn ? "ON" : "OFF");
        aplicarTira();
      } else {
        // Corta: alternar modo
        modo = (modo == MODO_BRILLO) ? MODO_COLOR : MODO_BRILLO;
        Serial.print("Modo: ");
        Serial.println(modo == MODO_BRILLO ? "Brillo" : "Color");
        // Alinear el encoder al nuevo valor activo
        syncEncoderWithCurrentValue();
      }
    }
  }
}

// Lectura ABSOLUTA (como el basico) + paso de 5 en 5 + saturacion + reanclaje
void actualizarEncoderABS()
{
  // Notches absolutos con signo de direccion
  long notches = (miEncoder.read() * DIR_SIGN) / STEPS_PER_NOTCH;

  if (modo == MODO_BRILLO) {
    // Mapear notches -> brillo en pasos de STEP_SIZE_BRIGHT
    long rawValue = notches * STEP_SIZE_BRIGHT;

    // Saturar 0..255
    if (rawValue < 0)   rawValue = 0;
    if (rawValue > 255) rawValue = 255;

    // Re-anclar el contador si saturamos (para que no siga sumando fuera)
    long satNotches = rawValue / STEP_SIZE_BRIGHT;
    if (satNotches != notches) {
      miEncoder.write(satNotches * STEPS_PER_NOTCH * DIR_SIGN);
    }

    if ((uint8_t)rawValue != bright) {
      bright = (uint8_t)rawValue;
      Serial.print("Brillo: ");
      Serial.println(bright);
      aplicarTira();
    }

  } else { // MODO_COLOR
    long rawValue = notches * STEP_SIZE_HUE;

    // Saturar 0..359
    if (rawValue < 0)   rawValue = 0;
    if (rawValue > 359) rawValue = 359;

    long satNotches = rawValue / STEP_SIZE_HUE;
    if (satNotches != notches) {
      miEncoder.write(satNotches * STEPS_PER_NOTCH * DIR_SIGN);
    }

    if ((uint16_t)rawValue != hue) {
      hue = (uint16_t)rawValue;
      Serial.print("Hue: ");
      Serial.println(hue);
      aplicarTira();
    }
  }
}

void setup()
{
  Serial.begin(9600);

  // Importante para encoders mecanicos:
  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN,  INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN,  INPUT_PULLUP);

  strip.begin();
  strip.show();

  aplicarTira();

  // Arranca sincronizado para evitar saltos iniciales
  syncEncoderWithCurrentValue();

  Serial.println("Listo: brillo/color en pasos de 5; click corto cambia modo; mantener pulsado: ON/OFF.");
}

void loop()
{
  actualizarBoton();
  actualizarEncoderABS();
}
