# BMP280 I2C Temperature Driver

## Table of Contents
1. [Introduction](#introduction)
2. [Prerequisites](#prerequisites)
3. [Connecting the BMP280 Sensor](#connecting-the-bmp280-sensor)
4. [Confirming I2C Address](#confirming-i2c-address)
5. [Driver Compilation and Loading](#driver-compilation-and-loading)
6. [Registering the Device](#registering-the-device)
7. [Reading the Sensor Data](#reading-the-sensor-data)
8. [Unloading the Driver](#unloading-the-driver)
9. [Registers Used](#registers-used)
10. [Dataset Link](#dataset-link)
11. [Troubleshooting](#troubleshooting)
12. [Motivation and Learning Experience](#motivation-and-learning-experience)

## Introduction
The BMP280 I2C Temperature Driver is a Linux kernel module that allows interfacing with the BMP280 temperature sensor via I2C. This README provides all necessary information for setting up, connecting, and reading data from the BMP280 sensor using this driver.

## Prerequisites
- Raspberry Pi (or any other platform running Linux/Embedded linux, supporting I2C communication).
- BMP280 temperature sensor; can purchased from [Amazon](https://www.amazon.ca/dp/B0CD4PQZGQ?ref=nb_sb_ss_w_as-reorder_k8_1_3&amp=&crid=2O5HL4SJU7RKQ&amp=&sprefix=bmp).
- GCC and necessary kernel headers.
- An I2C enabled kernel.

### Required Packages
Make sure to install the following:
```bash
sudo apt-get install build-essential raspberrypi-kernel-headers i2c-tools
```

## Connecting the BMP280 Sensor

The BMP280 sensor must be physically connected to the I2C bus of the Raspberry Pi.

- **VCC**: Connect to 3.3V power supply.
- **GND**: Connect to ground.
- **SCL**: Connect to I2C clock line according to your own GPIO layout.
- **SDA**: Connect to I2C data line again in accordance with your GPIO layout.

Ensure that the sensor is properly connected to avoid connection errors; I sugggest refering to the specific device specification to learn the GPIO pins.

When using Raspberrypi as a master, on rare case, the i2c protocole might have been disabled so again, check the hardware you are working with!

## Confirming I2C Address

To confirm the BMP280 sensor address on the I2C bus, use `i2cdetect`. The default I2C address for BMP280 is usually `0x76` or `0x77`.

```bash
sudo i2cdetect -y 1
```
The output should show an address, like **0x76**, indicating that the BMP280 sensor is detected. If no address is detected, recheck your connections.

## Driver Compilation and Loading

This section provides instructions to compile and load the BMP280 kernel module.

### Compilation

Ensure that you have the kernel headers installed, then use `make` to compile:

```bash
make
```
