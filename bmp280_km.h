#ifndef BMP280_KM_H
#define BMP280_KM_H
#include <stdint.h>
#include <linux/i2c-dev.h>


//Constants for BMP280 Registers and global variables
#define BMP280_I2C_ADDRESS 0x76
#define DEVICE_NAME "bmp280"

#define BMP280_REG_CHIPID 0xD0
#define BMP280_TEMP_MSB 0xFA
#define BMP280_TEMP_LSB 0xFB
#define BMP280_TEMP_XLSB 0xFC



// BMP280 Data structure
struct bmp280_data {
    struct i2c_client *client;
    struct mutexlock;
    struct {
        uint8_t chip_id;
        uint16_t dig_T1;
        int16_t dig_T2;
        int16_t dig_T3;
    } calib;
};

















#endif