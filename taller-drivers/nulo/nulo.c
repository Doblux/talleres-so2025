#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

static ssize_t nulo_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
  return 0;
}

static ssize_t nulo_write(struct file *filp, const char __user *data, size_t s, loff_t *off) {
  int i = 0 ;
  while (data[i] != '\0'){
    i++;
  }
  
  return i;
}

static struct file_operations nulo_operaciones = {
  .owner = THIS_MODULE,
  .write = nulo_write,
  .read = nulo_read,
  // Completar
};

static struct cdev nulo_device;
static dev_t major;
static struct class *nulo_class;
int count = 1;

static int __init hello_init(void) {
  
  //inicialiar disposivo y estructura de operaciones
  cdev_init(&nulo_device,&nulo_operaciones);
  
  //conseguir major y minor
  int status = alloc_chrdev_region(&major,0,count, "nulo");
  
  //asignar los numeros al dispositivo inicializado
  cdev_add(&nulo_device,major,count);

  //crear nodos
  nulo_class = class_create(THIS_MODULE, "nulo");
  device_create(nulo_class,NULL,major,NULL,"nulo" );

	
  return 0;
}

static void __exit hello_exit(void) {
  
  unregister_chrdev_region(major,count);
  cdev_del(&nulo_device);
  device_destroy(nulo_class,major);
  class_destroy(nulo_class);

}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de '/dev/null'");
