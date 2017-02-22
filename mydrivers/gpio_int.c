#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>

typedef struct{
  int gpio;
  int irq;
  char name[20];
}demo_data_t;

static irqreturn_t demo_isr(int irq,void *dev_id)
{
  demo_data_t *data = (demo_data_t*)dev_id;

  printk("%s enter,%s:gpio:%d,irq:%d\n",__func__,data->name,data->gpio,data->irq);

  return IRQ_HANDLED;
}

static int demo_probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  demo_data_t *data = NULL;
  int ret = 0,i = 0;

  printk("%s enter.\n",__func__);

  if(!dev->of_node){
    dev_err(dev,"no platform data\n");
    goto err1;
  }

  data = devm_kmalloc(dev,sizeof(*data) * 2,GFP_KERNEL);
  if(!data){
    dev_err(dev,"no memory\n");
    goto err0;
  }

  data[0].irq = platform_get_irq(pdev,0);
    ret = devm_request_any_context_irq(dev,data[0].irq,demo_isr,IRQF_TRIGGER_FALLING,data[0].name,data+0);
    if(ret < 0){
      dev_err(dev,"unable to claim irq %d,err:%d\n",data[0].irq,ret);
      goto err1;
    }

    data[1].irq = platform_get_irq(pdev,1);
    ret = devm_request_any_context_irq(dev,data[1].irq,demo_isr,IRQF_TRIGGER_FALLING,data[1].name,data+1);
    if(ret < 0){
      dev_err(dev,"unable to claim irq %d,err:%d\n",data[1].irq,ret);
      goto err1;
    }
  /* for(i=3;i>=0;i--){ */
  /*   sprintf(data[i].name,"tiny4412,int_gpio%d",i+1); */

  /*   data[i].gpio = of_get_named_gpio(dev->of_node,data[i].name,0); */
  /*   if(data[i].gpio < 0){ */
  /*     dev_err(dev,"looking up %s property in node %s failed %d\n",data[i].name,dev->of_node->full_name,data[i].gpio); */
  /*     goto err1; */
  /*   } */

  /*   data[i].irq = gpio_to_irq(data[i].gpio); */
  /*   if(data[i].irq < 0){ */
  /*     dev_err(dev,"unable to get irq number for gpio %d,err:%d\n",data[i].gpio,data[i].irq); */
  /*     goto err1; */
  /*   } */

  /*   ret = devm_request_any_context_irq(dev,data[i].irq,demo_isr,IRQF_TRIGGER_FALLING,data[i].name,data+i); */
  /*   if(ret < 0){ */
  /*     dev_err(dev,"unable to claim irq %d,err:%d\n",data[i].irq,ret); */
  /*     goto err1; */
  /*   } */
  /* } */

  return 0;

err1:
  devm_kfree(dev,data);
err0:
  return -EINVAL;
}

static int demo_remove(struct platform_device *pdev)
{
  printk("%s enter.\n",__func__);
  return 0;
}

static const struct of_device_id demo_dt_ids[]={
  {.compatible = "tiny4412,interrupt_demo",},
  {},
};

MODULE_DEVICE_TABLE(of, demo_dt_ids);

static struct platform_driver demo_driver = {
  .driver = {
    .name = "interrupt_demo",
    .of_match_table = of_match_ptr(demo_dt_ids),
  },
  .probe = demo_probe,
  .remove = demo_remove,
};

static int __init demo_init(void)
{
  int ret = platform_driver_register(&demo_driver);
  if(ret)
    printk(KERN_ERR"demo register failed:%d\n",ret);

  return ret;
}
module_init(demo_init);

static void __exit demo_exit(void)
{
  platform_driver_unregister(&demo_driver);
}
module_exit(demo_exit);

MODULE_LICENSE("GPL");
