//Librerias para comunicarse y manejar el Modulo
// LCD 168x120 8 Pines

// Microcontrolador ST7735S
// #include <Adafruit_ST7789.h>
// #include <Adafruit_ST7796S.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST77xx.h>

//
#include <Adafruit_GFX.h>
// #include <gfxfont.h>
// #include <Adafruit_GrayOLED.h>
// #include <Adafruit_SPITFT.h>
// #include <Adafruit_SPITFT_Macros.h>

// Pines de el LCD

#define TFT_CS 10 // Pin selector de Chip CS (Chip Select)
#define TFT_DC 9 // Pin de Comunicacion RS(DC) Data/Command
#define TFT_RST 8 // Pin para Reiniciar el Display RES(Reset)
#define PIN_BACKLIGHT 6 // Pin PWM (Digital) para Controlar el Brillo LEDA (Brillo de Fondo)

// Pin Sensor Efecto Hall
#define pinSensor A2

// Constructor
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Variables
float alpha = 0.05;
float filtrado = 0;
uint16_t minRaw = 1023, maxRaw = 0;
uint8_t brillo = 64; // uint8_t 0 - 255 | Ocupa 1 Byte

unsigned long lastUpdate = 0; // unsingned long 0 - 4,294,967,295 (Igual que un uint32_t) | 4 Bytes
const uint16_t updateInterval = 200; // cada 200 ms uint16_t 0 - 65535 | 2 Bytes

static int16_t prevLectura = -1; // int16_t -32768 - 32767 | 2 Bytes
static int16_t prevFiltrado = -1;
static int16_t prevMapeo = -1;

// Para tener en cuenta
/*

DIMENSIONES PANTALLA: 160 x 128

TAMAÑOS Text Sizes: Se multiplica la Base (1,2,3) x (6 en X y 8 en Y).
1 --> 6x8 px
2 --> 12x16 px
3 --> 18x24 px

tft.fillScreen(color) --> Rellena toda la pantalla con un color (p. ej. negro).
tft.fillRect(x, y, w, h, color) --> Dibuja un rectángulo lleno (sirve para “borrar” un área).
tft.setCursor(x, y) --> Mueve el “cursor” donde empezará a dibujar texto.
tft.setTextColor(color) --> Cambia el color del texto (solo letras).
tft.setTextColor(fg, bg) --> Cambia color de letras y de fondo (reemplaza texto anterior sin borrar pantalla).
tft.setTextSize(n) --> Escala el tamaño de las letras (1=normal, 2=doble, etc.).
tft.print("texto") --> Escribe texto en la pantalla (igual que Serial.print).
*/

// Funcion para calibrar la entrada analogica y definir los valores
// Maximos y Minimos Capturados al momento de usar el Dispositivo.
void calibrar() {
  // Serial.println("Mueve el imán a los extremos...");
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Calibrando...");
  tft.println("Mueve el iman a extremos");
  tft.println("min: ");
  tft.setCursor(0, 75);
  tft.print("max: "); 

  delay(2000);

  unsigned long t0 = millis();

  while (millis() - t0 < 6000) {
    int val = analogRead(pinSensor);
    if (val < minRaw) {
      minRaw = val;
      tft.fillRect(50, 50, 80, 20, ST77XX_BLACK);
      tft.setCursor(50, 50);
      tft.print(minRaw);
    }
    if (val > maxRaw) {
      maxRaw = val;
      tft.fillRect(50, 75, 80, 20, ST77XX_BLACK);
      tft.setCursor(50, 75);
      tft.print(maxRaw);
    }
    
    delay(5);
  }
  // Serial.println("Calibrado: min="); Serial.print(minRaw);
  // Serial.print(" max="); Serial.println(maxRaw);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(0, 0);
  tft.print("Min: "); tft.println(minRaw);
  tft.print("Max: "); tft.println(maxRaw);

  delay(1500);
}

void setup() {
  Serial.begin(9600);

  // Inicializando el Pin Digital para controlar el brillo
  pinMode(PIN_BACKLIGHT, OUTPUT);
  analogWrite(PIN_BACKLIGHT, brillo);

  // Inicializa la pantalla
  tft.initR(INITR_BLACKTAB); // Inicializa ST7732
  tft.fillScreen(ST77XX_BLACK);
  // tft.setTextColor(ST77XX_WHITE);
  tft.setRotation(1); // Se usa para girar la orientancion.
  tft.setTextSize(2);

  delay(1000);
  calibrar();

  filtrado = analogRead(pinSensor);

  // Limpiando La Pantalla
  tft.fillScreen(ST77XX_BLACK);
  
  // Mostrando los valores Min-Max
  tft.setCursor(0, 100);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  tft.print("Min: " + String(minRaw) + " Max: " + String(maxRaw));

  // Mostrando el Valor de Brillo Actual
  tft.setCursor(0, 110);
  tft.print("Brillo: " + String(brillo));

  // Dibujano las Estiquetas Fijas
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Raw:");
  tft.setCursor(0, 25);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Filtro:");
  tft.setCursor(0, 50);
  tft.setTextColor(ST77XX_RED);
  tft.print("Map:");

}

void loop() {
  // Control de brillo por serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    brillo = input.toInt();
    delay(200);
    brillo = constrain(brillo, 0, 255);
    analogWrite(PIN_BACKLIGHT, brillo);
    tft.setTextSize(1);
    tft.fillRect(46, 110, 100, 10, ST77XX_BLACK);
    tft.setCursor(46, 110);
    tft.setTextColor(ST77XX_CYAN);
    tft.print(brillo);
    tft.setTextSize(2);
    Serial.print("Brillo Ajustado a: ");
    Serial.println(brillo);
  }

  // Lectura y Filtrado del sensor
  int16_t lectura = analogRead(pinSensor);
  filtrado = alpha * lectura + (1 - alpha) * filtrado;
  int mapeo = map(constrain(filtrado, minRaw, maxRaw), minRaw, maxRaw, 0, 1023);

  // Mostrando los Datos en el LCD

  if (millis()- lastUpdate > updateInterval) {

    // Actualiza solo si cambia el valor.

    tft.setTextSize(2);

    if (lectura != prevLectura) {
      tft.fillRect(100, 0, 80, 20, ST77XX_BLACK); // Limpia solo el numero
      tft.setCursor(100, 0);
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      tft.print(lectura);
      prevLectura = lectura;
    }

    if ((int)filtrado != prevFiltrado) {
      tft.fillRect(100, 25, 80, 20, ST77XX_BLACK);
      tft.setCursor(100, 25);
      tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
      tft.print((int)filtrado);
      prevFiltrado = (int)filtrado;
    }

    if (mapeo != prevMapeo) {
      tft.fillRect(100, 50, 80, 20, ST77XX_BLACK);
      tft.setCursor(100, 50);
      tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
      tft.print(mapeo);
      prevMapeo = mapeo;
    }

    // Serial.print("Raw: ");
    // Serial.println(lectura);
    // Serial.print("Filtrado: ");
    // Serial.println((int)filtrado);
    // Serial.print("Mapeado: ");
    // Serial.println(mapeo);
    
    lastUpdate = millis();
  }

  delay(20);
}
