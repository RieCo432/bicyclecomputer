#include <SDFS.h>
#include <SPI.h>

#define SD_MISO 16
#define SD_MOSI 19
#define SD_CS 17
#define SD_SCK 18

#define TRIP_SEGMENT_STRING_MAX_LENGTH 97

class SDcard {
  private:
    int current_trip_number = 0;
    char current_trip_filename_buffer [23];
    
    
  public:
    bool active;
    SDcard() {
      
    }

    void initialize() {
      SPI.setRX(SD_MISO);
      SPI.setTX(SD_MOSI);
      SPI.setSCK(SD_SCK);

      SDFSConfig cfg;
      cfg.setCSPin(SD_CS);
      cfg.setAutoFormat(false);
      SDFS.setConfig(cfg);

      if (SDFS.begin()) {
        if (!SDFS.exists("/trips")) {
          SDFS.mkdir("/trips");
          SDFS.end();
        }
        active = true;
      } else {active = false;}
    }

    void new_trip() {
      if (SDFS.begin()) {
        Dir trips_dir = SDFS.openDir("/trips");
        while (trips_dir.next()) {
          current_trip_number++;
        }
        Serial.println(current_trip_number);

        sprintf(current_trip_filename_buffer, "/trips/trip_%06d.txt", current_trip_number);
        SDFS.end();
        active = true;
      } else {active = false;}
    }

    void append_trip_segment(int trip_duration_ms, int trip_distance_mm, int trip_pause_ms, float trip_max_speed_km_h) {
      if (SDFS.begin()) {
        char trip_segment_string_buffer [TRIP_SEGMENT_STRING_MAX_LENGTH];
        int trip_segment_string_length = sprintf(trip_segment_string_buffer, "duration_ms=%d\ndistance_mm=%d\npause_ms=%d\nmax_speed_kmh=%.2f\n\n", trip_duration_ms, trip_distance_mm, trip_pause_ms, trip_max_speed_km_h);
        File current_trip_file = SDFS.open(current_trip_filename_buffer, "a");
        //Serial.print(trip_segment_string_buffer);
        current_trip_file.write(trip_segment_string_buffer, trip_segment_string_length);
        current_trip_file.close();
        Serial.println("File Written");
        SDFS.end();
        active = true;
      } else {active = false;} 
    }
};
