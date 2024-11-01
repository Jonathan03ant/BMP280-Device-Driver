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
