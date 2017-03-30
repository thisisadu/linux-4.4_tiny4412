#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/of_gpio.h>

#define uchar unsigned char

static void touch_read_handler(struct work_struct *work);
DECLARE_WORK(touch_read_work,touch_read_handler);

static struct i2c_client *touch_client;
static struct input_dev *touch_dev;
static int irq;

static void touch_read(uchar addr,uchar *buf,unsigned int len)
{
  int i,ret;
  struct i2c_msg msg[2];
  uchar address;

  for(i=0;i<len;i++){
    address = addr+i;
    msg[0].addr = touch_client->addr;
    msg[0].buf = &address;
    msg[0].len = 1;
    msg[0].flags = 0;

    msg[1].addr = touch_client->addr;
    msg[1].buf = &buf[i];
    msg[1].len = 1;
    msg[1].flags = I2C_M_RD;
    ret = i2c_transfer(touch_client->adapter,msg,2);
    if(ret < 0){
      printk("i2c transfer error\n");
    }
    mdelay(10);
  }
}

static void touch_read_handler(struct work_struct *work)
{
  uchar buf[13];
  uchar touches,i,event,id;
  unsigned short x,y;
  bool act;

  touch_read(0x00,buf,13);

  touches = buf[2]&0x0f;

  if(touches > 2){
    printk("touch read touches greater than 2\n");
    touches = 2;
  }

  for(i=0;i<touches;i++){
    y = ((buf[5+i*6]&0x0f)<<8) | buf[6+i*6];
    x = ((buf[3+i*6]&0x0f)<<8) | buf[4+i*6];

    event = buf[3+i*6]>>6;
    id = buf[5+i*6]>>4;

    act = (event == 0x00 || event == 0x02);

    input_mt_slot(touch_dev,id);
    input_mt_report_slot_state(touch_dev,MT_TOOL_FINGER,act);

    if(!act)
      return;

    input_report_abs(touch_dev,ABS_MT_POSITION_X,x);
    input_report_abs(touch_dev,ABS_MT_POSITION_Y,y);
  }

  input_mt_sync_frame(touch_dev);
  input_sync(touch_dev);
}

static irqreturn_t touch_isr(int irq,void *dev_id)
{
  schedule_work(&touch_read_work);
  return IRQ_HANDLED;
}

static int touch_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  uchar buf;
  int ret;

  printk("%s enter.\n",__func__);
  touch_client = client;

  touch_read(0xa3,&buf,1);
  printk("chip vendor id:%x\n",buf);

  touch_read(0xa6,&buf,1);
  printk("firmware id:%x\n",buf);

  touch_dev = input_allocate_device();
  if(touch_dev == NULL){
    printk("allocate input device error\n");
    return -1;
  }

  input_set_abs_params(touch_dev,ABS_MT_POSITION_X,0,800,0,0);
  input_set_abs_params(touch_dev,ABS_MT_POSITION_Y,0,480,0,0);
  ret = input_mt_init_slots(touch_dev,2,INPUT_MT_DIRECT|INPUT_MT_DROP_UNUSED);
  if(ret){
    printk("input mt init slots error\n");
    return ret;
  }

  touch_dev->name = "touch";
  touch_dev->id.bustype = BUS_I2C;
  touch_dev->dev.parent = &touch_client->dev;
  ret = input_register_device(touch_dev);
  if(ret){
    printk("register device error\n");
    return ret;
  }

  printk("irq is %d\n",irq);
  ret = devm_request_threaded_irq(&touch_client->dev,irq,touch_isr,NULL,IRQF_TRIGGER_FALLING|IRQF_ONESHOT,"touch1",NULL);
  if(ret){
    printk("failed to request irq\n");
    return ret;
  }

  return 0;
}

static int touch_remove(struct i2c_client *client)
{
  printk("%s enter.\n",__func__);
  return 0;
}

static int int_probe(struct platform_device *pdev)
{
  printk("%s enter.\n",__func__);
  irq = platform_get_irq(pdev,0);
  return 0;
}

static int int_remove(struct platform_device *pdev)
{
  printk("%s enter.\n",__func__);
  return 0;
}

static const struct i2c_device_id touch_id_table[] =
{
  { "touch",0 },
  {},
};

static struct i2c_driver touch_driver =
{
  .driver = {
    .name = "touch",
    .owner = THIS_MODULE,
  },
  .probe = touch_probe,
  .remove = touch_remove,
  .id_table = touch_id_table,
};

static const struct of_device_id touch_dt_ids[] =
{
    { .compatible = "tiny4412,touch_demo", },
    {},
};
MODULE_DEVICE_TABLE(of, touch_dt_ids);

static struct platform_driver touch_demo_driver =
{
    .driver        = {
        .name      = "touch_demo",
        .of_match_table    = of_match_ptr(touch_dt_ids),
    },
    .probe         = int_probe,
    .remove        = int_remove,
};

static int touch_init(void)
{
    int ret;
    printk("enter %s\n", __func__);
    ret = platform_driver_register(&touch_demo_driver);
    if (ret)
    {
        printk(KERN_ERR "touch demo: probe faid touch: %d\n", ret);
    }

    i2c_add_driver(&touch_driver);

    return ret;
}

static void touch_exit(void)
{
    printk("enter %s\n", __func__);
    i2c_del_driver(&touch_driver);
    platform_driver_unregister(&touch_demo_driver);
}

module_init(touch_init);
module_exit(touch_exit);
MODULE_LICENSE("GPL");
