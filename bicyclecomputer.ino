#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <Wire.h>
#include "QMA7981.cpp"
#include <math.h>
#include <EasyColor.h>
#include <tuple>

#define BUTTON_UPPER_RIGHT 22
#define BUTTON_UPPER_LEFT 23
#define BUTTON_LOWER_LEFT 24
//#define BUTTON_LOWER_RIGHT 0
#define SENSOR_PIN 3

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
TFT_eSprite face = TFT_eSprite(&tft);
TFT_eSprite current_stats_face = TFT_eSprite(&tft);
TFT_eSprite colored_arc_sprite = TFT_eSprite(&tft);

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

bool rot_detected = false;

QMA7981 acc;

int previous_rotation_timestamp_millis = 0;
int current_rotation_timestamp_millis = 0;

int wheel_circumference_mm = 2000;

int distance_travelled_mm = 0;

EasyColor::HSVRGB HSVConverter;


void setup(void) {
  Serial.begin(9600);
  acc.initialize();
  
  tft.init();
  face.createSprite(240, 240);
  current_stats_face.createSprite(120, 120);

  colored_arc_sprite.createSprite(240, 120);

  create_colored_arc_sprite();
  
  pinMode(BUTTON_UPPER_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_UPPER_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LOWER_LEFT, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), interrupt_function, FALLING);
}

void loop() {
  //tft.fillScreen(bg_color_16);
  draw_current_trip_stats_sprite();
  colored_arc_sprite.pushSprite(0,0, TFT_TRANSPARENT);
  // current_stats_face.pushSprite(100, 140, TFT_TRANSPARENT);
}

void interrupt_function() {
  previous_rotation_timestamp_millis = current_rotation_timestamp_millis;
  current_rotation_timestamp_millis = millis();
  distance_travelled_mm += wheel_circumference_mm;
}

void draw_current_trip_stats_sprite() {
  current_stats_face.fillSprite(bg_color_16);
  current_stats_face.setCursor(0,0);
  int rotation_time_ms = (current_rotation_timestamp_millis - previous_rotation_timestamp_millis);
  int time_since_last_rotation_ms = millis() - current_rotation_timestamp_millis;
  float current_speed_m_s = (time_since_last_rotation_ms < 10000) ? (float) wheel_circumference_mm / (float) rotation_time_ms : 0;
  current_stats_face.println(distance_travelled_mm);
  current_stats_face.print(current_speed_m_s);
}

void create_colored_arc_sprite() { 
  colored_arc_sprite.fillSprite(bg_color_16);
  float center_x = 119.5;
  float center_y = 119;
  int outer_radius = 120;
  int inner_radius = 90;

  for (int x = 0; x < 240; x++) {

    for (int y = 0; y < 120; y++) {

      float dist_x = x - center_x;
      float dist_y = y - center_y;
      float hypothenuse = sqrt(sq(x - center_x) + sq(y - center_y));
      if ((hypothenuse <= outer_radius) & (hypothenuse >= inner_radius)) {
        float angle_rads = acos(dist_x / hypothenuse);
        float hue_degrees = angle_rads / 3.1415 * 120.0;
        
        rgb out_rgb;
        hsv in_hsv;
        in_hsv.h = hue_degrees;
        in_hsv.s = 100;
        in_hsv.v = 255;
    
        out_rgb = HSVConverter.HSVtoRGB(in_hsv,out_rgb);

        uint16_t pixel_color = Hue2RGB16(hue_degrees);
        

        /*if (((x == 10) | (x == 100) | (x == 119) | (x == 140) | (x == 230)) & ((y == 10) | (y == 110))) {
          Serial.print("x = ");
          Serial.print(x);
          Serial.print("; y = ");
          Serial.println(y);

          Serial.print("hypo = ");
          Serial.print(hypothenuse);
          Serial.print("; angle rads = ");
          Serial.println(angle_rads);

          Serial.print("hue = ");
          Serial.println(hue_degrees);
        }*/
       
        colored_arc_sprite.drawPixel(x, y, pixel_color);
      }
    }
  }
}

uint16_t Hue2RGB16(float h) {
  float s = 1;
  float v = 1;
  h = fmod(h, 360);

  float c = v*s;
  float x = c * (1 - fabsf( fmod(h/60.0, 2) -1));
  float m = v - c;

  int sextant = h / 60;

  float rp;
  float gp;
  float bp;

  switch (sextant) {
    case 0:
      rp = c;
      gp = x;
      bp = 0;
      break;
      
    case 1:
      rp = x;
      gp = c;
      bp = 0;
      break;
      
    case 2:
      rp = 0;
      gp = c;
      bp = x;
      break;
      
    case 3:
      rp = 0;
      gp = x;
      bp = c;
      break;
      
    case 4:
      rp = x;
      gp = 0;
      bp = c;
      break;
      
    case 5:
      rp = c;
      gp = 0;
      bp = x;
      break;
  };

  int r = (rp+m)*255;
  int g = (gp+m)*255;
  int b = (bp+m)*255;

  /*Serial.print("r = ");
  Serial.print(r);
  Serial.print("; g = ");
  Serial.print(g);
  Serial.print("; b = ");
  Serial.println(b);*/

  return tft.color565(r, g, b);
}
