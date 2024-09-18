#include <linux/module.h>   // Kernel Modules, Printk
#include <linux/kernel.h>  // Kernel Functions
#include <linux/fs.h>     // File system functions
#include <linux/cdev.h>   // For Character device registration
#include <linux/uaccess.h> // For a data transfer to/from user space
#include <linux/i2c.h>   // I2C Communication
#include <linux/init.h>  // Module init
#include <linux/slab.h>  // FOr memory Allocation (Kmalloc, Kfree)

#define DEVICE_NAME "bmp280"
#define BMP280_I2C_ADDRESS 0x76

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
static const struct i2c_device_id bmp280_id[] = {
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
    * Define the actual client Data structure to represent our I2C Client
    * This should point to an i2c client structure
    * Holds the client data, and is used to communicate with the device
    * This is the structure that is passed to the driver's probe function
*/
struct bmp280_data {
    struct i2c_client* client; //https://www.linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-i2c-client.html
    // Calibration data
    struct {
        u16 dig_T1;  // unsigned
        s16 dig_T2;  // signed
        s16 dig_T3;  // signed 
    } calib;
};

/*
    * Implement the probe function
    * the probe() function is called when the kernel finds a BMP280 sensor on the I2C bus. 
    * Passes the I2C client ==> specific BMP280 sensor on the bus.
*/
static int bmp280_probe(struct i2c_client* client, const struct i2c_device_id* id) {
    int ret;
    u8 chip_id;
    struct bmp280_data* data;

    /* #1
        * Verify BMP280 by reading the chip ID at register 0xD0
        * BMP280 has a chip ID of 0x58, check if the chip ID is 0x58
    */
    ret = i2c_smbus_read_byte_data(client, 0xD0);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to read chip ID\n");
        return ret;
    }
    chip_id = ret;

    if (chip_id != 0x58) {
        printk(KERN_ALERT "Invalid chip ID: 0x%x\n", chip_id);
        return -ENODEV;
    }
    printk(KERN_INFO "Detected BMP280 with chip ID: 0x%x\n", chip_id);

    /* #2  
        * https://www.linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-i2c-client.html
        * Allocate memory for the device-specific structure
        * Associate the data->client with the client
    */
    data = devm_kzalloc(&client->dev, sizeof(struct bmp280_data), GFP_KERNEL); //Driver model device node for the slave.
    if (!data) {
        printk(KERN_ALERT "Failed to allocate memory for device data\n");
        return -ENOMEM;
    }
    data->client = client;
    i2c_set_clientdata(client, data);

    /* #3
        * Retrieve Calibration data from the BMP280
        * Registers to read: 0x88 to 0xA1
        * Store the values in our data (Check for errors)
    */
    data->calib.dig_T1 = i2c_smbus_read_word_data(client, 0x88);  // Read 16 bits from registers 0x88 and 0x89
    data->calib.dig_T2 = i2c_smbus_read_word_data(client, 0x8A);  // Read 16 bits from registers 0x8A and 0x8B
    data->calib.dig_T3 = i2c_smbus_read_word_data(client, 0x8C);  // Read 16 bits from registers 0x8C and 0x8D

    // Check for errors while reading calibration data (BMP280 has only unsigned or signed calibration values)
    if (data->calib.dig_T1 == 0 || data->calib.dig_T2 == 0 || data->calib.dig_T3 == 0) {
        printk(KERN_ALERT "Failed to read calibration data\n");
        return -EIO;
    }
    printk(KERN_INFO "BMP280: Calibration data retrieved\n");

    /* #4
        * Configuring the sensor 
        * Registers to write to: 0xF4 (Normal Mode, 1x oversampling for temperature)
        * Value to write: 0x27
    */
    ret = i2c_smbus_write_byte_data(client, 0xF4, 0x27);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to configure sensor\n");
        return ret;
    }
    printk(KERN_INFO "BMP280: Sensor configured successfully\n");

    return 0;
};


/*
    * Implement the remove function
    * This function is called by the kernel when the device is removed
*/
static int bmp280_remove(struct i2c_client* client){
    struct bmp280_data* data = i2c_get_clientdata(client); // we will use this to get the data structure later to remove data
    //(devm_kzalloc) is automatically freed,
    printk(KERN_INFO "Removing BMP280 device\n");
    return 0;
}



/*
    * Define the driver structure to implement essential callbacks WITHIN THE KERNEL
    * This functions are used once the driver id is found by the Master
    * https://www.linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-i2c-driver.html
*/
static struct i2c_driver bmp280_driver = {
    .driver = {
            .name = "bmp280",
            .owner = THIS_MODULE,
    },
    .probe = bmp280_probe,   // This function is called when the device is found
    .remove = bmp280_remove, // This function is called when the device is removed
    .id_table = bmp280_id,   // "Hey driver, here are the devices i know how to handle"
};

   
/*
    * Define structures for Major and Minor Numbers
    * cdev structure
    * and the device class
*/
static dev_t dev_num;
static struct cdev* bmp280_cdev;
static struct class *bmp280_class;

// Defining File Ops of bmp280
/*
    * This function is called when we want to open the device file
*/
static int bmp280_open(struct inode* inode, struct file* file) {
};


/*
    * This function is called when we want to release the device file
*/
static int bmp280_release(struct inode* inode, struct file* file) {
};


/*
    * This function is called when we want to read Temp from the device file
    * we will be using client that is passed to the driver's probe function
        * To communicate with the BMP280 sensor
*/

static int bmp280_compensate_temp(struct bmp280_data *data, s32 raw_temp) {
    s32 var1, var2, T;

    var1 = ((((raw_temp >> 3) - ((s32)data->calib.dig_T1 << 1))) * ((s32)data->calib.dig_T2)) >> 11;
    var2 = (((((raw_temp >> 4) - ((s32)data->calib.dig_T1)) * ((raw_temp >> 4) - ((s32)data->calib.dig_T1))) >> 12) * ((s32)data->calib.dig_T3)) >> 14;
    
    T = var1 + var2;
    return (T * 5 + 128) >> 8;  // Return temperature in Celsius (scaled by 100)
}

static ssize_t bmp280_read(struct file* file, const char __user* buf, size_t count, loff_t* offset) {
    /* #1
        * Read the Raw Temp Data from the BMP280
        * Registers to read: 0xFA to 0xFC
        * 20 bits of data
    */

    struct bmp280_data* data = file->private_data;
    struct i2c_client* client = data->client;

    s32 raw_temp;
    u8 temp_msb; //most significant byte
    u8 temp_lsb; //least significant byte
    u8 temp_xlsb; //extra least significant byte

    int compensated_temp;
    char temp_str[10]; // temp string
    int len;

    //READ TEMP DATA
    temp_msb = i2c_smbus_read_byte_data(client, 0xFA);
    temp_lsb = i2c_smbus_read_byte_data(client, 0xFB);
    temp_xlsb = i2c_smbus_read_byte_data(client, 0xFC);

    if (temp_msb < 0 || temp_lsb < 0 || temp_xlsb < 0) {
        printk(KERN_ALERT "Failed to read temperature data\n");
        return -EIO;
    }

    raw_temp = (temp_msb << 12) | (temp_lsb << 4) | (temp_xlsb >> 4);

    /* #2
        * Apply calibration data to the raw temp data
        * Formula: T = (raw_temp / 16384.0 - data->calib.dig_T1 / 1024.0) * data->calib.dig_T2
    */
    compensated_temp = bmp280_compensate_temp(data, raw_temp);

    /* #3
        * Convert the temperature to a string
        * Copy the string to the user space
    */

    len = snprintf(temp_str, sizeof(temp_str), "%d\n", compensated_temp);
    if (*offset != 0) {
        return 0;
    }

    if (count > len - *offset) {
        count = len - *offset;
    }

    if (copy_to_user(buf, temp_str + *offset, count) != 0) {
        return -EFAULT;
    }

    *offset += count;
    return count;
};

/* 
    * Defining the FOPS stucture for the driver
*/


/*
    * This will be the file operations the Kernel calls
    * When the driver is called from the user space
    # FUNCTION PROTOTYPES
*/
struct file_operations f_ops = {
    .owner = THIS_MODULE
    .open = bmp280_open,
    .release = bmp280_release,
    .read = bmp280_read,
    .write = bmp280_write,
};


/*
    * Define the init and exit functions for the driver
    * These functions are called when the driver is loaded and unloaded
*/
static int __init bmp280_init(void) {
    /*
        * Register the I2C driver with the Kernel
    */
    int ret = i2c_add_driver(&bmp280_driver);
    if (ret) {
        printk(KERN_ALERT "Failed to register I2C driver\n");
        return ret;
    }
    printk(KERN_INFO "BMP280 driver registered successfully\n");

    /*
        * Allocate a major number dynamically
    */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) {
        printk(KERN_ALERT "Failed to allocate a Major number");
        i2c_del_driver(&bmp280_driver);
        return ret;
    }

    major = MAJOR(dev_num);
    printk(KERN_INFO "Allocated major number: %d\n", major);

    /*
        * Create and add the character device
        * Associate file operations (f_ops)
        * Add the device to the system
    */
    bmp280_cdev = cdev_alloc();
    if (!bmp280_cdev) {
        unregister_chrdev_region(dev_num, 1);
        i2c_del_driver(&bmp280_driver);  
        return -ENOMEM;
    }

    cdev_init(bmp280_cdev, &f_ops);  
    bmp280_cdev->owner = THIS_MODULE;
    ret = cdev_add(bmp280_cdev, dev_num, 1);  
    if (ret) {
        printk(KERN_ALERT "Failed to add cdev to the kernel\n");
        cdev_del(bmp280_cdev);
        unregister_chrdev_region(dev_num, 1);
        i2c_del_driver(&bmp280_driver);  
        return ret;
    }

    printk(KERN_INFO "Character device created successfully\n");
    return 0;  // Success
};    


/*
    * Remove the Chr Device
    * Unregister the driver
*/
static void __exit bmp280_exit(void) {
    cdev_del(bmp280_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "Character device unregistered\n");

    i2c_del_driver(&bmp280_driver);
    printk(KERN_INFO "BMP280 driver unregistered\n");
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jonathan");
MODULE_DESCRIPTION("Driver for BMP280 I2C Temp/Hum Sensor Module");

module_init(bmp280_init);
module_exit(bmp280_exit);
