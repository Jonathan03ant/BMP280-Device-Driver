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

/*
*	*  This will be the file operations the Kernel cal
*	*  when the driver is called from the user space)
*/


struct file_operations f_ops = {
	.owner = THIS_MODULE,
	.release = deviceRelease,
	.read = deviceRead,
	.write = deviceWrite,	
	.open = deviceOpen,
}

/* 
*	*Now we can define our operation we defined above
*	
*/

int deviceOpen(struct inode *inode, struct file *instance){
	if (down_interruptible(&vertual_dev.sem) != 0) {
		printk(KERN_ALERT "Device running already, CANNOT be locked");
			return -1
	}

	printk(KERNEL_INFO "Device Open() is called");
	return 0
}


int deviceRelease(struct inode *inode, struct file *instance) {
	up(&vertual_dev.sem);
	printk(KERNEL_INFO "Device close() called, closing device");
	return 0;
}

/*
	* KERNEL---->USER
	*/

ssize_t device_read( stuct file *filp, chat *buffer, size_t buffcnt, loff_t *off_p) {
	printk(KERNEL_INFO "Device read() is called, reading from driver");
	int ret = copy_to_user(buffer, vertula_dev.data, buffcnt);  # (to, from, n)


	return ret;
}

/*
	* USER---->KERNEL
*/
ssize_t deviceWrite(struct file *filp, const chat *buffer, size_t buffcnt, loff_t *off_p){
	printk(KERNEL_INFO "Device write() is called, writing to driver.");
	int ret = copy_from_user(vertual_dev.data, buffer, buffercnt); # (to, from , n)

	return ret;
}

