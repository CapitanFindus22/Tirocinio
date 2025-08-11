EXTRA_CFLAGS += -g
obj-m += dev_mod.o

all:
	make -C ../linux-6.14.7 M=$(PWD) modules 
	cp dev_mod.ko ../rootfs/lib/modules

clean:
	make -C ../linux-6.14.7 M=$(PWD) clean 
