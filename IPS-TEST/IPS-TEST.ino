// TFT (Actually an IPS panel) test for my thermal camera interface

#include <SPI.h>
// NOTE: ALL COLORS MUST BE SUBTRACTED FROM 0xFFFF (inverted) TO MAINTAIN COMPATIBILITY WITH THIS LIBRARY
#include <TFT_eSPI.h>

#include "EXAMPLE.h"  // Stores pre-recorded data from the sensor

TFT_eSPI ips = TFT_eSPI();
// int image[sizeof(__test_image)] = __test_image;

void setup() {
  Serial.begin(115200);
  ips.init();
  ips.setRotation(3);
  ips.fillScreen(0xFFFF-TFT_BLACK);
}

void find_min_max(int image[], int size, double *max, double *min) {
  *max = *min = image[0];

  for (int i = 0; i < size; i++) {
    if (image[i] > *max) {
      *max = image[i];
    }
    if (image[i] < *min) {
      *min = image[i];
    }
  }
}

void loop() {

  double max, min;

  find_min_max(__test_image, 19200, &max, &min);

  for (int i = 0; i < 160; i++) {
    for (int j = 0; j < 120; j++) {
      
      double val = (__test_image[i + j*160] - min) / (max - min);
      int color;
      if (val == 1.0){
        color = 0x0000;
      } else if (val == 0) {
        color = ips.color565(0,255,0);
      } else {
        color = 0xFFFF - ips.color565(255 * val,0, 255 *(1 - val));
      }
      if (i < 40) {
        if (j > 103) {
          continue;
        }
      }
      
      ips.drawPixel(1 * i,1 * j, color);
      // ips.drawPixel(2 * i,2 * j, color);
      // ips.drawPixel(2 * i + 1, 2 * j, color);
      // ips.drawPixel(2 * i, 2 * j + 1, color);
      // ips.drawPixel(2 * i + 1, 2 * j + 1, color);
    }
  }

  ips.setCursor(0, 210, 2);
  ips.setTextColor(0xFFFF - TFT_WHITE, 0xFFFF - TFT_BLACK);
  ips.setTextColor(1);
  ips.print("Max: "); ips.println("25 C");
  ips.print("Min: "); ips.print("-3 C");
}

