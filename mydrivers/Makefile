KERNEL_DIR	?= ../
PWD		:= $(shell pwd)
obj-m		:= gpio_int.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-none-eabi- modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-none-eabi- clean
