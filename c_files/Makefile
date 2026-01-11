PWD := $(shell pwd)
KDIR := /home/anx/linux-6.18.3/

MODULE_NAME = periodic_blink

obj-m += ${MODULE_NAME}.o

.PHONY: all clean move_ko del_initrd run_tempsh run_qemu run_clear

all: modules move_ko del_initrd run_tempsh run_qemu clean run_clear

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

move_ko:
	rm -rf ../busyboxx/*.ko
	mv ${MODULE_NAME}.ko ../busyboxx/

del_initrd:
	rm -rf ../busyboxx/initrd

run_tempsh:
	cd ../busyboxx && ./temp.sh

run_qemu:
	cd ../busyboxx && qemu-system-x86_64 -kernel bzImage -initrd initrd.img -nographic -append "console=ttyS0"

run_clear:
	clear

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean