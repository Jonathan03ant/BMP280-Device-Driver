#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/slab.h>

define DEVICE_NAME "utg"
define BMP280_I2C_ADDRESS 0x76


/* 
    * We need the following to register a device file
    * cdev to register our char device file
    * a major and minor no
    * dev_t to hold major and minor no
*/

static struct cdev *my_char_dev;
static int major;
static int minor = 0;
dev_t dev_n;

/*
    * struct to represent our device specific data (clinet)
*/


struct bmp280_data {
               struct i2c_client *client;
}

/*
    * Define device id structure
    * This implements a single driver structure, and instantiate all clients from it.
*/

static const struct i2c_device_id bmp280_id[] = {
    {"bmp280", 0},
    {}
};

//The following are function prototypes needed for our i2c_device

static int bmp280_probe(s);
static int bmp28_remove();
static int bmp280_read_register();
static int bmp280_write_to_register();

/*
    *  This will be the file operations the Kernel calls
    *  when the driver is called from the user space
    # FUNCTION PROTOTYPES
*/

static int deviceOpen(struct inode *inode, struct file *instance);
static int deviceRelease(struct inode *inode, struct file *instance);
static ssize_t deviceRead(struct file *filp, char *buffer, size_t buffcnt, loff_t *off_p);
static ssize_t deviceWrite(struct file *filp, const char *buffer, size_t buffcnt, loff_t *off_p);

struct file_operations f_ops = {
    .owner = THIS_MODULE
    .open = deviceOpen,
    .release = deviceRelease,
    .read = deviceRead,
    .write = deviceWrite,
};

/* 
   * Now we can define our operations we defined above
  
*/

int deviceOpen(struct inode *inode, struct file *instance) {
    if (down_interruptible(&virtual_dev.sem) != 0) {
        printk(KERN_ALERT "Device running already, CANNOT be locked");
        return -1;
    }

    printk(KERN_INFO "Device Open() is called");
    return 0;
}

int deviceRelease(struct inode *inode, struct file *instance) {
    up(&virtual_dev.sem);
    printk(KERN_INFO "Device close() called, closing device");
    return 0;
}

/*
    * KERNEL---->USER
*/

ssize_t deviceRead(struct file *filp, char *buffer, size_t buffcnt, loff_t *off_p) {
    printk(KERN_INFO "Device read() is called, reading from driver");
    int ret = copy_to_user(buffer, virtual_dev.data, buffcnt);  // (to, from, n)
    return ret;
}

/*
    * USER---->KERNEL
*/
ssize_t deviceWrite(struct file *filp, const char *buffer, size_t buffcnt, loff_t *off_p) {
    printk(KERN_INFO "Device write() is called, writing to driver.");
    int ret = copy_from_user(virtual_dev.data, buffer, buffcnt); // (to, from, n)
    return ret;
}

/* 
    * This function will be called when we initialize the driver
    * First we call alloc_chrdev_region() to allocate range of numbers for our device
    * Next, we call cdev_alloc() to create a chr device structure
    * this chrdev will have fops we defined during earlier stages
    * Finally we call cdev_add to associate the cdev we created with the numbers we allocated earlier.
*/

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
