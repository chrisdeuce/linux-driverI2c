/*
Información del driver
date:    Jan 20 2017
version: 1.0
brief:  Driver para utilizar la estación meteorologica I2C sparkfun weathershield 
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h> // copy to user ; copy from user
#define DEVICE_NAME "spark"  // El dispositivo aparecera en /dev/spark/
#define CLASS_NAME "spk"    // Tipo de dispositivo

//Register functions
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team StationX");
MODULE_DESCRIPTION("Homebrew driver for sparkfun weather station for embedded linux diploma curse");
MODULE_VERSION("0.1");

static int majorNumber;                     // Almacena el número de dispositivo
static char message[256] ={0};              // Memoria para la cadena que pasa del espacio de usuario
static short size_of_message;               // Utilizado para saber el tamaño de la cadena almacenada
static int numberOpens =0;                  // Contador de uso del dispositivo
static struct class* sparkClass = NULL;     // puntero de la clase del dispositivo
static struct device* sparkDevice = NULL;   // puntero a la estructura del dispositivo


/* Funciones prototipo para el driver */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, size_t,loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t*);


/*Brief devices*/
static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};



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
  mcdev =cdev_alloc(); //Inicializando el driver
  mcdev->ops=&fops;
  mcdev-owner = THIS_MODULE;
  /*creación de cdev*/
  /*Agregando el device al kernel*/
  ret =cdev_add(mcdev,dev_num,1);
  if(ret<0){
      printk(KERN_ALERT "Sparkfun: unable of load the cdev to kernel")
      return ret;
  }
  //Inicializando el semáforo
  sema_init(&virtual_device.sem,1); /*value 1 to the semaphore*/
  return 0;
}

static void driver_exit(void){
  // removiendo todo en orden contrario
  cdev_del(mcdev);
}



/*Iniciando el driver*/
static int _init spark_init(void)
{
  printk(KERN_INFO "SParkfun: Initializing the Sparkfun \n");
  // Allocating a major number for the device
  major_number = register_chrdev(0,DEVICE_NAME,&fops);
  if (majorNumber<0)
    {
      printk(KERN_ALERT "Sparkfun failed to register a major number\n");
      return majorNumber;
    }
  printk(KERN_INFO "Sparkfun: registered correctly with major number %d\n",majorNumber);
  /*Register the device class*/
  sparkClass = device_create(THIS_MODULE,CLASS_NAME);
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

static void __exit sparkfun_exit(void){
  device_destroy(slarkClass,MKDEV(majorNumber,0)); /*Removing the device*/
  class_unregister(sparkClass);
  class_destroy(sparkClass);
  unregister_chrdev(majorNumber,DEVICE_NAME);
  printk(KERN_INFO "Sparkfun: Removing from kernel\n");
}

static int dev_open(struct inode *inodep, struct file *filep){
  numberOpens++;
  printk(KERN_INFO "Sparkfun: Device has been opened %d time(s)\n");
  return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
  int error_count = 0;
  error_count = copy_to_user(buffer,message,size_of_message);

  if(error_count=0){
    printk(KERN_INFO "Sparkfun: Sent %d of chars to the user \n",size_of_message);
    return (size_of_message=0);
  }
  else{
    printk(KERN_INFO "Sparkfun: Failed to send info from sensors to the user\n",error_count);
    return -EFAULT;
  }
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
