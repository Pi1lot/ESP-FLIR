# ESP-FLIR
 Basic implementation of an interface between a generic ESP-WROOM-32 dev module and a FLIR Lepton 3.5 with it's respsective dev board using the Arduino IDE. This is also designed to implement an IPS LCD Display, but would likely also work with a TFT LCD.

 Development of this implementation for the ESP-WROOM-32 will not continue. This is because for a full proper impelementation which fills the entire screen with the thermal image it would be required to have more memory through addition of an external SRAM module, or modification of the utilized TFT_eSPI library to suppport scaling of smaller sprites when pushing to the screen. This functionality is not native to the library as of 2/17/2024. The ESP-WROOM-32 does not have enough memory to store both the full thermal data and a 320x240 frame buffer sprite.
 The program will instead be ported to a Teensy 4.1, as it has enough memory to achieve the end goal, and an integrated uSD card slot which decreases final complexity.
 <p align="center">
  <img src="https://github.com/Jacob-Lundstrom/ESP-FLIR/blob/main/demo-images/IMG_1396.png?raw=true" width="600">
 </p>
 
 PARTS USED:
 - Generic ESP-WROOM-32 Dev Board [https://www.amazon.com/gp/product/B0BB2PJBQD]
 - FLIR Lepton 3.5 (I got mine second hand on ebay for about a third of its price at Digikey) [https://www.digikey.com/en/products/detail/flir-lepton/500-0771-01/7606616]
 - FLIR Lepton breakout board V2 [https://www.digikey.com/en/products/detail/flir-lepton/250-0577-00/10385179]
