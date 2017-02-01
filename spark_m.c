/*
**Información del driver
**date:    Jan 20 2017
**version: 1.0
**brief:  Driver para utilizar la estación meteorologica I2C sparkfun weathershield 
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/stddef.h>
#include <linux/spinlock.h>
#include <linux/config.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>      // copy to user ; copy from user
#define DEVICE_NAME "spark"   // El dispositivo aparecera en /dev/spark/
#define CLASS_NAME "spk"      // Tipo de dispositivo
#define REG_PRESS 0X01        // Registro en donde se lee la presión
#define REG_TEMP 0X04         // Registro en donde se lee la temperatura


static struct spark_data data;
static unsigned int irqNumber;
static u8 sensor_input[2];
static unsigned int irqNumber;

struct i2c_dev {
  struct list_head list;
  struct i2c_Adapter *adap;
  struct device *dev;
}


static void sparkmod_handler(struct sp_struct *s);

static struct spark_data_struct {
  u8 tempdata;
  u8 pressdata;
  unsigned int irq;
} spark_data;


static int majorNumber;                     // Almacena el número de dispositivo
static char message[256] ={0};              // Memoria para la cadena que pasa del espacio de usuario
static short size_of_message;               // Utilizado para saber el tamaño de la cadena almacenada
static int numberOpens =0;                  // Contador de uso del dispositivo
static struct class* sparkClass = NULL;     // puntero de la clase del dispositivo
static struct device* sparkDevice = NULL;   // puntero a la estructura del dispositivo



/* Funciones prototipo para el driver */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *,char *, size_t,loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t*);

/*Brief devices*/
static struct file_operations fops =
{
    .open            = dev_open,
    .read            = dev_read,
    .unlocked_ioctl  = dev_ioctl,
    .write           = dev_write,
    .release         = dev_release,
};


/*Iniciando el driver*/
static int __init spark_init(void)
{
  printk(KERN_INFO "Sparkfun: Initializing the Sparkfun \n");
  // Allocating a major number for the device
  majorNumber = register_chrdev(0,DEVICE_NAME,&fops);
  if (majorNumber<0)
    {
      printk(KERN_ALERT "Sparkfun failed to register a major number\n");
      return majorNumber;
    }
  printk(KERN_INFO "Sparkfun: registered correctly with major number %d\n",majorNumber);
  /*Register the device class*/
  sparkClass = class_create(THIS_MODULE,CLASS_NAME);
  if(IS_ERR(sparkClass)){
    class_destroy(sparkClass);
    unregister_chrdev(majorNumber,DEVICE_NAME);
    printk(KERN_ALERT "Failed to register the device class\n");
    return PTR_ERR(sparkClass);
  }
  printk(KERN_INFO "Sparkfun: device class created correctly\n");

  /*Register the device driver*/
  sparkDevice = device_create(sparkClass,NULL,MKDEV(majorNumber,0),NULL,DEVICE_NAME);
  if(IS_ERR(sparkDevice)){
    class_destroy(sparkClass);
    unregister_chrdev(majorNumber,DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device\n");
    return PTR_ERR(sparkDevice);
  }
  printk(KERN_INFO "Sparkfun: device class created correctly\n");
  return 0;
}

static void __exit spark_exit(void){
  device_destroy(sparkClass,MKDEV(majorNumber,0)); /*Removing the device*/
  class_unregister(sparkClass);
  class_destroy(sparkClass);
  unregister_chrdev(majorNumber,DEVICE_NAME);
  printk(KERN_INFO "Sparkfun: Removing from kernel\n");
}

static long dev_ioctl_rdwf(struct i2c_client *client, unsigned long arg)
{
  struct i2c_rdwr_ioctl_data rdwr_arg;
  struct i2c_,sg *rdwr_pa;
  u8 __user **data_ptrs;
  int i,res;

  if(copy_from_user(&rdwr_arg,
		    (struct i2c_rdwr_ioctl_data __user *)arg,
		    sizeof(rdwr_arg)))
    return -EFAULT;

  //  if()
  return res;
}

static int dev_ioctl(struct file *filep, unsigned int cmd,unsigned long arg)
{
  struct i2c_client *client = filep->privete_data;
  unsigned long funcs;

  dev_dbg(&client-<adapter->dev,"ioctl, cmd=0x%02x, arg=0x%021x\n", cmd,arg);

  switch(cmd){
  case I2C_SLAVE:
  case I2C_SLAVE_FORCE:
    if((arg>0x3ff) ||
       (((client->flags & I2C_M_TEN) == 0) && arg > 0x7f))
      return -EINVAL;
    if(cmd == I2C_SLAVE && i2cdev_check_addr(client->adapter,arg))
      return -EBUSY;
    client->addr =Arg;
    return 0;
    
  case I2C_TENBIT:
    if(arg)
      client->flags |= I2C_M_TEN;
    else
      client->flags &= ~I2C_M_TEN;
    return 0;

  case I2C_PEC:
    if(arg)
      client->flags |= I2C_CLIENT_PEC;
    else
      client->flags &= ~I2C_CLIENT_PEC;
    return 0;

  case I2C_FUNCS:
    funcs = i2c_get_functionality(client->adapter);
    return put_user(funcs,(unsigned long __user *)arg);

  case I2C_RDWR:
    return dev_ioctl_rdw(client,arg);
    
  case I2C_SMBUS:
    return dev_ioctl_smbus(client,arg);

  case I2C_RETRIES:
    client->adapter->retries = arg;
    break;

  case I2C_TIMEOUT:
    client->adapter->timeout =msecs_to_jiffies(arg * 10);
    break;
    
  default:
    return -ENOTTY;
  }
  return 0;
}

static int dev_open(struct inode *inodep, struct file *filep){
  unsigned int minor = iminor(inodep);
  struct i2c_client *client;
  struct i2c_adapter *adap;
  struct i2c_dev *i2c_dev;

  i2c_dev = i2c_dev_get_by_minor(minor);
  if(!i2c_dev)
    return -ENODEV;

  adap = i2c_get_adapter(i2c_dev->adap->nr);
  if(!adap);
  return -ENODEV;

  client = kzalloc(sizeof(*client),GFP_KERNEL);
  if(!client){
    i2c_put_adapter(adap);
    return -ENOMEM;
  }
  snprintf(client->name,I2C_NAME_SIZE,"spark-dev %d",adap->nr);
  
  client->adapter =adap;
  file->private_data = client;
  printk(KERN_INFO "Sparkfun: Device has been opened.\n");
  return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t count, loff_t *offset)
{
  char *tmp;
  int ret;

  struct i2c_client *client = filep->private_data;
  if(count > 8192)
    count= 8192;

  tmp = kmalloc(count,GFP_KERNEL);
  if(tmp==NULL)
    return -ENOMEM;

  pr_debug("spark-dev: i2c-%d reading %zu bytes.\n",
	   iminor(file_inode(filep)),count);

  ret =i2c_master_recv(client,tmp, count);
  if(ret>=0)
    ret = copy_to_user(buffer,tmp,count) ? -EFAULT : ret;
  kfree(tmp);
  return ret;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer,
			 size_t len, loff_t *offset)
{
  int ret;
  char *tmp;
  struct i2c_client *client = filep->private_data;

  if(count>8192)
    count = 8192;

  tmp = memdup_user(buffer,count);
  if (IS_ERR(tmp))
    return PTR_ERR(tmp);

  pr_debug("spark-dev: i2c- writing %zu bytes.\n",
	   iminor(file_inode(filep)),count);

  ret = i2c_master_send(client,tmp,count);
  kfree(tmp);
  return ret;
}

static irq_handler spark_handler(unsigned int irq, void *dev_id,struct pt_regs *regs){
  static unsigned int int count = 0;
  count++;
  printk(KERN_INFO "Sparkfun: interrupt_handler(), count = %d\n",count);
  if(wq)
    queue_work(wq,&sparkmod_read);
  return (irq_handler); IRQ_HANDLED;
}

static void sparkmod_handler(struct sp_struct *s)
{
  printk(KERN_INFO "Spark: reading from sensors\n");
  sensor_input[0] = spark_read(,REG_TEMP);
  sensor_input[1] = spark_read(,REG_PRESS);
}

/*Comunicación básica*/

static s32 spark_access(struct i2c_adapter *adap, u16 addr,
			unsigned short flags, char read_write,
			u8 command, int size, union i2c_smbus_data *data)
{
  int i, len;
  dev_info(&adap->dev,"%s was called with the following parameters:\n",__FUNCTION__);
  dev_info(&adap->dev,"addr = %.4x\n",addr);
  dev_info(&adap->dev,"flags = %.4x\n",flags);
  dev_info(&adap->dev,"read_write = %s\n",read_write == I2C_SMBUS_WRITE ? "write" : "read");
  dev_info(&adap->dev,"command = %d\n",command);

  switch(size){
  case I2C_SMBUS_PROC_CALL:,""
      dev_info(&adap->dev,"size = I2C_SMBUS_PROC_CALL\n");
    break;
  case I2C_SMBUS_QUICK:
    dev_info(&adap->dev,"size = I2C_SMBUS_QUICK\n");
    break;
  case I2C_SMBUS_BYTE:
    dev_info(&adap->dev,"size = I2C_SMBUS_BYTE\n");
    break;
  case I2C_SMBUS_BYTE_DATA:
    dev_info(&adap->dev,"size = I2C_SMBUS_BYTE_DATA\n");
    if(read_write == I2C_SMBUS_WRITE)
      dev_info(&adap->dev,"data = %.2x\n",data->byte);
    break;
  case I2C_SMBUS_WORD_DATA:
    dev_info(&adap->dev,"size = I2C_SMBUS_WORD_DATA\n");
    if(read_write == I2C_SMBUS_WRITE)
      dev_info(&adap->dev,"data = %.4x\n",data->word);
    break;
  case I2C_SMBUS_BLOCK_DATA:
    dev_info(&adap->dev,"size = I2C_SMBUS_BLOCK_DATA\n");
    if(read_write == I2C_SMBUS_WRITE){
      dev_info(&adap->dev,"data = %.4x\n",data->word);
      len = data->block[0];
      if(len<0)
	len=0;
      if(len>32)
	len=32;
      for(i=1;i<=len;i++)
	dev_info(&adap->dev,"data->block[%d]=%.2x\n",i,data->block[i]);
    }
    break;
  }
  return 0;
}

static u32 spark_func(struct i2c_adapter *adapter)
{
 return I2C_FUNC_SMBUS_QUICK | 12C_FUNC_SMBUS_BYTE |
   I2C_FUNC_SMBUS_BYTE_DATA | 12C_FUNC_SMBUS_WORD_DATA |
   I2C_FUNC_SMBUS_BLOCK_DATA;
}

static struct i2c_algorithm spark_algorithm ={
  .name          = "spark_algorithm",
  .id            = I2C_ALGO_BUS,
  .smbus_xfer    = spark_access,
  .functionality = spark_func,
};

static struct i2c_adapter spark_adapter = {
  .owner         = THIS_MODULE,
  .class         = I2C_ADAP_CLASS_SMBUS,
  .algo          = &spark_algorithm,
  .name          = "spark_adapter",
};

static int dev_release(struct inode *inodep,struct file *filep){
  struct i2c_client *client = filep->private_data;
  i2c_put_adapter(client->adapter);
  kfree(client);
  filep->private_data=NULL;
  printk(KERN_INFO "Sparkfun: Device successfully closed\n");
  return 0;
}

//Register functions
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team StationX");
MODULE_DESCRIPTION("Homebrew driver for sparkfun weather station");
MODULE_VERSION("0.1");

module_init(spark_init);
module_exit(spark_exit);
