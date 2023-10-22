#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <Wire.h>
#include "QMA7981.cpp"
#include <math.h>
#include <MemoryFree.h>
#include "SDcard.cpp"

#define WHEEL_CIRCUMFERENCE_MM 2100

#define PIXELS_PER_TEXTSIZE 7
#define CURRENT_SPEED_TEXTSIZE 6

#define BUTTON_UPPER_RIGHT 22
#define BUTTON_UPPER_LEFT 23
#define BUTTON_LOWER_LEFT 24
#define SENSOR_PIN 3

#define MIN_SPEED_KM_H 1
#define MAX_SPEED_KM_H 10
#define SPEED_ARC_IR 100
#define SPEED_ARC_OR 120
#define SPEED_ARC_X 119
#define SPEED_ARC_Y 119
#define SPEED_ARC_END_DEGREES 270
#define SPEED_ARC_TOTAL_DEGREES 180

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite screen_sprite = TFT_eSprite(&tft);

TFT_eSprite static_sprite = TFT_eSprite(&tft);
//TFT_eSprite current_stats_sprite = TFT_eSprite(&tft);


uint16_t bg_color_16 = tft.color565(0, 0, 80);
uint16_t arc_bg_color_16 = tft.color565(0,0, 50);

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
SDcard card;

int trip_start_timestamp_millis;

int previous_rotation_timestamp_millis = 0;
int current_rotation_timestamp_millis = 0;

int trip_distance_travelled_mm = - WHEEL_CIRCUMFERENCE_MM;
int trip_pause_duration_ms = 0;
int current_pause_duration_ms = 0;
int current_pause_start_timestamp_millis = 0;
bool is_paused = false;

int trip_last_save_timestamp_millis = 0;

float trip_max_speed_km_h = 0;

bool trip_active = false;
bool manual_pause = false;


int pause_button_pressed_timestamp_millis = 0;
int trip_reset_timestamp_millis = 0;



void setup(void) {
  Serial.begin(9600);
  tft.init();
  screen_sprite.createSprite(240, 240);
  static_sprite.createSprite(240, 240);

  acc.initialize();
  card.initialize();

  draw_static_sprite();
  
  pinMode(BUTTON_UPPER_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_UPPER_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LOWER_LEFT, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), interrupt_function, FALLING);
}

void loop() {
  screen_sprite.fillSprite(bg_color_16);
  static_sprite.pushToSprite(&screen_sprite, 0, 0, TFT_TRANSPARENT);

  draw_current_stats();
  draw_system_info();

  if (trip_active) {
    handle_trip_pausing();
    handle_trip_saving();
    handle_sleep_mode();
    handle_trip_interaction();
  }
  
  screen_sprite.pushSprite(0,0);
}

void reset_trip() {
  trip_active = false;
  is_paused = false;
  manual_pause = false;
  card.new_trip();
  trip_start_timestamp_millis = millis();
  previous_rotation_timestamp_millis = millis();
  current_rotation_timestamp_millis = millis();
  trip_distance_travelled_mm = - WHEEL_CIRCUMFERENCE_MM;
  trip_pause_duration_ms = 0;
  current_pause_duration_ms = 0;
  trip_max_speed_km_h = 0;
}

void interrupt_function() {
  if (!trip_active) {
    reset_trip();
    trip_active = true;
  }

  if (is_paused & !manual_pause) {
    trip_pause_duration_ms += current_pause_duration_ms;
    current_pause_duration_ms = 0;
    is_paused = false;
  }
  
  if (!is_paused) {
    previous_rotation_timestamp_millis = current_rotation_timestamp_millis;
    current_rotation_timestamp_millis = millis();
    trip_distance_travelled_mm += WHEEL_CIRCUMFERENCE_MM;
  }
}

void handle_trip_interaction() {
  if ((millis() - trip_reset_timestamp_millis > 10000) & BOOTSEL) {
    screen_sprite.drawString("HOLD TO RESET", 90, 200);
    if (!pause_button_pressed_timestamp_millis) {
      pause_button_pressed_timestamp_millis = millis();
    }
  }
  if (pause_button_pressed_timestamp_millis) {
    int press_duration = millis() - pause_button_pressed_timestamp_millis;
    if (press_duration > 4000) {
      save_trip();
      reset_trip();
      trip_reset_timestamp_millis = millis();
      pause_button_pressed_timestamp_millis = 0;
    } else if (!BOOTSEL & press_duration < 1000) {
      pause_button_pressed_timestamp_millis = 0;
      manual_pause = !manual_pause;
      if (!is_paused) {
        current_pause_start_timestamp_millis = millis();
        is_paused = true;
      }
    }
  }
}

void draw_current_stats() {
  int rotation_time_ms = (current_rotation_timestamp_millis - previous_rotation_timestamp_millis);
  
  float current_speed_m_s = (!is_stopped() & (current_rotation_timestamp_millis - previous_rotation_timestamp_millis != 0)) ? (float) WHEEL_CIRCUMFERENCE_MM / (float) rotation_time_ms : 0;
  float current_speed_km_h = current_speed_m_s * 3.6;
  if (current_speed_km_h > trip_max_speed_km_h) {trip_max_speed_km_h = current_speed_km_h;};

  screen_sprite.setTextColor(TFT_WHITE);
  screen_sprite.setTextPadding(0);

  screen_sprite.setTextDatum(BC_DATUM);
  screen_sprite.setTextSize(6);
  
  screen_sprite.drawFloat(current_speed_m_s * 3.6, 1, 119, 119 + PIXELS_PER_TEXTSIZE);

  float current_max_speed_percentage = constrain(current_speed_km_h / MAX_SPEED_KM_H, 0, 1);
  int speed_arc_cutoff_degrees = SPEED_ARC_END_DEGREES - SPEED_ARC_TOTAL_DEGREES * (1 - current_max_speed_percentage);
  screen_sprite.drawArc(119, 119, SPEED_ARC_OR, SPEED_ARC_IR, speed_arc_cutoff_degrees, SPEED_ARC_END_DEGREES, arc_bg_color_16, bg_color_16, false);

  screen_sprite.setTextSize(4);
  screen_sprite.drawFloat((float) max(trip_distance_travelled_mm, 0) / 1000000, 3, 119, 160);


  int trip_duration_seconds = trip_active ? get_trip_duration_ms() / 1000 : 0;
  int s = trip_duration_seconds % 60;
  int trip_duration_minutes = trip_duration_seconds / 60;
  int m = trip_duration_minutes % 60;
  int h = trip_duration_minutes / 60;
  char trip_duration_string_buffer [10];
  sprintf(trip_duration_string_buffer, "%d:%02d:%02d", h, m, s);
  screen_sprite.drawString(trip_duration_string_buffer, 119, 201);

  float trip_avg_speed_km_h = 3.6 * max(trip_distance_travelled_mm, 0) / get_trip_duration_ms();
  screen_sprite.drawFloat(trip_avg_speed_km_h, 1, 119, 234);

  screen_sprite.setTextSize(2);
  screen_sprite.setTextDatum(TL_DATUM);
  screen_sprite.drawNumber(current_pause_duration_ms / 1000, 0, 0);

  screen_sprite.setTextDatum(TR_DATUM);
  screen_sprite.drawNumber(trip_pause_duration_ms / 1000, 239, 0);
  
  screen_sprite.setTextDatum(BL_DATUM);
  screen_sprite.drawFloat(trip_max_speed_km_h, 1, 0, 239);
}

void draw_system_info() {
  // draw an SD card symbol in green if active, red if not active
  uint16_t sdcard_symbol_color = card.active ? TFT_GREEN : TFT_RED;
  screen_sprite.fillRect(214, 217, 20, 17, sdcard_symbol_color);
  screen_sprite.fillRect(214, 209, 12, 8, sdcard_symbol_color);
  screen_sprite.fillTriangle(226, 209, 233, 214, 226, 217, sdcard_symbol_color);
}

void handle_trip_saving() {
  if (millis() - trip_last_save_timestamp_millis > 5000) {
    save_trip();
    trip_last_save_timestamp_millis = millis();
  }
}

void handle_sleep_mode() {

}

void handle_trip_pausing() {
  if (!is_paused & is_stopped()) {
    is_paused = true;
    current_pause_start_timestamp_millis = millis();
  };

  if (is_paused) {
    current_pause_duration_ms = millis() - current_pause_start_timestamp_millis;
    if (manual_pause) {
      screen_sprite.fillRect(70, 60, 30, 120, TFT_RED);
      screen_sprite.fillRect(140, 60, 30, 120, TFT_RED);
    } else {
      screen_sprite.drawRect(70, 60, 30, 120, TFT_RED);
      screen_sprite.drawRect(71, 61, 28, 118, TFT_RED);
      screen_sprite.drawRect(72, 62, 26, 116, TFT_RED);
      screen_sprite.drawRect(73, 63, 24, 114, TFT_RED);
      screen_sprite.drawRect(74, 64, 22, 112, TFT_RED);
      screen_sprite.drawRect(75, 65, 20, 110, TFT_RED);
      
      screen_sprite.drawRect(140, 60, 30, 120, TFT_RED);
      screen_sprite.drawRect(141, 61, 28, 118, TFT_RED);
      screen_sprite.drawRect(142, 62, 26, 116, TFT_RED);
      screen_sprite.drawRect(143, 63, 24, 114, TFT_RED);
      screen_sprite.drawRect(144, 64, 22, 112, TFT_RED);
      screen_sprite.drawRect(145, 65, 20, 110, TFT_RED);
    }
  }
}

void save_trip() {
  card.append_trip_segment(get_trip_duration_ms(), trip_distance_travelled_mm, trip_pause_duration_ms, trip_max_speed_km_h);
}

void draw_static_sprite() { 
  static_sprite.fillSprite(TFT_TRANSPARENT);
  float center_x = 119;
  float center_y = 119;

  for (int x = 0; x < 240; x++) {

    for (int y = 0; y < 120; y++) {

      float dist_x = x - center_x;
      float dist_y = y - center_y;
      float hypothenuse = sqrt(sq(x - center_x) + sq(y - center_y));
      if ((hypothenuse < SPEED_ARC_OR) & (hypothenuse > SPEED_ARC_IR)) {
        float angle_rads = acos(dist_x / hypothenuse);
        float hue_degrees = angle_rads / 3.1415 * 120.0;

        uint16_t pixel_color = Hue2RGB16(hue_degrees);
        
        static_sprite.drawPixel(x, y, pixel_color);
      }
    }
  }
}

bool is_stopped() {
  float min_speed_mm_ms = MIN_SPEED_KM_H / 3.6;
  float max_time_wheel_rotation_ms = WHEEL_CIRCUMFERENCE_MM / min_speed_mm_ms;
  return get_time_since_last_rotation_ms() > max_time_wheel_rotation_ms;
}

int get_time_since_last_rotation_ms() {
  return millis() - current_rotation_timestamp_millis;
}

int get_trip_duration_ms() {
  return millis() - trip_start_timestamp_millis - trip_pause_duration_ms - current_pause_duration_ms;
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

  return tft.color565(r, g, b);
}
