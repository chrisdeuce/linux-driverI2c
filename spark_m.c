#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <uasm/uacces.h>  //used to move data to from kernel to user space
#include <linux/spinlock.h>
#include <asm/aotmic.h>



#define DRV_NAME "sparkfun_hmb"

//Register functions
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team StationX");
MODULE_DESCRIPTION("Homebrew driver for linux diploma curse");
MODULE_VERSION("0.1");

static int sparkfun_hmb_open()
{

}

static atomic 
