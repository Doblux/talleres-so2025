#include <linux/init.h>
#include <linux/module.h> // THIS_MODULE
#include <linux/kernel.h> // Kernel cosas
#include <linux/fs.h>     // File operations
#include <linux/cdev.h>   // Char devices
#include <linux/device.h> // Nodos filesystem
#include <linux/uaccess.h> // copy_to_user
#include <linux/slab.h>    // kmalloc
#include <linux/random.h>  // get_random_bytes

#define SLOT_COUNT 3

static spinlock_t lock;

#define MAX_USOS_DE_DISPOSITIVO 3
static int cant_usos = 0;

typedef struct user_data {
    bool valid;
    char input;
} user_data;

static int letras_open(struct inode *inod, struct file *filp) {
    unsigned long flags;
    spin_lock_irqsave(&lock, flags); // lockeamos

    if (cant_usos >= MAX_USOS_DE_DISPOSITIVO)
    {
        spin_unlock_irqrestore(&lock, flags); // deslockeamos
        return -EPERM;
    }
    cant_usos++;
    spin_unlock_irqrestore(&lock, flags);

    filp->private_data = kmalloc(sizeof(struct user_data), GFP_KERNEL);
    ((user_data *) filp->private_data)->valid = false;
    return 0;
 
}

static int letras_release(struct inode *inod, struct file *filp) {
    unsigned long flags;

    if (filp->private_data == NULL) {
        // No hay espacio asignado, nada que liberar ni decrementar
        return 0;
    }

    spin_lock_irqsave(&lock, flags);
    if (cant_usos > 0){
        cant_usos--;
    }
    spin_unlock_irqrestore(&lock, flags);

    kfree(filp->private_data);
    return 0;
}

static ssize_t letras_read(struct file *filp, char __user *data, size_t size, loff_t *offset) {
    user_data *udata = (user_data *) filp->private_data;

    if (!udata->valid){
        return -EPERM;
    }

    char* buffer = kmalloc(size, GFP_KERNEL); // GFP_KERNEL = Get Free Pages, Kernel allocation
   
    for (size_t i = 0; i < size; i++)
    {
        buffer[i] = udata->input;
    }
    
    if (copy_to_user(data, buffer, size)) {
        kfree(buffer);
        return -EFAULT;
    }

    kfree(buffer);
    return size;
}

static ssize_t letras_write(struct file *filp, const char __user *data, size_t size, loff_t *offset) {
    user_data *udata = (user_data *) filp->private_data;
    
    if (udata->valid || size == 0)
    {
        return size; // si ya hay data dentro, ignoramos y no hacemos nada
    }
    
    char c;
    if (copy_from_user(&c, data, sizeof(char))) {
        return -EFAULT;  // Falló al leer del usuario
    }

    udata->input = c;
    udata->valid = true;

    return size;
}

static struct file_operations letras_fops = {
  .owner = THIS_MODULE,
  .open = letras_open,
  .read = letras_read,
  .write = letras_write,
  .release = letras_release,
};

static dev_t letras_devno;
static struct cdev letras_cdev;
static struct class *letras_class;

static int __init letras_init(void) {
    spin_lock_init(&lock);
    
    cdev_init(&letras_cdev, &letras_fops);
    alloc_chrdev_region(&letras_devno, 0, 1, "SO-letras123");
    cdev_add(&letras_cdev, letras_devno, 1);

    letras_class = class_create(THIS_MODULE, "letras123");
    device_create(letras_class, NULL, letras_devno, NULL, "letras123");

    printk(KERN_ALERT "Loading module 'letras123'\n");
    return 0;
}

static void __exit letras_exit(void) {
    printk(KERN_ALERT "Unloading module 'letras123'\n");

    device_destroy(letras_class, letras_devno);
    class_destroy(letras_class);

    unregister_chrdev_region(letras_devno, 1);
    cdev_del(&letras_cdev);
}

module_init(letras_init);
module_exit(letras_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Generador de números al letras");
