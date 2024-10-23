#include <linux/module.h>   // Kernel Modules, Printk
#include <linux/kernel.h>  // Kernel Functions
#include <linux/fs.h>     // File system functions
#include <linux/cdev.h>   // For Character device registration
#include <linux/uaccess.h> // For a data transfer to/from user space
#include <linux/i2c.h>   // I2C Communication
#include <linux/init.h>  // Module init
#include <linux/slab.h>  // FOr memory Allocation (Kmalloc, Kfree)
#include <linux/device.h>

#include "bmp280_km.h"

/*
    * Define structures for Major and Minor Numbers
    * cdev structure
    * and the device class
*/
static dev_t dev_num;
static struct cdev* bmp280_cdev;
static struct class *bmp280_class;

/*
    * Define device id structure
    * This tells the kernel which devices this driver supports.
    * This driver supports an i2c device name called "bmp280"
    * "I'm the driver
        These are the devices i know how to handle,     
        add me to your list right?
        and if you find me, Please call My Probe function
        Heres my Contact Info"
*/
static const struct i2c_device_id bmp280_id[] = 
{
    {"bmp280", 0},
    {} // newer version of the driver
};

/*
    * Might Kernel adding the table to the list of devices it knows how to handle
    * "I got you bro, I'll call you when i find a device that matches your id on the Bus"
    * "I'll add you to the list of drivers i know how to handle"
*/
MODULE_DEVICE_TABLE(i2c, bmp280_id);    

/*
    * This function is called when __KERNEL finds BMP280 sensor on the I2C Bus
    * Initializes/Prepares the sensor before any operation could take palce
    * Simplified Overflow
        * 1. Verify hardware presence by reading CHIP ID (Register 0xD0)
        * 2. Allocate memory for bmp280_data which is used to store device specific data 
        * 3. Assign data0>client to client passed
        * 4. Read the calibration data from hardware; store it in the bmp280_data
        * 5. Additional config
            * Set to Normal Mode and oversampling
*/
static int bmp280_probe(struct i2c_client* client) 
{
    struct bmp280_data *data;
    int ret;
    u8 chip_id;

    /* #1
        * Verify BMP280 by reading the chip ID at register 0xD0
        * BMP280 has a chip ID of 0x58, check if the chip ID is 0x58
    */
    ret = i2c_smbus_read_byte_data(client, BMP280_REG_CHIPID);
    if (ret < 0) 
    {
        printk(KERN_ALERT "Failed to read chip ID\n");
        printk(KERN_INFO " __probe(), i2c_smbus_read => chipID");
        return ret;
    }

    chip_id = ret;
    if (chip_id != 0x58) 
    {
        printk(KERN_ALERT "Invalid chip ID: 0x%x\n", chip_id);
        printk(KERN_INFO " __probe(), chipID0x8!=ret");
        return -ENODEV;
    }
    printk(KERN_INFO "Detected BMP280 with chip ID: 0x%x\n", chip_id);
    printk(KERN_INFO " __probe(), step one complete");

    /* #2  
        * https://www.linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-i2c-client.html
        * Allocate memory for the device-specific data
        * dev_kzalloc https://www.unix.com/man-page/suse/9/devm_kzalloc (is freeed automatically)
        * Now we have our hardware data living in our computer memory
    */
    data = devm_kzalloc(&client->dev, sizeof(struct bmp280_data), GFP_KERNEL);   
    if (!data) {
        printk(KERN_ALERT "Failed to allocate memory for device data\n");
        printk(KERN_INFO " __probe(), dev_kzalloc => MemoryError");
        return -ENOMEM;
    }
    printk(KERN_INFO " __probe(), step two complete");

    /* #3
        * Assign Proper data
    */
    data->client = client;
    i2c_set_clientdata(client, data);
    printk(KERN_INFO " __probe(), step three complete");

    /* #4
        * Retrieve Calibration data from the BMP280
        * Registers to read: 0x88 to 0xA1
        * Store the values in our data (Check for errors)
    */
    ret = i2c_smbus_read_word_data(client, 0x88);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read calibration data (dig_T1)\n");
        return ret;
    }
    data->calib.dig_T1 = ret;

    ret = i2c_smbus_read_word_data(client, 0x8A);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read calibration data (dig_T2)\n");
        return ret;
    }
    data->calib.dig_T2 = ret;

    ret = i2c_smbus_read_word_data(client, 0x8C);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read calibration data (dig_T3)\n");
        return ret;
    }
    data->calib.dig_T3 = ret;

    printk(KERN_INFO "BMP280: Calibration data retrieved successfully\n");

    /* #5
        * Configuring additional settings
        * Registers to write to: 0xF4 (Normal Mode, 1x oversampling for temperature)
        * Value to write: 0x27
    */
    ret = i2c_smbus_write_byte_data(client, 0xF4, 0x27);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to configure sensor\n");
        printk(KERN_INFO " __probe(), i2c_smbus_write => NrmlXOvrSmpl(XXX)");
        return ret;
    }
    printk(KERN_INFO "BMP280: Sensor configured successfully\n");
    printk(KERN_INFO " __probe(), step four complete");
    return 0;
};


/*
    * Implement the remove function
    * This function is called by the kernel when the device is removed from the system
    * Or unloaded
*/
static void bmp280_remove(struct i2c_client *client)
{
    //(devm_kzalloc) is automatically freed,
    printk(KERN_INFO "Removing BMP280 device\n");
    return;
};


/*
    * Define the driver structure to implement essential callbacks WITHIN THE KERNEL
    * This functions are used once the driver id is found by the Master
    * https://www.linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-i2c-driver.html
*/
static struct i2c_driver bmp280_driver =
{
    .driver = {
            .name = "bmp280",
            .owner = THIS_MODULE,
    },
    .probe = bmp280_probe,   // This function is called when the device is found
    .remove = bmp280_remove, // This function is called when the device is removed
    .id_table = bmp280_id,   // "Hey driver, here are the devices i know how to handle"
};

   
/*
    * This function is called when we want to open the device file
    * From the userspace, this could be open()
    * We ensure the device is available, and is ready (Implies Probe() worked)
    * Retrieve the bmp280_data structure that contains the cdev.
*/
static int bmp280_open(struct inode* inode, struct file* file) 
{
    struct bmp280_data* data = container_of(inode->i_cdev, struct bmp280_data, cdev);   //Retrive data from inode ,with cdev
    file->private_data = data;
    return 0;
};


/*
    * This function is called when we want to close the device file
    * from userspace, this could be close()
    * As of now, nothing complicated is needed
*/
static int bmp280_release(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "BMP280 device closed\n");
    return 0;
};


/*
    * This function is called when _USR wants to read Temp from the  driver's device file
    * Copies temp data from kernel to user and returns the no of bytes read
    * Simplified Flow
        * 1. Read raw temperature (20 bit) data
        * 2. Apply the calibration (Static) value and compensate the temperature 
            * Calculation from the datasheet 
        * 3. Change the final final value to string, 
        * 4. copy value from KernelSpaceBuffer to UserSpaceBuffer
*/
static ssize_t bmp280_read(struct file* file, char __user* buf, 
                                        size_t count, loff_t* offset) 
{
    struct bmp280_data* data = file->private_data;  // file->private_data already contains data, because it is opned()
    struct i2c_client* client = data->client;
    int ret;
    int32_t raw_temp;                               // Possible Type Error  Raw temperature from registers (20 bit value)
    int32_t actual_temp;                            // Possible Type Error s32Maybe  Actual temperature (in °C)
    int32_t var1; 
    int32_t var2;                                   // Possible Type Error s32Maybe
    char temp_str[16];                              // Buffer to store the temp
    
    /* #1
        * Read the Raw Temp Data from the BMP280
        * Registers to read: 0xFA to 0xFC
        * 20 bits of data
    */
    raw_temp = (i2c_smbus_read_byte_data(client, BMP280_TEMP_MSB) << 12) |
              (i2c_smbus_read_byte_data(data->client, BMP280_TEMP_LSB) << 4)  |
              (i2c_smbus_read_byte_data(data->client, BMP280_TEMP_XLSB) >> 4);

    if (raw_temp < 0)
    {
        printk(KERN_ALERT "Failed to read temperature data from BMP280");
        printk(KERN_ALERT " __read(), i2c_smbus_read => raw_temp");
        return -EIO;
    }

    /* #2
        * Use the calibration data(Static) from bmp280_data (Read from registers)
        * Apply the compensation formulla
    */
    int32_t dig_T1 = data->calib.dig_T1;
    int32_t dig_T2 = data->calib.dig_T2;
    int32_t dig_T3 = data->calib.dig_T3;

    var1 = (((raw_temp / 8) - (dig_T1 * 2)) * dig_T2) / 2048;
    var2 = (((((raw_temp / 16) - dig_T1) * (raw_temp / 16 - dig_T1)) / 4096) * dig_T3) / 16384;
    actual_temp = (var1 + var2) / 5120;    // resulting temp in °C


    /* #3
        * Convert the temperature to a string
        * Copy the string to the user space
    */
    snprintf(temp_str, sizeof(temp_str), "%d.%02d\n", actual_temp / 100, actual_temp % 100);
    ret = copy_to_user(buf, temp_str, strlen(temp_str) + 1);
    if (ret)
    {
        printk(KERN_ALERT "Failed to copy KernelBuffer(temp_str) to UserSpaceBuffer(buf)");
        printk(KERN_ALERT "__read(), copy_to_usr => Kernel to User");
        return -EFAULT;
    }

    return strlen(temp_str) + 1;
};


/*
    * fops are used when we initialize a cdev structure
    * This will be the file operations the Kernel calls
    * When the driver is called from the user space
    # FUNCTION PROTOTYPES
*/
struct file_operations f_ops = 
{
    .owner = THIS_MODULE,
    .open = bmp280_open,
    .release = bmp280_release,
    .read = bmp280_read,
    //.write = bmp280_write,
};


/*
    * __init is called when the driver is loaded using sudo insmode .ko
    * Simplified flow
        * 1. Register the i2c driver first
        * 2. Allocate Maj and Min no (dynamicaly)
        * 3. cdev flow
            * # create cdev   cdev_alloc()
            * # Init allocated cdev with our fops
            * # Associate cdev with dev_t (device no)
*/
static int __init bmp280_init(void) 
{
    /*
        * Register the I2C driver with the Kernel
    */
    int major;
    int ret = i2c_add_driver(&bmp280_driver);
    if (ret) {
        printk(KERN_ALERT "Failed to register I2C driver\n");
        printk(KERN_INFO " __init(), i2c_add_driver(XXXX)");
        return ret;
    }
    printk(KERN_INFO "BMP280 driver registered successfully\n");

    /*
        * Allocate a major number dynamically
    */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) {
        printk(KERN_ALERT "Failed to allocate a Major number");
        printk(KERN_INFO " __init(), alloc_chrdev_region(XXXX)");
        i2c_del_driver(&bmp280_driver); //undo
        return ret;
    }

    //major = MAJOR(dev_num);
    printk(KERN_INFO "Allocated major number: %d\n", major);

    /*
        * Initializw the character device and add it to the syttesm
        * Associate file operations (f_ops)
        * Add the device to the system
    */
    bmp280_cdev = cdev_alloc();
    if (!bmp280_cdev) {
        printk(KERN_INFO " __init(), cdev_alloc(XXXX)");
        unregister_chrdev_region(dev_num, 1);
        i2c_del_driver(&bmp280_driver);  
        return -ENOMEM;
    }

    cdev_init(bmp280_cdev, &f_ops);  
    bmp280_cdev->owner = THIS_MODULE;

    ret = cdev_add(bmp280_cdev, dev_num, 1);  
    if (ret) {
        printk(KERN_ALERT "Failed to add cdev to the kernel\n");
        printk(KERN_INFO " __init(), cdev_add(XXXX)");
        cdev_del(bmp280_cdev);
        unregister_chrdev_region(dev_num, 1);
        i2c_del_driver(&bmp280_driver);  
        return ret;
    }

    printk(KERN_INFO "Character device created successfully\n");

    // 4. Create a device class
    bmp280_class = class_create(DEVICE_NAME);
    if (IS_ERR(bmp280_class)) {
        cdev_del(bmp280_cdev);
        unregister_chrdev_region(dev_num, 1);
        i2c_del_driver(&bmp280_driver);
        return PTR_ERR(bmp280_class);
    }

    // 5. Create a device node in /dev
    device_create(bmp280_class, NULL, dev_num, NULL, DEVICE_NAME);
    printk(KERN_INFO "BMP280: Device node created in /dev\n");

    return 0;  // Success
};    


/*
    * __init is called when the driver is loaded using sudo rmode .ko
    * Remove the Chr Device
    * Unregister the driver
*/
static void __exit bmp280_exit(void) {
    // 1. Remove the device node
    device_destroy(bmp280_class, dev_num);
    
    // 2. Destroy the device class
    class_destroy(bmp280_class);

    // 3. Delete the character device
    cdev_del(bmp280_cdev);

    // 4. Unregister the character device number
    unregister_chrdev_region(dev_num, 1);
    
    // 5. Unregister the I2C driver
    i2c_del_driver(&bmp280_driver);
    
    printk(KERN_INFO "BMP280: Driver unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jonathan Antenanie");
MODULE_DESCRIPTION("Driver for BMP280 I2C Temp/Hum Sensor Module");

module_init(bmp280_init);
module_exit(bmp280_exit);
