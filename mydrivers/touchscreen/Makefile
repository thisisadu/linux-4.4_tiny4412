KERNEL_DIR	?= /home/xiaoqingwa/git/linux-4.4_tiny4412
PWD		:= $(shell pwd)
obj-m		:= touchscreen.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-none-eabi- modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-none-eabi- clean
