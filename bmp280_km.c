#include <linux/module.h>   // Kernel Modules, Printk
#include <linux/kernel.h>  // Kernel Functions
#include <linux/fs.h>     // File system functions
#include <linux/cdev.h>   // For Character device registration
#include <linux/uaccess.h> // For a data transfer to/from user space
#include <linux/semaphore.h> 
#include <linux/i2c.h>   // I2C Communication
#include <linux/init.h>  // Module init
#include <linux/slab.h>  // FOr memory Allocation (Kmalloc, Kfree)

define DEVICE_NAME "utg"
define BMP280_I2C_ADDRESS 0x76

/*
    * Define device id structure
    * This tells the kernel which devices this driver supports.
    * "These are the devices i know how to handle"
*/
static const struct i2c_device_id bmp280_id[] = {
    {"bmp280", 0},
    {} // newer version of the driver
};
MODULE_DEVICE_TABLE(i2c, bmp280_id);    // Register Device Table


/*
    * Define the actual clinet structure to represent our I2c Client
    * This should point to an i2c client structure
*/
struct bmp280_data {
    struct i2c_client* client;
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
}

   
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
}


/*
    * This function is called when we want to release the device file
*/
static int bmp280_release(struct inode* inode, struct file* file) {
}


/*
    * This function is called when we want to read from the device file
*/
static ssize_t bmp280_read(stuct file* file, const char __user* buf, size_t count, loff_t* offset) {
}

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



static int __init driverInit(void) {

    /*
        * Allocate a major number dynamically
        * Pointer to dev_t, minorno, count(no of minor no), const char device name
    */
    int ret = _
    if (ret) {
        printk(KERN_ALERT "Failed to allocate a Major number");
        return ret;
    }

    major = MAJOR(dev_n);
    minor = MINOR(dev_n);

    printk(KERN_INFO "DEVICE --> %s, [Major = %d], [Minor = %d]\n", DEVICE_NAME, major, minor);
    printk(KERN_INFO "Use mknod or modprobe to insert the device file");

    /*
    * Now we can create a chr device and associate it with the dev_n
    */

    my_char_dev = cdev_alloc();
    my_char_dev->ops = &f_ops;
    my_char_dev->owner = THIS_MODULE; 

    ret = cdev_add(my_char_dev, dev_n, 1); /* Add device to the kernel */
    if (ret) {
        printk(KERN_ALERT "Unable to add cdev to the Kernel");
        return ret;
    }

    sema_init(&virtual_dev.sem, 1);

    return 0;
}    

static void __exit driverExit(void) {
    cdev_del(my_char_dev);
    unregister_chrdev_region(dev_n, 1);
    printk(KERN_INFO "Unloading Kernel Module");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jonathan");
MODULE_DESCRIPTION("Driver for BMP280 I2C Temp/Hum Sensor Module");

module_init(driverInit);
module_exit(driverExit);
