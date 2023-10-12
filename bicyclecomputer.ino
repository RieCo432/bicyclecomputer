#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library

#define BUTTON_UPPER_RIGHT 22
#define BUTTON_UPPER_LEFT 23
#define BUTTON_LOWER_LEFT 24
#define BUTTON_LOWER_RIGHT 

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

uint16_t bg_color_16 = tft.color565(0, 0, 80);

void setup(void) {
  Serial.begin(9600);
  tft.init();
  pinMode(BUTTON_UPPER_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_UPPER_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LOWER_LEFT, INPUT_PULLUP);
}

void loop() {
  tft.fillScreen(bg_color_16);
  if (digitalRead(BUTTON_UPPER_LEFT)==0) {
    tft.drawRect(10, 10, 20, 20, TFT_RED);
  }
  if (digitalRead(BUTTON_UPPER_RIGHT)==0) {
    tft.drawRect(210, 10, 20, 20, TFT_RED);
  }
  if (digitalRead(BUTTON_LOWER_LEFT)==0) {
    tft.drawRect(10, 210, 20, 20, TFT_RED);
  }

  
  delay(30);
}
