#include <linux/init.h>        
#include <linux/module.h>       
#include <linux/device.h> 
#include <linux/slab.h>        
#include <linux/kernel.h>        
#include <linux/fs.h>          
#include <linux/string.h>  
#include <linux/uaccess.h> 

#define DEVICE_NAME "gonilnik"
#define MESSAGE_SIZE 512

MODULE_LICENSE("GPL");

int zacni_modul(void);
void koncaj_modul(void);
int odpri(struct inode *, struct file *);
int sprosti(struct inode *, struct file *);
ssize_t beri(struct file *, char *, size_t, loff_t *);
ssize_t pisi(struct file *, const char *, size_t, loff_t *);

int Major;
static char   *message;     	///< Memory for the string that is passed from userspace
//static short  size_of_message;  ///< Used to remember the size of the string stored
static int    numberOpens = 0;  ///< Counts the number of times the device is opened


struct file_operations fops = {
	.open = odpri,
	.release = sprosti,
	.read = beri,
	.write = pisi
};

module_init(zacni_modul);
module_exit(koncaj_modul);

int zacni_modul(void){
	message = kmalloc(512 * sizeof(char), GFP_KERNEL);
	memset(message, 0, 512 * sizeof(char));
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "Registracija znakovne naprave spodletela.\n");
		return Major;
	}
	printk(KERN_INFO "Glavno stevilo je %d.\n", Major);
	return 0;
}

void koncaj_modul(void){
	kfree(message);
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "Gonilnik: Bye bye!\n");
}

int odpri(struct inode *inode, struct file *file){
	numberOpens++;
	printk(KERN_INFO "Gonilnik: Device has been opened %d time(s)\n", numberOpens);
	return 0; 
}

int sprosti(struct inode *inode, struct file *file){ 
	return 0; 
}

ssize_t beri(struct file *filp, char __user *buff, size_t len, loff_t *offset){
	int size = strlen(message);
	if ( *offset >= size)return 0;

	if ( len > size - *offset)len = size - *offset;

	if ( copy_to_user( buff, message + *offset, len) )return -EFAULT;

	*offset += len;
	return len;
}

ssize_t pisi(struct file *filp, const char __user *buff, size_t len, loff_t *offset){
	int size = MESSAGE_SIZE;
	if ( *offset >= size)return 0;

	if ( len > size - *offset)len = size - *offset;

	if ( copy_from_user(message + *offset, buff, len) )return -EFAULT;
	*offset += len;
	message[*offset] = '\0';
	return len;
}