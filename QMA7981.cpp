#include <Wire.h>

#define PIN_SDA 20
#define PIN_SCL 21

#define ADDR 0x12

#define REG_DEVICE_ID 0x00
#define REG_X_LSB 0x01
#define REG_X_MSB 0x02
#define REG_Y_LSB 0x03
#define REG_Y_MSB 0x04
#define REG_Z_LSB 0x05
#define REG_Z_MSB 0x06

#define REG_RANGE 0x0f
#define REG_BANDWIDTH 0x10
#define REG_MODE 0x11

#define REG_SOFT_RESET 0x36


#define RANGE_2G 0b0001
#define RANGE_4G 0b0010
#define RANGE_8G 0b0100
#define RANGE_16G 0b1000
#define RANGE_32G 0b1111

#define MCLK_DIV_BY_7695 0b000
#define MCLK_DIV_BY_3855 0b001
#define MCLK_DIV_BY_1935 0b010
#define MCLK_DIV_BY_975 0b011
#define MCLK_DIV_BY_15375 0b101
#define MCLK_DIV_BY_30735 0b110
#define MCLK_DIV_BY_61455 0b111

#define CLK_500_KHZ 0b0001
#define CLK_333_KHZ 0b0000
#define CLK_200_KHZ 0b0010
#define CLK_100_KHZ 0b0011
#define CLK_50_KHZ 0b0100
#define CLK_25_KHZ 0b0101
#define CLK_12_KHZ_5 0b0110
#define CLK_5_KHZ 0b0111

class QMA7981 {
  public:
  
    QMA7981() {
    }

    void initialize() {
      Wire.setSDA(PIN_SDA);
      Wire.setSCL(PIN_SCL);
      Wire.begin();

      soft_reset();
      set_active();
      set_frequency();
      set_bandwidth();
      set_range();
      
    }

    void set_active() {
      byte mode_current = read_register(REG_MODE);
      byte mode_active = mode_current | 0b10000000;
      write_register(REG_MODE, mode_active);
    }

    byte read_device_id() {
      return read_register(REG_DEVICE_ID);
    }

    int16_t read_x() {
      byte x_lsb = read_register(REG_X_LSB);
      byte x_msb = read_register(REG_X_MSB);

      int16_t x = x_msb;
      x <<= 8;
      x |= (x_lsb & 0b11111100);
      x >>= 2;
      
      return x;
    }

    int16_t read_y() {
      byte y_lsb = read_register(REG_Y_LSB);
      byte y_msb = read_register(REG_Y_MSB);

      int16_t y = y_msb;
      y <<= 8;
      y |= (y_lsb & 0b11111100);
      y >>= 2;
      
      return y;
    }

    int16_t read_z() {
      byte z_lsb = read_register(REG_Z_LSB);
      byte z_msb = read_register(REG_Z_MSB);

      int16_t z = z_msb;
      z <<= 8;
      z |= (z_lsb & 0b11111100);
      z >>= 2;
      
      return z;
    }

    void write_register(byte reg, byte val){
        Wire.beginTransmission(ADDR);
        Wire.write(reg);
        Wire.write(val);
        int result = Wire.endTransmission();
        if (result != 0) Serial.println("Error");
    }

    byte read_register(byte reg) {
      byte received = 0x00;
      Wire.beginTransmission(ADDR);
      Wire.write(reg);
      Wire.endTransmission();

      Wire.requestFrom(ADDR, 1);
      received = Wire.read();
      return received;
    }
    
    void soft_reset() {
      write_register(REG_SOFT_RESET, 0xB6);
      write_register(REG_SOFT_RESET, 0x00);
    }

    void set_mode() {
      byte value = read_register(REG_MODE);
      value |= 0b10000000;
      write_register(REG_MODE, value);
    }

    void set_frequency() {
      byte value = read_register(REG_MODE);
      value &= 0b11110000; // discard last 4 bits
      value |= CLK_500_KHZ; // set frequency to 50kHz
      write_register(REG_MODE, value);
    }

    void set_range() {
      write_register(REG_RANGE, 0b11100000 | RANGE_4G);
    }

    void set_bandwidth() {
      write_register(REG_BANDWIDTH, 0b11100000 | MCLK_DIV_BY_61455);
    }

    void interpret_transmission_result(byte result) {
      if (result == 0) {
        Serial.println("read register address success");
      } else if (result == 1) {
        Serial.println("data too long");
      } else if (result == 2) {
        Serial.println("ADDRESS NACK");
      } else if (result == 3) {
        Serial.println("DATA NACK");
      } else if (result == 4) {
        Serial.println("other error");
      } else if (result == 5) {
        Serial.println("timeout");
      }
    }

    
};
