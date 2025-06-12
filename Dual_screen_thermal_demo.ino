#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// --- Brochage ST7735 --- //
#define TFT_CS    5
#define TFT_DC    4
#define TFT_RST   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// --- Image thermique importée depuis EXAMPLE.h --- //
#include "EXAMPLE.h"  // contient: int __test_image[160*120];

const int IMAGE_WIDTH = 160;
const int IMAGE_HEIGHT = 120;

// Calcul des valeurs min et max dynamiquement
void find_min_max(int image[], int size, double *max, double *min) {
  *min = *max = image[0];
  for (int i = 1; i < size; i++) {
    if (image[i] > *max) *max = image[i];
    if (image[i] < *min) *min = image[i];
  }
}

// Génère une couleur pseudo-thermique simple (bleu ↔ rouge)
uint16_t getThermalColor(double normVal) {
  if (normVal == 1.0) {
    return ST77XX_BLACK;
  } else if (normVal == 0.0) {
    return tft.color565(0, 255, 0);  // Vert pour valeur mini
  } else {
    int r = 255 * normVal;
    int b = 255 * (1.0 - normVal);
    return tft.color565(r, 0, b);
  }
}

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // Portrait, modifie selon ton câblage
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  double minVal, maxVal;
  find_min_max(__test_image, 19200, &maxVal, &minVal);

  for (int y = 0; y < IMAGE_HEIGHT; y++) {
    for (int x = 0; x < IMAGE_WIDTH; x++) {
      int idx = y * IMAGE_WIDTH + x;
      double norm = (__test_image[idx] - minVal) / (maxVal - minVal);
      uint16_t color = getThermalColor(norm);

      // Ne pas afficher le bas-gauche (optionnel, comme dans ton code)
      if (x < 40 && y > 103) continue;

      // Si tu veux un zoom 2x : remplace drawPixel par fillRect ici
      tft.drawPixel(x, y, color);
    }
  }

  // Affiche les min/max fixes (ou calculés dynamiquement si tu veux)
  tft.setCursor(0, 122); // Laisse 120 lignes pour l’image
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.print("Min: ");
  tft.print((int)minVal);
  tft.print("  Max: ");
  tft.println((int)maxVal);

  delay(1000); // Pause avant mise à jour suivante
}
