#ifndef BMP280_KM_H
#define BMP280_KM_H
#include <linux/i2c-dev.h>
#include <linux/mutex.h>
#include <linux/cdev.h>


//Constants for BMP280 Registers and global variables
#define BMP280_I2C_ADDRESS 0x76
#define DEVICE_NAME "bmp280"

#define BMP280_REG_CHIPID 0xD0
#define BMP280_TEMP_MSB 0xFA
#define BMP280_TEMP_LSB 0xFB
#define BMP280_TEMP_XLSB 0xFC



/*
    * BMP280 Device Specific Data
    * Data's will be retrived from the sensor during the probe()
    * And they will be stored in the computer’s memory (RAM) as this structure
    * Hence, BMP280, Device Specific Data
    * Once Probe() verified BMP280 Hardware, Probe() will 
        * Allocate memory, size of bmpdata_280
        * Assign client struct to the client passed to t (Probe(client))
        * Read and store calibration values
        * Set additional Settings
*/ 
struct bmp280_data {
    struct i2c_client* client;
    struct cdev cdev;
    struct mutex lock;
    // The following structure contains calibration data of the sensor
    struct {
        u8 chip_id;     //unsigned 8 bit
        u16 dig_T1;     //unsigned 16 bit
        s16 dig_T2;     //signed 16 bit
        s16 dig_T3;     //signed 16 bit
    } calib;
};

















#endif