EXTRA_CFLAGS += -g
obj-m += main.o

all:
	make -C ../../../../linux-6.14.7 M=$(PWD) modules 

clean:
	make -C ../../../../linux-6.14.7 M=$(PWD) clean 
