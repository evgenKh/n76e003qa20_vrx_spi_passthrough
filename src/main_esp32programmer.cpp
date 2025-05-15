#if USE_MAIN_ESP32PROGRAMMER

#include <Arduino.h>
#include <SPI.h>
#include "SpiIAPProgrammer.h"

#if USE_FT3500
# include "config.h"
# define CONFIG_FT3500_START 3300
# define CONFIG_FT3500_STOP 3400
# define CONFIG_FT3500_STEP 5
# if USE_FT3500_DETECTED_VIDEO
#   include "ADCUtils.h"
#   include "DetectedVideo.h"
#   include "VRX58.h"
# else
class DetectedVideo{};
# endif // USE_FT3500_DETECTED_VIDEO
# include "FT3500.h"
#endif // USE_FT3500

#define PIN_ESP32_CS 5
#define PIN_ESP32_MISO 19
#define PIN_ESP32_MOSI 23
#define PIN_ESP32_SCK 18
#define PIN_ESP32_ADC 32

SPIClass          m_spiClass(VSPI);
SpiIAPProgrammer  m_programmer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PIN_ESP32_CS, OUTPUT);
  pinMode(PIN_ESP32_SCK, OUTPUT);
  pinMode(PIN_ESP32_MOSI, OUTPUT);
  pinMode(PIN_ESP32_MISO, INPUT_PULLUP);

  m_spiClass.begin(PIN_ESP32_SCK, PIN_ESP32_MISO, PIN_ESP32_MOSI, -1);  
  delay(10);

  Serial.printf("Starting programming\n");
  bool programSuccess = m_programmer.Program(&m_spiClass, PIN_ESP32_CS, SpiIAPProgrammer::k_hexStringsSimplePassthrough, 10000);
  Serial.printf("Programming finished with: %d", (int)programSuccess);

  delay(1000);
  m_spiClass.end();

}



void loop() {   

#if USE_FT3500
  FT3500 ft3500(PIN_ESP32_MOSI, PIN_ESP32_SCK, PIN_ESP32_CS);
  ft3500.begin(PIN_ESP32_ADC);
  while(true)
  {
    //ft3500.nextChanel();
    ft3500.loops(0);
    delay(1000);
  }
#endif //USE_FT3500   
  
}

#endif // USE_MAIN_ESP32PROGRAMMER