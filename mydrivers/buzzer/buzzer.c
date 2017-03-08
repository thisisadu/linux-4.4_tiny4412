static int major;
static struct cdev pwm_cdev;
static struct class *cls;

struct TIMER_BASE{
  unsigned int TCFG0;
  unsigned int TCFG1;
  unsigned int TCON;
  unsigned int TCNTB0;
  unsigned int TCMPB0;
};
volatile static struct TIMER_BASE * timer = NULL;


static int pwm_probe(struct platform_device *pdev)
{
  int ret;
  dev_t devid;
  struct device *dev = &pdev->dev;
  struct resource *res = NULL,
  struct clk *clk;

  printk("enter %s\n",__func__);

  res = platform_get_resource(pdev,IORESOURCE_MEM,0);
  if(res == NULL){
    printk("platform get resource error\n");
    return -EINVAL;
  }

  timer = devm_ioremap_resource(dev,res);
  if(timer == NULL){
    printk("ioremap resource error\n");
    return -EINVAL;
  }

  clk = devm_clk_get(dev,"timers");
  if(IS_ERR(clk)){
    printk("clk get error\n");
    return PTR_ERR(clk);
  }

  ret = clk_prepare_enable(clk);
  if(ret < 0){
    printk("enable clk error\n");
    return ret;
  }

  init_timer();

  if(alloc_chrdev_region(&devid,0,1,"pwm") < 0){
    printk("alloc chrdev region error\n");
    return -EINVAL;
  }

  major = MAJOR(devid);
  cdev_init(&pwm_cdev,&pwm_fops);
  cdev_add(&pwm_dev,devid,1);
  cls = class_create(THIS_MODULE,"mypwm");
  device_create(cls,NULL,MKDEV(major,0),NULL,"pwm0");
  return 0;
}

static int pwm_remove(struct platform_device *pdev)
{
  printk("enter %s\n",__func__);
  device_destroy(cls,MKDEV(major,0));
  class_destroy(cls);
  cdev_del(&pwm_cdev);
  unregister_chrdev_region(MKDEV(major,0),1);
  return 0;
}

static const struct of_device_id dt_ids[] = {
  {.compatible = "tiny4412,pwm_demo",},
};
MODULE_DEVICE_TABLE(of,dt_ids);

static struct platform_driver pwm_driver = {
  .driver = {
    .name = "pwm_demo",
    .of_match_table = of_match_ptr(dt_ids),
  },
  .probe = pwm_probe,
  .remove = pwm_remove,
};

static int pwm_init()
{
  int ret;
  printk("enter %s\n",__func__);
  ret = platform_driver_register(&pwm_driver);
  if(ret){
    printk("pwm demo probe failed:%d\n",ret);
  }

  return ret;
}


static void pwm_exit()
{
  printk("leave %s\n",__func__);
  platform_driver_unregister(&pwm_driver);
}

module_init(pwm_init);
module_exit(pwm_exit);
MODULUE_LICENSE("GPL");
