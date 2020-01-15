#include <linux/init.h>        
#include <linux/module.h>       
#include <linux/device.h> 
#include <linux/slab.h>        
#include <linux/kernel.h>        
#include <linux/fs.h>          
#include <linux/string.h>  
#include <linux/uaccess.h> 
#include <linux/timer.h>
#include <linux/ioport.h>
#include <asm/io.h>

#define DEVICE_NAME "gonilnik"
//------------------ stvari za diodo --------------------
#define BCM2708_PERI_BASE	0x3F000000
#define GPIO_BASE   (BCM2708_PERI_BASE + 0x200000)
#define GPIO_PIN_LED 15
#define GPIO_PIN_SWITCH 14
//-------------------------------------------------------
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

// ------ stvari za preverjenje -------...............................
int switch_trigger = 1; // a je stikalo sklenjeno

int dioda_prizgana = 0; //da utripa

int dioda_ugasnjena = 1; // ali je bit na 0 ali 1

int got_mem_region;
u8 *addr;

static unsigned PORT = GPIO_BASE; /* Fizični naslov GPIO: 0x3F200000  */
static unsigned RANGE =  0xB4; 	/* Velikost V/I pomnilniškega področja za GPIO */


struct timer_list casovnik; //2 sekundni timer
//--------------------------------------------------------------------

struct file_operations fops = {
	.open = odpri,
	.release = sprosti,
	.read = beri,
	.write = pisi
};

module_init(zacni_modul);
module_exit(koncaj_modul);


//------------------- funkcija k preverja ker timer se zazene -----------------------
void dioda(struct timer_list  *timer){
		unsigned val;
		unsigned switch_val;
		
		//----------------------------------------------------------
		if(switch_trigger){
			if(dioda_ugasnjena){
				if(dioda_prizgana){
					printk("Dioda prizgana!");
					dioda_prizgana = 0;

					//---------prizgi diodo---------
					val = 1 << 15; // set GPIO15
					iowrite32(val, addr+28);
					
				}else{
					printk("Dioda ugasnjena!");
					dioda_prizgana = 1;

					//---------cleari diodo---------
					val = 1 << 15; // clear GPIO15
					iowrite32(val, addr+40);
				}
			}
			mod_timer(&casovnik, jiffies + msecs_to_jiffies(500)); // 0.5 sekundi
		}else{
			if(dioda_ugasnjena){
				if(dioda_prizgana){
					printk("Dioda prizgana!");
					dioda_prizgana = 0;

					//---------prizgi diodo---------
					val = 1 << 15; // set GPIO15
					iowrite32(val, addr+28);
				}else{
					printk("Dioda ugasnjena!");
					dioda_prizgana = 1;

					//---------cleari diodo---------
					val = 1 << 15; // clear GPIO15
					iowrite32(val, addr+40);
				}
			}
			mod_timer(&casovnik, jiffies + msecs_to_jiffies(2000)); // 2 sekundi
		}
		//----------------- switch preverjenje ---------------------
		
		switch_val = ioread32(addr+0x34);
		if ((switch_val & (1 << 14))){ 
			
			switch_trigger = 1;
		}
		else switch_trigger = 0;
}

//-----------------------------------------------------------------------------------

int zacni_modul(void){
	unsigned config;
	message = kmalloc(512 * sizeof(char), GFP_KERNEL);
	memset(message, 0, 512 * sizeof(char));
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "Registracija znakovne naprave spodletela.\n");
		return Major;
	}
	printk(KERN_INFO "Glavno stevilo je %d.\n", Major);
	//-----------------------requested memory for the PI3 ---------------------------
	if (request_mem_region(PORT, RANGE, DEVICE_NAME) == NULL) {
		printk(KERN_ALERT "Request memory ni uspel.\n");
		got_mem_region = 0; // da ga ne bomo poskusili sprostiti
	}
	else got_mem_region = 1;
	
	addr = ioremap(PORT, RANGE);
	config = 1 << 15;
	//-------------------------------- set the switch ? -----------------------------
	config &= ~(1 << 12);
	config &= ~(1 << 13);
	config &= ~(1 << 14);
	
	iowrite32(config, addr+4);
	
	printk(KERN_INFO "Memory init: success!\n");
	// --------------------- initamo timer --------------------------------
	timer_setup(&casovnik, dioda, 0);
	mod_timer(&casovnik, jiffies + msecs_to_jiffies(2000)); // 2 sekundi
	printk(KERN_INFO "Timer ustvarjen!\n");
	// --------------------------------------------------------------------------------
	return 0;
}

void koncaj_modul(void){
	kfree(message);
	unregister_chrdev(Major, DEVICE_NAME);
	if (del_timer(&casovnik))printk("Timer removed!\n");
	if (got_mem_region){release_mem_region(PORT, RANGE); printk("Memory cleared!\n");}
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
	if ( *offset >= size) return 0;

	if ( len > size - *offset)len = size - *offset;

	if ( copy_to_user( buff, message + *offset, len) )return -EFAULT;
	
	*offset += len;
	return len;
}

ssize_t pisi(struct file *filp, const char __user *buff, size_t len, loff_t *offset){
	int size = MESSAGE_SIZE;
	unsigned val;
	if ( *offset >= size)return 0;

	if ( len > size - *offset)len = size - *offset;

	if ( copy_from_user(message + *offset, buff, len) )return -EFAULT;
	*offset += len;
	message[*offset] = '\0';
	//----------------- gledamo ce je switch pritisnjen in na podlagi tega preberemo bit ----------------------------
	if(switch_trigger){
		if(message[0] == '\x00'){
			printk("---------- Ugasam diodo! ----------");
			dioda_ugasnjena = 0;
			
			//---------cleari diodo---------
			val = 1 << 15; // clear GPIO15
			iowrite32(val, addr+40);
			
		}else if(message[0] == '\x01'){
			printk("---------- Dioda prizgana! ----------");
			dioda_ugasnjena = 1;
			
		}else{
			printk("Sintax ne ustreza!");
		}
	}
	return len;
}