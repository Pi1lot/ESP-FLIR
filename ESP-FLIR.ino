/* used with an ESP-WROOM-32 dev board to interface with a FLIR Lepton 3.5 dev module. 
 * This program will use the HSPI bus on the dev board.
 */

#include <SPI.h>
#include <Wire.h>
// #include <LeptonFLiR.h>

#define MISO_PIN      19
#define MOSI_PIN      23 // This is ignored by the module and can be left N/C
#define SCK_PIN       16
#define FLIR_NCS_PIN  5

static const int spiClk = 20000000; // 16 MHz, min is 2.2, max 20. the one article i found used 16

#define I2C_SDA_PIN   2
#define I2C_SCL_PIN   15
#define ADDRESS       0x2A

#define VOSPI_FRAME_SIZE  (164)
uint16_t lepton_frame_packet[VOSPI_FRAME_SIZE / 2];
uint16_t lepton_frame_segment[60][VOSPI_FRAME_SIZE / 2]; // 60 packets per segment



//defining variables related with the image
int image_index;
#define image_x (160)
#define image_y (120)
int image[image_x][image_y];
bool doneCapturing = 0;

// SPI bus which will implement the Lepton VOSPI
SPIClass * hspi = NULL;

void setup() {

  // Setup the hspi bus
  hspi = new SPIClass(HSPI);
  hspi->begin(SCK_PIN, MISO_PIN, MOSI_PIN, FLIR_NCS_PIN);
  hspi->setDataMode(SPI_MODE3);
  pinMode(FLIR_NCS_PIN, OUTPUT);

  // Initialize serial port
  Serial.begin(115200);

  // Add delay needed for lepton setup
  delay(7000);

  // Set up i2c on alternate pins, 400kHz baud
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);
  bool isBusy = true;
  do {
    int status = readRegister
    (0x0002);
    if(!(status & 0b100) & (status & 0b1)){
      Serial.println("I2C is busy.");
      delay(1000);
    } else {
      isBusy = false;
    }
  } while (isBusy);
}

void loop() {
  captureImage();
  transferImage();
  delay(1000);
}

void leptonSync(void){
  int i;
  int data = 0x0f00;

  digitalWrite(FLIR_NCS_PIN, HIGH);
  delay(185); // Waits for the required time indicated by datasheet, ensures a timeout of VoSPI I/F
  
  while ((data & (0x0f00)) == 0x0f00){ // I changed this because I don't think it was right

    // begin checking the first returned data packet, should be a discard packet

    // Start of VoSPI transfer for first packet, reading for discard packet
    digitalWrite(FLIR_NCS_PIN, LOW);
    hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3)); // these arent included in others' code
    data = hspi->transfer(0x00) << 8;
    data |= hspi->transfer(0x00);
    // hspi->endTransaction();
    digitalWrite(FLIR_NCS_PIN, HIGH);

    // Process and discard the remaining data in the packet
    for (i = 0; i < ((VOSPI_FRAME_SIZE - 2) / 2); i++){
      digitalWrite(FLIR_NCS_PIN, LOW);
      hspi->transfer(0x00); // unused garbage data
      hspi->transfer(0x00); // unused garbage data
      digitalWrite(FLIR_NCS_PIN, HIGH);
    }
    
    hspi->endTransaction();
  }
}

void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--) {
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
  }
}

void captureImage( void ) {
  leptonSync();
  delay(50);
  
  bool collectedSegments[4];
  collectedSegments[0] = false;
  collectedSegments[1] = false;
  collectedSegments[2] = false;
  collectedSegments[3] = false;
  uint8_t lastFoundSegment = 0;

  while(!collectedSegments[0] | !collectedSegments[1] | !collectedSegments[2] | !collectedSegments[3]){
      
    // Get 60 valid packets per segment
    digitalWrite(13, LOW); // Debug indicator
    for (int packetNumber = 0; packetNumber < 60; packetNumber++){
      do {
        digitalWrite(FLIR_NCS_PIN, LOW);  
        hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE3));
        byte dummyBuffer[164];
        hspi->transfer(dummyBuffer, sizeof(dummyBuffer));

        for (int i = 0; i < 164l; i += 2) {
          lepton_frame_packet[i/2] = dummyBuffer[i] << 8 | dummyBuffer[i+1];
        }
          
        hspi->endTransaction();
        digitalWrite(FLIR_NCS_PIN, HIGH);
      } while (((lepton_frame_packet[0] & 0x0f00) >> 8 == 0x0f)); // wait until a non-discard packet has been found

      // Load the packet into the segment
      for (int i = 0; i < VOSPI_FRAME_SIZE / 2; i++) {
        lepton_frame_segment[packetNumber][i] = lepton_frame_packet[i];
      }
    }
    digitalWrite(13,HIGH); // Debug indicator

    // Load the collected segment into the image after it's all been captured
    
    int segmentNumber = (lepton_frame_segment[20][0] >> 12) & 0b0111; // This should give the segment number which is held at packet 20. The transmitted packets start at number 0.

    if (segmentNumber != 0){ // if the segment is number 0, ignore the segment
      Serial.println(segmentNumber);
      collectedSegments[segmentNumber - 1] = true;

      for (int packetNumber = 0; packetNumber < 60; packetNumber++) {
        for (int px = 0; px < 80; px ++) {
          if (packetNumber % 2){
            // If the packet number is odd, put it on the right side of the image
            // Placement starts at X = 80 px
            image[80 + px][(segmentNumber - 1) * 30 + (int)(packetNumber/2)] = lepton_frame_segment[packetNumber][px + 2];
          } else {
            // Otherwise put it on the left side
            // Placement starts at X = 0 px
            image[px][(segmentNumber - 1) * 30 + (int)(packetNumber/2)] = lepton_frame_segment[packetNumber][px + 2];
          }
        }
      } 
    }
  }
  Serial.println("Image Complete");
}

void transferImage( void ) {
  int divider = 1;
  for(int i=0; i < image_y / divider; i++){
    for(int j=0; j < image_x / divider; j++){
      Serial.print((int)image[j * divider][i * divider], DEC);
      Serial.print("\t");
    }
    Serial.print("\n");
  }
  Serial.println("Transfer Complete");
}

void debugPackets( byte * packet ) {
  if (((packet[0] & 0x0f) != 0x0f) & (packet[0] != 0x00)){
    // Serial.print("Segment number:\t");
    // Serial.print((lepton_frame_packet[0] & 0xf0) >> 4, HEX);
    // Serial.print("\tPacket Number:\t");
    // Serial.print(lepton_frame_packet[1], DEC);
    printBin(packet[0]);
    Serial.print("\t");
    printBin(packet[1]);
    Serial.println("");
  }
}

int readRegister( unsigned int reg ) {
  int reading = 0;
  setRegister(reg);

  Wire.requestFrom(ADDRESS, 2);

  reading = Wire.read();  // receive high byte (overwrites previous reading)
  // Serial.println(reading);
  reading = reading << 8;    // shift high byte to be high 8 bits

  reading |= Wire.read(); // receive low byte as lower 8 bits
  // Serial.print("reg:");
  // Serial.print(reg);
  // Serial.print("==0x");
  // Serial.print(reading, HEX);
  // Serial.print(" binary:");
  // Serial.println(reading, BIN);
  return reading;
}

void setRegister( unsigned int reg ) {
  byte error;
  Wire.beginTransmission(ADDRESS); // transmit to device #4
  Wire.write(reg >> 8 & 0xff);
  Wire.write(reg & 0xff);            // sends one byte

  error = Wire.endTransmission();    // stop transmitting
  // if (error != 0){
  //   Serial.print("Error =");
  //   Serial.println(error);
  // }
}

