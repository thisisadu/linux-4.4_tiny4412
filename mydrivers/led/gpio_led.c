#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define LED_CNT 4

static int major;
static struct cdev led_cdev;
static struct class *cls;
static int led1,led2,led3,led4;

static ssize_t led_write(struct file *file,const char __user*user_buf,size_t count,loff_t *ppos)
{
  char buf;
  int minor = iminor(file->f_inode);

  if(count != 1)
    return 1;

  if(copy_from_user(&buf,user_buf,count))
    return -EFAULT;

  switch(minor){
  case 0:
    gpio_set_value(led1,buf);
    break;
  case 1:
    gpio_set_value(led2,buf);
    break;
  case 2:
    gpio_set_value(led3,buf);
    break;
  case 3:
    gpio_set_value(led4,buf);
    break;
  }
  return 0;
}

static int led_open(struct inode *inode,struct file *file)
{
  return 0;
}

static struct file_operations led_fops = {
  .owner = THIS_MODULE,
  .open = led_open,
  .write = led_write,
};

static int led_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  dev_t devid;

  led1 = of_get_named_gpio(dev->of_node,"tiny4412,int_gpio1",0);
  led2 = of_get_named_gpio(dev->of_node,"tiny4412,int_gpio2",0);
  led3 = of_get_named_gpio(dev->of_node,"tiny4412,int_gpio3",0);
  led4 = of_get_named_gpio(dev->of_node,"tiny4412,int_gpio4",0);

  devm_gpio_request_one(dev,led1,GPIOF_OUT_INIT_HIGH,"LED1");
  devm_gpio_request_one(dev,led2,GPIOF_OUT_INIT_HIGH,"LED2");
  devm_gpio_request_one(dev,led3,GPIOF_OUT_INIT_HIGH,"LED3");
  devm_gpio_request_one(dev,led4,GPIOF_OUT_INIT_HIGH,"LED4");

  if(alloc_chrdev_region(&devid,0,LED_CNT,"led") < 0){
    goto err;
  }

  major =MAJOR(devid);

  cdev_init(&led_cdev,&led_fops);
  cdev_add(&led_cdev,devid,LED_CNT);

  cls = class_create(THIS_MODULE,"led");
  device_create(cls,NULL,MKDEV(major,0),NULL,"led0");
  device_create(cls,NULL,MKDEV(major,1),NULL,"led1");
  device_create(cls,NULL,MKDEV(major,2),NULL,"led2");
  device_create(cls,NULL,MKDEV(major,3),NULL,"led3");

 err:
  unregister_chrdev_region(MKDEV(major,0),LED_CNT);
  return 0;
}

static int led_remove(struct platform_device *pdev)
{
  printk("%s enter.\n",__func__);
  device_destroy(cls,MKDEV(major,0));
  device_destroy(cls,MKDEV(major,1));
  device_destroy(cls,MKDEV(major,2));
  device_destroy(cls,MKDEV(major,3));
  class_destroy(cls);

  cdev_del(&led_cdev);

  unregister_chrdev_region(MKDEV(major,0),LED_CNT);
  return 0;
}

static const struct of_device_id led_dt_ids[]={
  {.compatible = "tiny4412,led_demo",},
  {},
};

MODULE_DEVICE_TABLE(of, led_dt_ids);

static struct platform_driver led_driver = {
  .driver = {
    .name = "led_demo",
    .of_match_table = of_match_ptr(led_dt_ids),
  },
  .probe = led_probe,
  .remove = led_remove,
};

static int __init led_init(void)
{
  int ret = platform_driver_register(&led_driver);
  if(ret)
    printk(KERN_ERR"led register failed:%d\n",ret);

  return ret;
}
module_init(led_init);

static void __exit led_exit(void)
{
  platform_driver_unregister(&led_driver);
}
module_exit(led_exit);

MODULE_LICENSE("GPL");
