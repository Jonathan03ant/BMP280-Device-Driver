#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#define DEVICE_NAME = "utg"



/* 
	* We need the following to register a device file
	* cdev to register our char device file
	* a major and minor no
	* dev_t to hold major and minor no
*/

static cdev *my_char_dev;
static int major;
static int minor = 0;
dev_t dev_n;

/*
*	*struct to represent our device file
*	*this is temporary
*/

struct my_device {
	char data[256];
	struct semaphore sem;
}
