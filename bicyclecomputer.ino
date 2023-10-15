#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <Wire.h>
#include "QMA7981.cpp"

#define BUTTON_UPPER_RIGHT 22
#define BUTTON_UPPER_LEFT 23
#define BUTTON_LOWER_LEFT 24
//#define BUTTON_LOWER_RIGHT 0
#define SENSOR_PIN 2

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
TFT_eSprite face = TFT_eSprite(&tft);

uint16_t bg_color_16 = tft.color565(0, 0, 80);

int16_t x_g;
int16_t y_g;
int16_t z_g;

bool button_upper_left_pressed = false;
bool button_upper_right_pressed = false;
bool button_lower_left_pressed = false;

bool button_upper_left_released = false;
bool button_upper_right_released = false;
bool button_lower_left_released = false;

QMA7981 acc;

void setup(void) {
  Serial.begin(9600);
  acc.initialize();
  
  tft.init();
  face.createSprite(240, 240);
  pinMode(BUTTON_UPPER_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_UPPER_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LOWER_LEFT, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
}

void loop() {
  face.fillSprite(bg_color_16);
  if (digitalRead(SENSOR_PIN)==0) {
    face.drawRect(110, 110, 20, 20, TFT_RED);
  }
  if (digitalRead(BUTTON_UPPER_LEFT)==0) {
    button_upper_left_pressed = true;
  } else if (button_upper_left_pressed) {
    button_upper_left_released = true;
  }
  
  if (digitalRead(BUTTON_UPPER_RIGHT)==0) {
    button_upper_right_pressed = true;
  } else if (button_upper_right_pressed) {
    button_upper_right_released = true;
  }
  
  if (digitalRead(BUTTON_LOWER_LEFT)==0) {
    button_lower_left_pressed = true;
  } else if (button_lower_left_pressed) {
    button_lower_left_released = true;
  }

  if (button_upper_left_pressed & button_upper_left_released) {
    face.drawRect(10, 10, 20, 20, TFT_RED);
    button_upper_left_pressed = false;
    button_upper_left_released = false;
  }

  if (button_upper_right_pressed & button_upper_right_released) {
    face.drawRect(210, 10, 20, 20, TFT_RED);
    button_upper_right_pressed = false;
    button_upper_right_released = false;
  }

  if (button_lower_left_pressed & button_lower_left_released) {
    face.drawRect(10, 210, 20, 20, TFT_RED);
    button_lower_left_pressed = false;
    button_lower_left_released = false;
  }

  x_g = acc.read_x();
  y_g = acc.read_y();
  z_g = acc.read_z();

  if ((abs(x_g) > abs(y_g)) & abs(x_g) > abs(z_g)) {
    if (x_g < 0) {
      face.drawRect(110, 210, 20, 20, TFT_GREEN);
    } else {
      face.drawRect(110, 10, 20, 20, TFT_GREEN);
    }
  } else if ((abs(y_g) > abs(x_g)) & (abs(y_g) > abs(z_g))) {
    if (y_g < 0) {
      face.drawRect(10, 110, 20, 20, TFT_GREEN);
    } else {
      face.drawRect(210, 110, 20, 20, TFT_GREEN);
    }
  } else if ((abs(z_g) > abs(x_g)) & (abs(z_g) > abs(y_g))) {
    if (z_g < 0) {
      face.drawRect(115, 115, 10, 10, TFT_GREEN);
    } else {
      face.drawRect(110, 110, 20, 20, TFT_GREEN);
    }
  }
  face.pushSprite(0,0, TFT_TRANSPARENT);
}
