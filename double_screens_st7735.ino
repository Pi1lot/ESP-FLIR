#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS    5
#define TFT_DC    4
#define TFT_RST   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

unsigned long frameCount = 0;
unsigned long lastFpsUpdate = 0;
float fps = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initialisation...");

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
}

void loop() {
  // --- DESSIN RAPIDE ---
  int x = random(0, tft.width());
  int y = random(10, tft.height()); // 🔐 commence à y = 10 pour laisser l’espace des FPS
  int r = random(2, 8);
  uint16_t color = random(0xFFFF);
  tft.fillCircle(x, y, r, color);

  frameCount++;

  // --- FPS UPDATE ---
  unsigned long now = millis();
  if (now - lastFpsUpdate >= 1000) {
    fps = frameCount * 1000.0 / (now - lastFpsUpdate);
    lastFpsUpdate = now;
    frameCount = 0;

    // Réserve une zone propre pour les FPS
    tft.fillRect(0, 0, 80, 10, ST77XX_BLACK); // Nettoie uniquement la bande du haut
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.print("FPS: ");
    tft.print((int)fps);

    Serial.print("FPS: ");
    Serial.println((int)fps);
  }
}
