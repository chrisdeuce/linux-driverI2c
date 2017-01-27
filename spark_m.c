/*
Información del driver
Driver para utilizar la estación meteorologica I2C sparkfun weathershield 
*/
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
#include <linux/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h> // copy to user ; copy from user

//#define DRV_NAME "spark"

//Register functions
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team StationX_FRM");
MODULE_DESCRIPTION("Homebrew driver for sparkfun weather station");
MODULE_VERSION("0.1");

/*
 1.  Creando la estructura del dispositivo
*/
struct spark_device {
  char data[100];
  struct data[100];
}virtual device;

/*
2. Registrando el dispositivo
*/
struct cdev *sparkdev
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "sparkfun"
/*
3. Registrando el dispositivo
*/

static int driver_entry(void){
  //Paso 1, usar asignación automática, utilizando major_number
  ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
  if(ret<0){ /* verificamos si hay error al registrar el dispositivo*/
    printk(KERN_ALERT "Sparkfun failed to allocate a major number");
    return ret;
  }
  major_number = MAJOR(dev_num); //extrae el número mayor que se almacena en la variable (MACRO)
  printk(KERN_INFO "Sparkfun: major number is: %d",major_number);
  printk(KERN_INFO "\tuse \"mkmod /dev/%s c %d 0\"for device file",DEVICE_NAME,major_number); //dmesg
  //Paso 2, crear la estructura cdev
  mcdev =cdev_alloc();
  mcdev->ops=&fops;
  mcdev-owner = THIS_MODULE;
  /*creación de cdev*/
  ret =cdev_add(mcdev,dev_num,1);
  if(ret<0){
      printk(KERN_ALERT "Sparkfun: incapaz de agregar cdev en el kernel")
      return ret;
  }
  return 0;
}



/*Iniciando el driver*/
static int _init spark_init(void)
{
  return i2c_add_driver(&spark_driver);
}
module_init(spark_init);

static void _exit spark_cleanup(void)
{
  i2c_del_driver(&spark_driver);
}
module_exit(spark_cleanup);

/*Comunicación básica*/

int i2c_master_send(struct i2c_client *client, const char *buf, int count);
int i2c_master_recv(struct i2c_client *client, char *buf, int count);


static struct spark_algorithm spfun_algorithm = {
  .name          = "fun_algorithm",
  .id            = I2C_ALGO_SMBUS,
  .smbus_xfer    = fun_access,
  .functionality = spfun_func,
};

static u32 spfun_func(struct spark_adapter *adapter)
{
  return I2C_FUNC_SMBUS_QUICK |
         I2C_FUNC_SMBUS_BYTE |
         I2C_FUNC_SMBUS_BYTE_DATA |
         I2C_FUNC_SMBUS_WORD_DATA |
         I2C_FUNC_SMBUS_BLOCK_DATA;
}

static s32 spfun_access(struct spark_adapter *adap,
			u16 addr,
			unsigned short flags,
			char read_write,
			u8 command,
			int size,
			union i2c_smbus_data *data)
{
  int i, len;
  dev_info(&adap->dev, "%s was called with the"
	   "following parameters:\n",
	   __FUNCTION__);
  dev_info(&adap->dev, "addr = %.4x\n",addr);
  dev_info(&adap->swv, "flags = %.4x\n",flags)
}
