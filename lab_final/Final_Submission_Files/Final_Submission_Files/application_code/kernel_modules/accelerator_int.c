/* ===================================================================
 *  gpio-interrupt.c
 *
 *  AUTHOR:     Mark McDermott
 *  CREATED:    March 12, 2009
 *  UPDATED:    May 2, 2017     Updated for ZED Board
 *  UPDATED:    Feb 5, 2021     Updated for the ULTRA96
 *
 *  DESCRIPTION: This kernel module registers interrupts from GPIO
 *               port and measures the time between them. This is
 *               used to also measure the latency through the kernel
 *               to respond to interrupts. 
 *               
 *  DEPENDENCIES: Works on the AVNET Ultra96 Board
 *
 */
 

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/gpio/driver.h>

#include <linux/pm_runtime.h>
#include <linux/of.h>

#define MODULE_VER "1.0"

#define ACCELERATOR_MAJOR 240                  // Need to mknod /dev/accelerator_int c 240 0
#define MODULE_NM "accelerator_interrupt"

#undef DEBUG
#define DEBUG

int             interruptcount  = 0;
int             temp            = 0;
int             len             = 0;
char            *msg            = NULL;
unsigned int    gic_interrupt;

static struct fasync_struct *fasync_accelerator_queue ;

/* ===================================================================
 * function: gpio_int_handler
 *
 * This function is the gpio_interrupt handler. It sets the tv2
 * structure using do_gettimeofday.
 */
 
//static irqreturn_t gpio_int_handler(int irq, void *dev_id, struct pt_regs *regs)

irq_handler_t accelerator_int_handler(int irq, void *dev_id, struct pt_regs *regs)
{
  interruptcount++;
  
    #ifdef DEBUG
    printk(KERN_INFO "accelerator_int: Interrupt detected in kernel \n");  // DEBUG
    #endif
  
    /* Signal the user application that an interupt occured */
  
    kill_fasync(&fasync_accelerator_queue, SIGIO, POLL_IN);

return  (irq_handler_t) IRQ_HANDLED;

}


static struct proc_dir_entry *proc_accelerator_int;

/* ===================================================================
*    function: read_proc   --- Example code
*/


ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{

    if(count>temp)
    {
        count=temp;
    }

    temp=temp-count;
    //copy_to_user(buf,msg, count);
    
    if(count==0) temp=len;
    printk("read_proc count value = %ld\n", count);
      
return count;
}


/* ===================================================================
*    function: write_proc   --- Example code
*/

ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
    //copy_from_user(msg,buf,count);
    len=count;
    temp=len;

    printk("write_proc count value = %ld\n", count);

return count;
}


/* ===================================================================
 * function: accelerator_open
 *
 * This function is called when the gpio_int device is opened
 *
 */
 
static int accelerator_open (struct inode *inode, struct file *file) {

#ifdef DEBUG
        printk(KERN_INFO "accelerator_int: Inside accelerator_open \n");  // DEBUG
#endif
    return 0;
}

/* ===================================================================
 * function: accelerator_release
 *
 * This function is called when the gpio_int device is
 * released
 *
 */
 
static int accelerator_release (struct inode *inode, struct file *file) {

#ifdef DEBUG
        printk(KERN_INFO "\naccelerator_int: Inside accelerator_release \n");  // DEBUG
#endif
    return 0;
}

/* ===================================================================
 * function: accelerator_fasync
 *
 * This is invoked by the kernel when the user program opens this
 * input device and issues fcntl(F_SETFL) on the associated file
 * descriptor. fasync_helper() ensures that if the driver issues a
 * kill_fasync(), a SIGIO is dispatched to the owning application.
 */

static int accelerator_fasync (int fd, struct file *filp, int on)
{
    #ifdef DEBUG
    printk(KERN_INFO "\naccelerator_int: Inside accelerator_fasync \n");  // DEBUG
    #endif
    
    return fasync_helper(fd, filp, on, &fasync_accelerator_queue);
}; 

/* ===================================================================
*
*  Define which file operations are supported
*
*/

struct file_operations accelerator_fops = {
    .owner          =    THIS_MODULE,
    .llseek         =    NULL,
    .read           =    NULL,
    .write          =    NULL,
    .poll           =    NULL,
    .unlocked_ioctl =    NULL,
    .mmap           =    NULL,
    .open           =    accelerator_open,
    .flush          =    NULL,
    .release        =    accelerator_release,
    .fsync          =    NULL,
    .fasync         =    accelerator_fasync,
    .lock           =    NULL,
    .read           =    NULL,
    .write          =    NULL,
};

struct file_operations proc_fops = {
    read: read_proc,
    write: write_proc
};

static const struct of_device_id zynq_accelerator_of_match[] = {
    { .compatible = "xlnx,accelerator_Lab3" }, //"xlnx,lab_2_timer"
    { /* end of table */ }
};    
    
MODULE_DEVICE_TABLE(of, zynq_accelerator_of_match);


/* ===================================================================
 *
 * zynq_gpio_probe - Initialization method for a zynq_gpio device
 *
 * Return: 0 on success, negative error otherwise.
 */

static int zynq_accelerator_probe(struct platform_device *pdev)
{
    struct resource *res;
        
    printk("In probe funtion\n");

    // This code gets the IRQ number by probing the system.

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
   
    if (!res) {
        printk("No IRQ found\n");
        return 0;
    } 
    
    // Get the IRQ number 
    gic_interrupt  = res->start;

    printk("Probe IRQ # = %lld\n", res->start);

    return 0;

}

/* ===================================================================
 *
 * zynq_gpio_remove - Driver removal function
 *
 * Return: 0 always
 */
 
static int zynq_accelerator_remove(struct platform_device *pdev)
{
    //struct zynq_gpio *gpio = platform_get_drvdata(pdev)

    return 0;
}


static struct platform_driver zynq_accelerator_driver = {
    .driver    = {
        .name = MODULE_NM,
        .of_match_table = zynq_accelerator_of_match,
    },
    .probe = zynq_accelerator_probe,
    .remove = zynq_accelerator_remove,
};


/* ===================================================================
 * function: init_accelerator_int
 *
 * This function creates the /proc directory entry gpio_interrupt.
 */
 
static int __init init_accelerator_int(void)
{

    int rv = 0;
    int err = 0;
    
    //platform_driver_unregister(&zynq_gpio_driver);
    
   
    printk("ZED Interrupt Module\n");
    printk("ZED Interrupt Driver Loading.\n");
    printk("Using Major Number %d on %s\n", ACCELERATOR_MAJOR, MODULE_NM); 

    err = platform_driver_register(&zynq_accelerator_driver);
      
    if(err !=0) printk("Driver register error with number %d\n",err);       
    else        printk("Driver registered with no error\n");
    
    if (register_chrdev(ACCELERATOR_MAJOR, MODULE_NM, &accelerator_fops)) {
        printk("accelerator_int: unable to get major %d. ABORTING!\n", ACCELERATOR_MAJOR);
    goto no_accelerator_interrupt;
    }

    proc_accelerator_int = proc_create("accelerator_interrupt", 0444, NULL, &proc_fops );
    msg=kmalloc(GFP_KERNEL,10*sizeof(char));
    
    if(proc_accelerator_int == NULL) {
          printk("accelerator_int: create /proc entry returned NULL. ABORTING!\n");
    goto no_accelerator_interrupt;
    }

    // Request interrupt
    
    // rv = request_irq(gic_interrupt, gpio_int_handler, IRQF_TRIGGER_RISING,
    rv = request_irq(gic_interrupt, 
                    (irq_handler_t) accelerator_int_handler, 
                     IRQF_TRIGGER_RISING,
                    "accelerator_interrupt", NULL);
  
    if ( rv ) {
       // printk("Can't get interrupt %d\n", INTERRUPT);
        printk("Can't get interrupt %d\n", gic_interrupt);
    goto no_accelerator_interrupt;
    }

    printk(KERN_INFO "%s %s Initialized\n",MODULE_NM, MODULE_VER);
    
    return 0;

    // remove the proc entry on error
    
no_accelerator_interrupt:
    free_irq(gic_interrupt,NULL);                   // Release IRQ    
    unregister_chrdev(ACCELERATOR_MAJOR, MODULE_NM);       // Release character device
    unregister_chrdev(ACCELERATOR_MAJOR, MODULE_NM);
    platform_driver_unregister(&zynq_accelerator_driver);
    remove_proc_entry("accelerator_interrupt", NULL);
    return -EBUSY;
};

/* ===================================================================
 * function: cleanup_accelerator_interrupt
 *
 * This function frees interrupt then removes the /proc directory entry 
 * gpio_interrupt. 
 */
 
static void __exit cleanup_accelerator_interrupt(void)
{

    free_irq(gic_interrupt,NULL);                   // Release IRQ    
    unregister_chrdev(ACCELERATOR_MAJOR, MODULE_NM);       // Release character device
    platform_driver_unregister(&zynq_accelerator_driver);  // Unregister the driver
    remove_proc_entry("accelerator_interrupt", NULL);      // Remove process entry
    kfree(msg);
    printk(KERN_INFO "%s %s removed\n", MODULE_NM, MODULE_VER);
     
}


/* ===================================================================
 *
 *
 *
 */


module_init(init_accelerator_int);
module_exit(cleanup_accelerator_interrupt);

MODULE_AUTHOR("John Zhang and Marius Facktor");
MODULE_DESCRIPTION("accelerator interrupt proc module");
MODULE_LICENSE("GPL");

