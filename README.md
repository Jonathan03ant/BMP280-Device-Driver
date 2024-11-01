# BMP280 Device Driver Documentation

## Prerequisites
- **Raspberry Pi** (or any other platform running Linux/Embedded linux, supporting I2C communication).
- **BMP280 temperature sensor**: can purchased from [Amazon](https://www.amazon.ca/dp/B0CD4PQZGQ?ref=nb_sb_ss_w_as-reorder_k8_1_3&amp=&crid=2O5HL4SJU7RKQ&amp=&sprefix=bmp).
- **GCC and necessary kernel headers**.
- **An I2C enabled linux kernel**.

## Connecting the BMP280 Sensor

The BMP280 sensor must be physically connected to the I2C bus of the Raspberry Pi.

- **VCC**: Connect to 3.3V power supply.
- **GND**: Connect to ground.
- **SCL**: Connect to I2C clock line (e.g., GPIO3/SCL).
- **SDA**: Connect to I2C data line (e.g., GPIO2/SDA).

Ensure that the sensor is properly connected to avoid connection errors.

## Confirming I2C Address

To confirm the BMP280 sensor address on the I2C bus, use `i2cdetect`. The default I2C address for BMP280 is usually `0x76` or `0x77`.

```bash
sudo i2cdetect -y 1
```

The output should show an address, like `0x76`, indicating that the BMP280 sensor is detected. If no address is detected, recheck your connections.

## Driver Compilation and Loading

This section provides instructions to compile and load the BMP280 kernel module.

### Cloning the repository

The first step is always to clone this repository and save it inside your Raspberry Pi (or other platform) prefered directory.

```bash
cd /path/to/file
git clone git@github.com:Jonathan03ant/BMP280-Device-Driver.git
cd BMP280-Device-Driver
```

### Compilation

Ensure that you have the kernel headers installed, then use `make` to compile:

```bash
make
```

If compilation is successful, you should see a `.ko` (kernel object) file generated.
.ko represents the driver code which is compiled and ready to be loaded.
We will use this Machine Code to load the module dynamically.

### Loading the Driver

To load the compiled driver:

```bash
sudo insmod bmp280_km.ko
```

After loading, check the kernel log using `dmesg` to confirm that the driver has been successfully loaded:

```bash
dmesg | tail
```

Expected output:

```
[BMP280]: Driver registered successfully
[BMP280]: Device node created in /dev
```

## Registering the Device

The BMP280 device will automatically be registered with the kernel when the probe function executes successfully. To verify:

- Check if `/dev/bmp280` exists:

```bash
ls -l /dev/bmp280
```

This indicates that the device file has been created for user interaction.

## Reading the Sensor Data

You can read the temperature data from the device using standard file read commands in Linux.

### Using Command Line

To read temperature data:

```bash
sudo cat /dev/bmp280
```

The output should display the temperature value in Celsius.

### Using Python

Below is a Python example to read temperature data from `/dev/bmp280`.

```python
import os

DEVICE_FILE = "/dev/bmp280"

def read_from_device():
    try:
        with open(DEVICE_FILE, 'r') as file:
            data = file.read()
            print(f"Read from device: {data}")
    except IOError as e:
        print(f"Error reading from file: {e}")

if __name__ == "__main__":
    read_from_device()
```

Run the script with:

```bash
sudo python3 userspace_app.py
```

Ensure the device is correctly registered before running the script.

## Unloading the Driver

If you need to remove the driver from the kernel, you can do so using `rmmod`.

### Unload Driver

```bash
sudo rmmod bmp280_km
```

If you get an error about the module being in use, ensure there are no processes accessing `/dev/bmp280`.

To check processes using the module:

```bash
sudo lsof | grep bmp280
```

Then kill the processes that are holding onto the module and try unloading again.

## Registers Used

The BMP280 sensor uses various registers to communicate data. Below are some of the important registers that the driver interacts with:

- **Register 0xD0 (CHIP ID)**: Used to verify the sensor's identity. Expected value is `0x58`.
- **Registers 0xFA to 0xFC**: Temperature data registers, which store the 20-bit temperature data.
- **Register 0x88 to 0xA1**: Calibration data registers used for temperature compensation.
- **Register 0xF4**: Control register used to configure the sensor (e.g., Normal mode, oversampling).

### Reading CHIP ID

To verify the BMP280 sensor, the driver reads the CHIP ID register at `0xD0`.

```c
ret = i2c_smbus_read_byte_data(client, BMP280_REG_CHIPID);
if (ret != 0x58) {
    printk(KERN_ALERT "Invalid chip ID: 0x%x\n", ret);
    return -ENODEV;
}
```

This code ensures that the driver is indeed communicating with a BMP280 sensor.

## Dataset Link

The BMP280 driver uses static calibration data which is stored in its internal registers. For a detailed understanding of calibration and sensor usage, refer to the official datasheet: [BMP280 Datasheet](https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/)

## Troubleshooting

### Issue: `No such device`

- Ensure that the device is correctly connected.
- Ensure that the I2C bus is enabled (`sudo raspi-config` for Raspberry Pi).

### Issue: `BMP280 client data is NULL`

- This indicates that the probe function did not successfully initialize the device. Check `dmesg` for details.

### Issue: Module In Use

- Check open processes with `lsof`:

```bash
sudo lsof | grep bmp280
```

- If necessary, force unload:

```bash
sudo rmmod -f bmp280_km
```

## Motivation and Learning Experience

I built this driver to learn about device files and I2C communication in the Linux kernel. This experience taught me a lot about kernel modules, handling hardware through I2C, and interacting with user space via device files. It motivated me to continue striving and learning more about Linux and driver development, pushing me towards gaining a deeper understanding of how low-level systems work.

## Current Progress

Currently, the driver only supports reading temperature so I plan on using its pressure registers to read the pressure as well!

## Conclusion

This README covered everything from sensor connection to reading data and unloading the module. For more information, refer to the kernel logs (`dmesg`) or the datasheet.

## Contact

For questions, feel free to create an issue on GitHub.

Stay Lowlevel!!! 
