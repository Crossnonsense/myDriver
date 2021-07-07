#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#define  DEVICE_NAME "myDriver"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "my"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Georgiy Voronkov");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("My first charecter driver");  ///< The description -- see modinfo
MODULE_VERSION("1.0");            ///< A version number to inform users

unsigned int  val = 0;                      ///< Counts the number of times the iformation is read from the device
static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  myDriverClass  = NULL; ///< The device-driver class struct pointer
static struct device* myDriverDevice = NULL; ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};
 
void u_int_to_char (unsigned int number, char* res)
{
   int i;
   int j = 9;
   for (i=0; i < 9; i++)
   {
	res[i]='0';
   }
   
   while (number>0)
   {
	int digit = number % 10;
	res[j] = '0' + digit;
	number /= 10;
	j = j -1;
   }
}
static int __init myDriver_init(void){
   printk(KERN_INFO "myDriver: Initializing the myDriver LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "myDriver failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "myDriver: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   myDriverClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(myDriverClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(myDriverClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "myDriver: device class registered correctly\n");

   // Register the device driver
   myDriverDevice = device_create(myDriverClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(myDriverDevice)){               // Clean up if there is an error
      class_destroy(myDriverClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(myDriverDevice);
   }
   printk(KERN_INFO "myDriver: device class created correctly\n"); // Made it! device was initialized
   return 0;
}

static void __exit myDriver_exit(void){
   device_destroy(myDriverClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(myDriverClass);                          // unregister the device class
   class_destroy(myDriverClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "myDriver: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "myDriver: Device has been opened time(s)\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   char val_c[10] ="";
   val++; 
   u_int_to_char(val, val_c);
   error_count = copy_to_user(buffer, val_c, len);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "myDriver: Sent %u number to the user\n", val);
      return 0;  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "myDriver: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}


static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "myDriver: Device successfully closed\n");
   return 0;
}


module_init(myDriver_init);
module_exit(myDriver_exit);
